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

#include "EquivalentLinearCalculator.h"

#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

EquivalentLinearCalculator::EquivalentLinearCalculator(QObject *parent)
    : AbstractIterativeCalculator(parent)
{
    m_strainRatio = 0.65;
}

QString EquivalentLinearCalculator::toHtml() const
{
    return tr(
            "<li>Equivalent Linear Parameters"
            "<table border=\"0\">"
            "<tr><th>Effective strain ratio:</th><td>%1 Hz</td></tr>"
            "<tr><th>Error tolerance:</th><td>%2</td></tr>"
            "<tr><th>Maximum number of iterations:</th><td>%3</td></tr>"
            "</table>"
            "</li>"
            )
            .arg(m_strainRatio)
            .arg(m_errorTolerance)
            .arg(m_maxIterations);
}

double EquivalentLinearCalculator::strainRatio() const
{
    return m_strainRatio;
}

void EquivalentLinearCalculator::setStrainRatio(double strainRatio)
{
    if (m_strainRatio != strainRatio) {
        m_strainRatio = strainRatio;

        emit strainRatioChanged(m_strainRatio);
        emit wasModified();
    }
}

bool EquivalentLinearCalculator::updateSubLayer(int index, const QVector<std::complex<double> > strainTf)
{
    const double strainMax = 100 * m_motion->calcMaxStrain(strainTf);

    if (strainMax <= 0)
        return false;

    m_site->subLayers()[index].setStrain(m_strainRatio * strainMax, strainMax);

    // Compute the complex shear modulus and complex shear-wave velocity
    // for each soil layer -- these change because the damping and shear
    // modulus change.
    m_shearMod[index].fill(calcCompShearMod(
            m_site->subLayers().at(index).shearMod(),
            m_site->subLayers().at(index).damping() / 100.));

    return true;
}

QDataStream & operator<< (QDataStream & out,
                                 const EquivalentLinearCalculator* elc)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractIterativeCalculator*>(elc)
            << elc->m_strainRatio;

    return out;
}

QDataStream & operator>> (QDataStream & in,
                                 EquivalentLinearCalculator* elc)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractIterativeCalculator*>(elc);
    in >> elc->m_strainRatio;

    return in;
}
