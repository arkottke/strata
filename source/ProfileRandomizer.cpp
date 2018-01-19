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

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

#include "ProfileRandomizer.h"

#include "BedrockDepthVariation.h"
#include "Distribution.h"
#include "LayerThicknessVariation.h"
#include "RockLayer.h"
#include "SoilProfile.h"
#include "SoilLayer.h"
#include "Units.h"
#include "VelocityVariation.h"

#include <QObject>
#include <QDebug>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

ProfileRandomizer::ProfileRandomizer(gsl_rng * rng, SoilProfile* siteProfile)
    : _siteProfile(siteProfile)
{
    connect(_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateEnabled()));

    _bedrockDepthVariation = new BedrockDepthVariation(rng, this);
    connect(_bedrockDepthVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    _layerThicknessVariation = new LayerThicknessVariation(rng, this);
    connect(_layerThicknessVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    _velocityVariation = new VelocityVariation(rng, this);
    connect(_velocityVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    _enabled = false;
}

ProfileRandomizer::~ProfileRandomizer()
{
    _bedrockDepthVariation->deleteLater();
    _layerThicknessVariation->deleteLater();
    _velocityVariation->deleteLater();
}

bool ProfileRandomizer::enabled() const
{
    return _siteProfile->isVaried() && _enabled;
}

void ProfileRandomizer::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();
    }
}

void ProfileRandomizer::updateEnabled()
{
    emit enabledChanged(enabled());
}

BedrockDepthVariation* ProfileRandomizer::bedrockDepthVariation()
{
    return _bedrockDepthVariation;
}

LayerThicknessVariation* ProfileRandomizer::layerThicknessVariation()
{
    return _layerThicknessVariation;
}

VelocityVariation* ProfileRandomizer::velocityVariation()
{
    return _velocityVariation;
}

void ProfileRandomizer::fromJson(const QJsonObject &json)
{
    _bedrockDepthVariation->fromJson(json["bedrockDepthVariation"].toObject());
    _layerThicknessVariation->fromJson(json["layerThicknessVariation"].toObject());
    _velocityVariation->fromJson(json["velocityVariation"].toObject());

    bool enabled = json["enabled"].toBool();
    setEnabled(enabled);
}

QJsonObject ProfileRandomizer::toJson() const
{
    QJsonObject json;
    json["enabled"] = _enabled;

    json["bedrockDepthVariation"] = _bedrockDepthVariation->toJson();
    json["layerThicknessVariation"] = _layerThicknessVariation->toJson();
    json["velocityVariation"] = _velocityVariation->toJson();
    return json;
}

QDataStream & operator<< (QDataStream & out, const ProfileRandomizer* pv)
{
    out << (quint8)1;

    out << pv->_enabled
        << pv->_velocityVariation
        << pv->_layerThicknessVariation
        << pv->_bedrockDepthVariation;

    return out;
}

QDataStream & operator>> (QDataStream & in, ProfileRandomizer* pv)
{
    quint8 ver;
    in >> ver;

    bool enabled;

    in >> enabled
       >> pv->_velocityVariation
       >> pv->_layerThicknessVariation
       >> pv->_bedrockDepthVariation;

    pv->setEnabled(enabled);

    return in;
}
