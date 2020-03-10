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

#include "VelocityVariation.h"

#include "ProfileRandomizer.h"

#include "RockLayer.h"
#include "SoilLayer.h"
#include "Units.h"

#include <gsl/gsl_randist.h>

#include <cfloat>
#include <cmath>

VelocityVariation::VelocityVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    _enabled(false),
    _stdevModel(Custom),
    _stdevIsLayerSpecific(false),
    _stdev(0),
    _correlModel(Custom),
    _rng(rng), 
    _correlInitial(0),
    _correlFinal(0),
    _correlDelta(0),
    _correlIntercept(0),
    _correlExponent(0),
    _profileRandomizer(profileRandomizer)
{
    connect(_profileRandomizer, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateEnabled()));
    setStdevModel(USGS_C);
    setCorrelModel(USGS_C);
}

auto VelocityVariation::modelList() -> QStringList
{
    QStringList list;

    list << tr("Custom")
        << tr("GeoMatrix A&B, Rock & Shallow Soil")
        << tr("GeoMatrix C&D, Deep Narrow/Broad Soil");

    if (Units::instance()->system() == Units::Metric ) {
        list << tr("USGS A&B, >360 m/s")
            << tr("USGS C&D, <360 m/s")
            << tr("USGS A, >750 m/s")
            << tr("USGS B, 360-750 m/s")
            << tr("USGS C, 180 to 360 m/s")
            << tr("USGS D, <180 m/s");
    } else {
        list << tr("USGS A&B, >1200 ft/s")
            << tr("USGS C&D, <1200 ft/s")
            << tr("USGS A, >2500 ft/s")
            << tr("USGS B, 1200-2500 ft/s")
            << tr("USGS C, 600 to 1200 ft/s")
            << tr("USGS D, <600 ft/s");
    }

    return list;
}

auto VelocityVariation::enabled() const -> bool
{
    return _profileRandomizer->enabled() && _enabled;
}

void VelocityVariation::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        emit enabledChanged(this->enabled());
        emit stdevIsLayerSpecificChanged(stdevIsLayerSpecific());
        emit wasModified();
    }
}

void VelocityVariation::updateEnabled()
{
    emit enabledChanged(enabled());
    emit stdevIsLayerSpecificChanged(stdevIsLayerSpecific());
}

auto VelocityVariation::stdevModel() const -> VelocityVariation::Model
{
    return _stdevModel;
}

auto VelocityVariation::stdevCustomEnabled() const -> bool
{
    return _stdevModel == Custom;
}

void VelocityVariation::setStdevModel(Model model)
{
    if (_stdevModel != model) {
        _stdevModel = model;

        switch (_stdevModel) {
        case Custom:
            break;
        case GeoMatrix_AB:
            setStdev(0.46);
            break;
        case GeoMatrix_CD:
            setStdev(0.38);
            break;
        case USGS_AB:
            setStdev(0.35);
            break;
        case USGS_CD:
            setStdev(0.36);
            break;
        case USGS_A:
            setStdev(0.36);
            break;
        case USGS_B:
            setStdev(0.27);
            break;
        case USGS_C:
            setStdev(0.31);
            break;
        case USGS_D:
            setStdev(0.37);
            break;
        }

        emit stdevModelChanged(_stdevModel);
        emit stdevCustomEnabledChanged(stdevCustomEnabled());
        emit wasModified();
    }
}

void VelocityVariation::setStdevModel(int model)
{
    setStdevModel((Model)model);
}

auto VelocityVariation::stdevIsLayerSpecific() const -> bool
{
    return _stdevIsLayerSpecific;
}

void VelocityVariation::setStdevIsLayerSpecific(bool b)
{
    if (_stdevIsLayerSpecific != b) {
        _stdevIsLayerSpecific = b;

        emit stdevIsLayerSpecificChanged(b);
        emit wasModified();
    }
}

auto VelocityVariation::stdev() const -> double
{
    return _stdev;
}

void VelocityVariation::setStdev(double stdev)
{
    if (abs(_stdev - stdev) > DBL_EPSILON) {
        _stdev = stdev;

        emit stdevChanged(_stdev);
        emit wasModified();
    }
}

auto VelocityVariation::correlModel() const -> VelocityVariation::Model
{
    return _correlModel;
}

auto VelocityVariation::correlCustomEnabled() const -> bool
{
    return _correlModel == Custom;
}

void VelocityVariation::setCorrelModel(Model model)
{
    if (_correlModel != model) {
        _correlModel = model;

        switch (_correlModel) {
        case Custom:
            break;
        case GeoMatrix_AB:
            setCorrelInitial(0.96);
            setCorrelFinal(0.96);
            setCorrelDelta(13.1);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.095);
            break;
        case GeoMatrix_CD:
            setCorrelInitial(0.99);
            setCorrelFinal(1.00);
            setCorrelDelta(8.0);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.160);
            break;
        case USGS_AB:
            setCorrelInitial(0.95);
            setCorrelFinal(1.00);
            setCorrelDelta(4.2);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.138);
            break;
        case USGS_CD:
            setCorrelInitial(0.99);
            setCorrelFinal(1.00);
            setCorrelDelta(3.9);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.293);
            break;
        case USGS_A:
            setCorrelInitial(0.95);
            setCorrelFinal(0.42);
            setCorrelDelta(3.4);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.063);
            break;
        case USGS_B:
            setCorrelInitial(0.97);
            setCorrelFinal(1.00);
            setCorrelDelta(3.8);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.293);
            break;
        case USGS_C:
            setCorrelInitial(0.99);
            setCorrelFinal(0.98);
            setCorrelDelta(3.9);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.344);
            break;
        case USGS_D:
            setCorrelInitial(0.00);
            setCorrelFinal(0.50);
            setCorrelDelta(5.0);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.744);
            break;
        }

        emit correlModelChanged(_correlModel);
        emit correlCustomEnabledChanged(correlCustomEnabled());
        emit wasModified();
    }
}

void VelocityVariation::setCorrelModel(int model)
{
    setCorrelModel((Model)model);
}

auto VelocityVariation::correlInitial() const -> double
{
    return _correlInitial;
}

void VelocityVariation::setCorrelInitial(double correlInitial)
{
    if (abs(_correlInitial - correlInitial) > DBL_EPSILON) {
        _correlInitial = correlInitial;

        emit correlInitialChanged(_correlInitial);
        emit wasModified();
    }
}

auto VelocityVariation::correlFinal() const -> double
{
    return _correlFinal;
}

void VelocityVariation::setCorrelFinal(double correlFinal)
{
    if (abs(_correlFinal - correlFinal) > DBL_EPSILON) {
        _correlFinal = correlFinal;

        emit correlFinalChanged(_correlFinal);
        emit wasModified();
    }
}

auto VelocityVariation::correlDelta() const -> double
{
    return _correlDelta;
}

void VelocityVariation::setCorrelDelta(double correlDelta)
{
    if (abs(_correlDelta - correlDelta) > DBL_EPSILON) {
        _correlDelta = correlDelta;

        emit correlDeltaChanged(_correlDelta);
        emit wasModified();
    }
}

auto VelocityVariation::correlIntercept() const -> double
{
    return _correlIntercept;
}

void VelocityVariation::setCorrelIntercept(double correlIntercept)
{
    if (abs(_correlIntercept - correlIntercept) > DBL_EPSILON) {
        _correlIntercept = correlIntercept;

        emit correlInterceptChanged(_correlIntercept);
        emit wasModified();
    }
}

auto VelocityVariation::correlExponent() const -> double
{
    return _correlExponent;
}

void VelocityVariation::setCorrelExponent(double correlExponent)
{
    if (abs(_correlExponent - correlExponent) > DBL_EPSILON) {
        _correlExponent = correlExponent;

        emit correlExponentChanged(_correlExponent);
        emit wasModified();
    }
}

void VelocityVariation::vary(QList<SoilLayer*> & soilLayers, RockLayer* bedrock) const
{
    // Random variable for the previous layer
    double prevRandVar = 0;
    // Current random variable and standard deviation
    double randVar = 0;
    double stdev = 0;

    const double toMeters = Units::instance()->toMeters();

    for (int i = 0; i < soilLayers.size(); ++i) {
        stdev = _stdevIsLayerSpecific ? soilLayers.at(i)->stdev() : _stdev;

        if (i == 0) {
            // First layer is not correlated
            randVar = gsl_ran_gaussian(_rng, stdev);
        } else {
            // Depth at the middle of the layer
            double depthToMid = soilLayers.at(i)->depth() + soilLayers.at(i)->thickness()/2.;

            // If the English units are used convert the depthToMid to meters
            depthToMid *= toMeters;

            // Depth dependent correlation
            const double dCorrel =  (depthToMid <= 200) ?
                                    _correlFinal * pow((depthToMid + _correlInitial)
                                                        / (200 + _correlInitial), _correlExponent)
                                                            : _correlFinal;
            // Thickness dependent correlation. Again convert to meters
            const double tCorrel = _correlInitial * exp( -toMeters *
                    soilLayers.at(i)->thickness() / _correlDelta );

            // Combine the correlations
            double correl = (1 - dCorrel) * tCorrel + dCorrel;

            // Compute the random variable taking into account the correlation from
            // the previous layer.
            randVar = correl * prevRandVar
                             + gsl_ran_gaussian(_rng, stdev) * sqrt( 1 - correl * correl);
        }

        if (soilLayers.at(i)->isVaried()) {
            // Vary the layer using the random variable
            soilLayers.at(i)->setVaried(soilLayers.at(i)->avg() * exp(randVar));
        } else {
            // Use average velocity for the layer
            soilLayers.at(i)->reset();
        }

        // Save the previous random variable
        prevRandVar = randVar;
    }

    // Randomize the bedrock layer
    //
    // The bedrock layer is assumed to be perfectly correlated to the previous
    // soil layer.  In addition the velocity is set to be at least as fast as
    // the velocity of the last soil layer.
    //
    if (bedrock->isVaried()) {
        double randVar = prevRandVar;

        // Adjust the random variable by the ratio of the standard deviation
        if (_stdevIsLayerSpecific) {
            randVar *= bedrock->stdev() / soilLayers.last()->stdev();
        }

        bedrock->setVaried(qMax(
                    bedrock->avg() * exp(randVar),
                    soilLayers.last()->shearVel()));

    } else {
        // Use average velocity for the layer
        bedrock->reset();
    }
}

void VelocityVariation::fromJson(const QJsonObject &json)
{
    _enabled = json["enabled"].toBool();

    setStdevModel(json["stdevModel"].toInt());
    if (_stdevModel == VelocityVariation::Custom) {
         _stdevIsLayerSpecific = json["stdevIsLayerSpecific"].toBool();
         _stdev = json["stdev"].toDouble();
    }

    setCorrelModel(json["correlModel"].toInt());
    if (_correlModel == VelocityVariation::Custom) {
        _correlInitial = json["correlInitial"].toDouble();
        _correlFinal = json["correlFinal"].toDouble();
        _correlDelta = json["correlDelta"].toDouble();
        _correlIntercept = json["correlIntercept"].toDouble();
        _correlExponent = json["correlExponent"].toDouble();
    }
}

auto VelocityVariation::toJson() const -> QJsonObject
{
    QJsonObject json;
    json["enabled"] = _enabled;
    json["stdevModel"] = (int) _stdevModel;
    json["correlModel"] = (int) _correlModel;

    if (_stdevModel == VelocityVariation::Custom) {
        json["stdevIsLayerSpecific"] = _stdevIsLayerSpecific;
        json["stdev"] = _stdev;
    }

    if (_correlModel == VelocityVariation::Custom) {
        json["correlInitial"] = _correlInitial;
        json["correlFinal"] = _correlFinal;
        json["correlDelta"] = _correlDelta;
        json["correlIntercept"] = _correlIntercept;
        json["correlExponent"] = _correlExponent;
    }

    return json;
}

auto operator<< (QDataStream & out, const VelocityVariation* vv) -> QDataStream &
{
    out << (quint8)1;

    out << vv->_enabled
        << (int)vv->_stdevModel;

    if (vv->_stdevModel == VelocityVariation::Custom) {
        out << vv->_stdevIsLayerSpecific
            << vv->_stdev;
    }

    out << (int)vv->_correlModel;

    if (vv->_correlModel == VelocityVariation::Custom) {
        out << vv->_correlInitial
            << vv->_correlFinal
            << vv->_correlDelta
            << vv->_correlIntercept
            << vv->_correlExponent;
    }

    return out;
}

auto operator>> (QDataStream & in, VelocityVariation* vv) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    int stdevModel;
    int correlModel;

    in >> vv->_enabled
       >> stdevModel;

    // Need to call set model to update values
    vv->setStdevModel(stdevModel);

    if (vv->_stdevModel == VelocityVariation::Custom) {
        in >> vv->_stdevIsLayerSpecific
           >> vv->_stdev;
    }

    in >> correlModel;
    vv->setCorrelModel(correlModel);

    if (vv->_correlModel == VelocityVariation::Custom) {
        in >> vv->_correlInitial
           >> vv->_correlFinal
           >> vv->_correlDelta
           >> vv->_correlIntercept
           >> vv->_correlExponent;
    }

    return in;
}
