////////////////////////////////////////////////////////////////////////////////
//
// This file is part of Strata.
//
// Strata is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Strata is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// Strata.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "AbstractIterativeCalculator.h"

#include "RockLayer.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"

AbstractIterativeCalculator::AbstractIterativeCalculator(QObject *parent)
    : AbstractCalculator(parent), m_maxIterations(0), m_errorTolerance(0),
    m_converged(false)
{
    m_maxIterations = 10;
    m_errorTolerance = 2.;
    m_converged = false;
}

bool AbstractIterativeCalculator::run(AbstractMotion* motion, SoilProfile* site)
{
    init(motion, site);
    m_okToContinue = true;
    m_converged = false;

    // Compute the bedrock properties -- these do not change during the process.
    // The shear modulus is constant over the frequency range.
    m_shearMod[m_nsl].fill(calcCompShearMod(
            m_site->bedrock()->shearMod(), m_site->bedrock()->damping() / 100.));

    // Estimate the intial strain from the ratio of peak ground velocity of the
    //  motion and the shear-wave velocity of the layer
    for (int i = 0; i < m_nsl; ++i)
        m_site->subLayers()[i].setInitialStrain(
                m_motion->pgv() / m_site->subLayers().at(i).shearVel());

    // Compute the complex shear modulus and complex shear-wave velocity for
    // each soil layer -- initially this is assumed to be frequency independent
    for (int i = 0; i < m_nsl; ++i )
        m_shearMod[i].fill(calcCompShearMod(
                m_site->shearMod(i), m_site->damping(i) / 100.));

    // Initialize the loop control variables
    int iter = 0;
    double maxError = 0;
    QVector<std::complex<double> > strainTf;

    // While the error in the properties is greater than the tolerable limit
    // and the number of iterations is under the maximum compute the strain
    // compatible properties.
    do {
        if (!m_okToContinue)
            return false;

        // Compute the upgoing and downgoing waves
        if (!calcWaves())
            return false;

        // Compute the strain in each of the layers
        for (int i = 0; i < m_nsl; ++i) {
            strainTf = calcStrainTf(m_site->inputLocation(), m_motion->type(),
                                    Location(i, m_site->subLayers().at(i).thickness() / 2));

            // Update the soil layer with the new strain -- only changes the complex shear modulus
            if (!updateSubLayer(i, strainTf))
                return false;

            // Save the error for the first layer or if the error within the layer is larger than the previously saved max
            if (!i || maxError < m_site->subLayers().at(i).error())
                maxError = m_site->subLayers().at(i).error();

            if (!m_okToContinue)
                return false;
        }

        // Print information regarding the iteration
        if ( m_textLog->level() > TextLog::Low )
            m_textLog->append(QString(QObject::tr("\t\t\tIteration: %1 Maximum Error: %2 %"))
                              .arg(iter+1)
                              .arg(maxError, 0, 'f', 2));

        if ( m_textLog->level() > TextLog::Medium )
            m_textLog->append("\t\t" + m_site->subLayerTable());

        // Step the iteration
        ++iter;

    } while((maxError > m_errorTolerance)  && (iter < m_maxIterations));

    if ((iter == m_maxIterations ) && ( maxError > m_errorTolerance ) ) {
        m_textLog->append(QString(QObject::tr("\t\t\t!! -- Maximum number of iterations reached (%1). Maximum Error: %2 %"))
                          .arg(iter)
                          .arg(maxError, 0, 'f', 2));

        m_converged = false;
    } else {
        m_converged = true;
    }

    return true;
}

int AbstractIterativeCalculator::maxIterations() const
{
    return m_maxIterations;
}

void AbstractIterativeCalculator::setMaxIterations(int maxIterations)
{
    if (m_maxIterations != maxIterations) {
        m_maxIterations = maxIterations;

        emit maxIterationsChanged(m_maxIterations);
        emit wasModified();
    }
}

double AbstractIterativeCalculator::errorTolerance() const
{
    return m_errorTolerance;
}

void AbstractIterativeCalculator::setErrorTolerance(double errorTolerance)
{
    if (m_errorTolerance != errorTolerance) {
        m_errorTolerance = errorTolerance;

        emit errorToleranceChanged(m_errorTolerance);
        emit wasModified();
    }
}

bool AbstractIterativeCalculator::converged() const
{
    return m_converged;
}


double AbstractIterativeCalculator::relError(double value, double reference)
{
    return 100. * (value - reference) / reference;
}

double AbstractIterativeCalculator::maxError(const QVector<double> &maxStrain)
{
    Q_ASSERT(m_prevMaxStrain.size() == maxStrain.size());

    double max = relError(maxStrain.first(), m_prevMaxStrain.first());

    for (int i = 1; i < maxStrain.size(); ++i) {
        const double re = relError(maxStrain.at(i), m_prevMaxStrain.at(i));

        if (re > max)
            max = re;
    }

    // Save the maximum strains
    m_prevMaxStrain = maxStrain;

    return max;
}

void AbstractIterativeCalculator::ptRead(const ptree &pt)
{
    m_maxIterations = pt.get<int>("maxIterations");
    m_errorTolerance = pt.get<double>("errorTolerance");
}

void AbstractIterativeCalculator::ptWrite(ptree &pt) const
{
    pt.put("maxIterations", m_maxIterations);
    pt.put("errorTolerance", m_errorTolerance);
}

QDataStream & operator<< (QDataStream & out, const AbstractIterativeCalculator* aic)
{
    out << (quint8)1;

    out << aic->m_maxIterations
            << aic->m_errorTolerance;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractIterativeCalculator* aic)
{
    quint8 ver;
    in >> ver;

    in >> aic->m_maxIterations
            >> aic->m_errorTolerance;

    return in;
}
