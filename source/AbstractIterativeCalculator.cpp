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
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "AbstractIterativeCalculator.h"

#include "RockLayer.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"

#include "math.h"

AbstractIterativeCalculator::AbstractIterativeCalculator(QObject *parent)
    : AbstractCalculator(parent), _maxIterations(0), _errorTolerance(0),
    _converged(false)
{
    _maxIterations = 10;
    _errorTolerance = 2.;
    _converged = false;
}

bool AbstractIterativeCalculator::run(AbstractMotion* motion, SoilProfile* site)
{
    init(motion, site);
    _okToContinue = true;
    _converged = false;

    // Compute the bedrock properties -- these do not change during the process.
    // The shear modulus is constant over the frequency range.
    _shearMod[_nsl].fill(calcCompShearMod(
            _site->bedrock()->shearMod(), _site->bedrock()->damping() / 100.));

    setInitialStrains();

    // Initialize the loop control variables
    int iter = 0;
    double maxError = 0;
    QVector<std::complex<double> > strainTf;

    // While the error in the properties is greater than the tolerable limit
    // and the number of iterations is under the maximum compute the strain
    // compatible properties.
    do {
        if (!_okToContinue)
            return false;

        // Compute the upgoing and downgoing waves
        if (!calcWaves())
            return false;

        // Compute the strain in each of the layers
        for (int i = 0; i < _nsl; ++i) {
            strainTf = calcStrainTf(_site->inputLocation(), _motion->type(),
                                    Location(i, _site->subLayers().at(i).thickness() / 2));

            // Update the soil layer with the new strain -- only changes the complex shear modulus
            if (!updateSubLayer(i, strainTf))
                return false;

            // Save the error for the first layer or if the error within the layer is larger than the previously saved max
            if (!i || maxError < _site->subLayers().at(i).error())
                maxError = _site->subLayers().at(i).error();

            if (!_okToContinue)
                return false;
        }

        // Print information regarding the iteration
        if ( _textLog->level() > TextLog::Low )
            _textLog->append(QString(QObject::tr("\t\t\tIteration: %1 Maximum Error: %2 %"))
                              .arg(iter+1)
                              .arg(maxError, 0, 'f', 2));

        if ( _textLog->level() > TextLog::Medium )
            _textLog->append("\t\t" + _site->subLayerTable());

        // Step the iteration
        ++iter;

    } while((maxError > _errorTolerance)  && (iter < _maxIterations));

    if ((iter == _maxIterations ) && ( maxError > _errorTolerance ) ) {
        _textLog->append(QString(QObject::tr("\t\t\t!! -- Maximum number of iterations reached (%1). Maximum Error: %2 %"))
                          .arg(iter)
                          .arg(maxError, 0, 'f', 2));

        _converged = false;
    } else {
        _converged = true;
    }

    return true;
}

void AbstractIterativeCalculator::setInitialStrains()
{
    // Estimate the intial strain from the ratio of peak ground velocity of the
    //  motion and the shear-wave velocity of the layer.
    double estimatedStrain = 0;
    for (int i = 0; i < _nsl; ++i) {
        estimatedStrain = _motion->pgv() / _site->subLayers().at(i).shearVel();
        _site->subLayers()[i].setInitialStrain(estimatedStrain);
    }

    // Compute the complex shear modulus and complex shear-wave velocity for
    // each soil layer -- initially this is assumed to be frequency independent
    for (int i = 0; i < _nsl; ++i ) {
        _shearMod[i].fill(calcCompShearMod(
                _site->shearMod(i), _site->damping(i) / 100.));
    }
}

int AbstractIterativeCalculator::maxIterations() const
{
    return _maxIterations;
}

void AbstractIterativeCalculator::setMaxIterations(int maxIterations)
{
    if (_maxIterations != maxIterations) {
        _maxIterations = maxIterations;

        emit maxIterationsChanged(_maxIterations);
        emit wasModified();
    }
}

double AbstractIterativeCalculator::errorTolerance() const
{
    return _errorTolerance;
}

void AbstractIterativeCalculator::setErrorTolerance(double errorTolerance)
{
    if (_errorTolerance != errorTolerance) {
        _errorTolerance = errorTolerance;

        emit errorToleranceChanged(_errorTolerance);
        emit wasModified();
    }
}

bool AbstractIterativeCalculator::converged() const
{
    return _converged;
}


double AbstractIterativeCalculator::relError(double value, double reference)
{
    return 100. * (value - reference) / reference;
}

double AbstractIterativeCalculator::maxError(const QVector<double> &maxStrain)
{
    Q_ASSERT(_prevMaxStrain.size() == maxStrain.size());

    double max = relError(maxStrain.first(), _prevMaxStrain.first());

    for (int i = 1; i < maxStrain.size(); ++i) {
        const double re = relError(maxStrain.at(i), _prevMaxStrain.at(i));

        if (re > max)
            max = re;
    }

    // Save the maximum strains
    _prevMaxStrain = maxStrain;

    return max;
}

void AbstractIterativeCalculator::fromJson(const QJsonObject &json)
{
    _maxIterations = json["maxIterations"].toInt();
    _errorTolerance = json["errorTolerance"].toDouble();
}

QJsonObject AbstractIterativeCalculator::toJson() const
{
    QJsonObject json;
    json["maxIterations"] = _maxIterations;
    json["errorTolerance"] = _errorTolerance;
    return json;
}


QDataStream & operator<< (QDataStream & out, const AbstractIterativeCalculator* aic)
{
    out << (quint8)1;

    out << aic->_maxIterations
            << aic->_errorTolerance;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractIterativeCalculator* aic)
{
    quint8 ver;
    in >> ver;

    in >> aic->_maxIterations
            >> aic->_errorTolerance;

    return in;
}
