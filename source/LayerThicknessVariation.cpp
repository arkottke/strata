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

#include "LayerThicknessVariation.h"

#include "ProfileRandomizer.h"
#include "Units.h"

#include <QDataStream>

#include <gsl/gsl_randist.h>

#include <cfloat>
#include <cmath>

LayerThicknessVariation::LayerThicknessVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    _enabled(false),
    _model(Custom),
    _coeff(0),
    _initial(0),
    _exponent(0),
    _rng(rng), 
    _profileRandomizer(profileRandomizer)
{
    connect(_profileRandomizer, SIGNAL(enabledChanged(bool)), this, SLOT(updateEnabled()));
    setModel(Default);
}

QStringList LayerThicknessVariation::modelList()
{
    QStringList list;

    list << tr("Custom")
        << tr("Default (Toro 1995)");

    return list;
}

bool LayerThicknessVariation::enabled() const
{
    return _profileRandomizer->enabled() && _enabled;
}

void LayerThicknessVariation::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();
    }
}

void LayerThicknessVariation::updateEnabled()
{
    emit enabledChanged(enabled());
}

LayerThicknessVariation::Model LayerThicknessVariation::model()
{
    return _model;
}

void LayerThicknessVariation::setModel(LayerThicknessVariation::Model model)
{
    if (_model != model) {
        _model = model;

        switch (_model) {
        case Default:
            setCoeff(1.98);
            setInitial(10.86);
            setExponent(-0.89);
            break;
        case Custom:
            break;
        }

        emit modelChanged(_model);
        emit customEnabledChanged(customEnabled());
        emit wasModified();
    }
}

void LayerThicknessVariation::setModel(int model)
{
    setModel((Model)model);
}

bool LayerThicknessVariation::customEnabled() const
{
    return _model == Custom;
}

double LayerThicknessVariation::coeff() const
{
    return _coeff;
}

void LayerThicknessVariation::setCoeff(double coeff)
{
    if (abs(_coeff - coeff) > DBL_EPSILON) {
        _coeff = coeff;

        emit coeffChanged(_coeff);
        emit wasModified();
    }
}

double LayerThicknessVariation::initial() const
{
    return _initial;
}

void LayerThicknessVariation::setInitial(double initial)
{
    if (abs(_initial - initial) > DBL_EPSILON) {
        _initial = initial;

        emit initialChanged(_initial);
        emit wasModified();
    }
}

double LayerThicknessVariation::exponent() const
{
    return _exponent;
}

void LayerThicknessVariation::setExponent(double exponent)
{
    if (abs(_exponent - exponent) > DBL_EPSILON) {
        _exponent = exponent;

        emit exponentChanged(_exponent);
        emit wasModified();
    }
}

void LayerThicknessVariation::reset()
{
    setEnabled(false);
    setModel(Default);
}

QList<double> LayerThicknessVariation::vary(double depthToBedrock) const
{
    // Need to convert the depth to bedrock into meters for the Toro (1997)
    // model.
    depthToBedrock *= Units::instance()->toMeters();

    // The thickness of the layers
    QList<double> thicknesses;

    // The layering is generated using a non-homogenous Poisson process.  The
    // following routine is used to generate the layering.  The rate function,
    // \lambda(t), is integrated from 0 to t to generate cumulative rate
    // function, \Lambda(t).  This function is then inverted producing
    // \Lambda^-1(t).  Random variables are produced using the a exponential
    // random variation with mu = 1 and converted to the nonhomogenous
    // variables using the inverted function.

    // Random variable that is a sum of exponential random variables
    double sum  = 0;

    // Previously computed depth
    double prevDepth = 0;

    while ( prevDepth < depthToBedrock ) {
        // Add a random increment
        sum += gsl_ran_exponential(_rng, 1.0);

        // Convert between x and depth using the inverse of \Lambda(t)
        double depth =
            pow((_exponent * sum) / _coeff
                    + sum / _coeff
                    + pow(_initial, _exponent + 1)
                    , 1 / (_exponent + 1)
               ) - _initial;

        // Add the thickness
        thicknesses << depth - prevDepth;

        prevDepth = depth;
    }

    // Correct the last layer of thickness so that the total depth is equal to the maximum depth
    if (thicknesses.size())
        thicknesses.last() = thicknesses.last() - (prevDepth - depthToBedrock);

    // Convert the thicknesses back into the target unit system
    for (int i = 0; i < thicknesses.size(); ++i)
        thicknesses[i] /= Units::instance()->toMeters();
    
    return thicknesses;
}

void LayerThicknessVariation::fromJson(const QJsonObject &json)
{
    _enabled = json["enabled"].toBool();
    int model = json["model"].toInt();
    setModel(model);
    if (_model == LayerThicknessVariation::Custom)
    {
        _coeff = json["coeff"].toDouble();
        _initial = json["initial"].toDouble();
        _exponent = json["exponent"].toDouble();
    }
}

QJsonObject LayerThicknessVariation::toJson() const
{
    QJsonObject json;
    json["enabled"] = _enabled;
    json["model"] = (int) _model;
    if (_model == LayerThicknessVariation::Custom) {
        json["coeff"] = _coeff;
        json["initial"] = _initial;
        json["exponent"] = _exponent;
    }
    return json;
}

QDataStream & operator<< (QDataStream & out, const LayerThicknessVariation* ltv)
{
    out << (quint8)1;

    out << ltv->_enabled
        << (int)ltv->_model;

    if (ltv->_model == LayerThicknessVariation::Custom) {
        out << ltv->_coeff
            << ltv->_initial
            << ltv->_exponent;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, LayerThicknessVariation* ltv)
{
    quint8 ver;
    in >> ver;

    int model;

    in >> ltv->_enabled
            >> model;

    ltv->setModel(model);
    if (ltv->_model == LayerThicknessVariation::Custom) {
        in >> ltv->_coeff
           >> ltv->_initial
           >> ltv->_exponent;
    }

    return in;
}
