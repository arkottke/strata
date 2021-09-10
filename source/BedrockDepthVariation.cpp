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

#include "BedrockDepthVariation.h"

#include "ProfileRandomizer.h"

BedrockDepthVariation::BedrockDepthVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    Distribution(rng), _profileRandomizer(profileRandomizer)
{
    connect(_profileRandomizer, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateEnabled()));

    _enabled = false;
}

auto BedrockDepthVariation::enabled() const -> bool
{
    return _profileRandomizer->enabled() && _enabled;
}

void BedrockDepthVariation::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();
    }
}

void BedrockDepthVariation::updateEnabled()
{
    emit enabledChanged(enabled());
}

void BedrockDepthVariation::fromJson(const QJsonObject &json)
{
    Distribution::fromJson(json);
    bool enabled = json["enabled"].toBool();
    setEnabled(enabled);
}

auto BedrockDepthVariation::toJson() const -> QJsonObject
{
    QJsonObject json = Distribution::toJson();
    json["enabled"] = _enabled;
    return json;
}


auto operator<< (QDataStream & out, const BedrockDepthVariation* bdv) -> QDataStream &

{
    out << (quint8)1;

    out << qobject_cast<const Distribution*>(bdv)
        << bdv->_enabled;
    return out;
}

auto operator>> (QDataStream & in, BedrockDepthVariation* bdv) -> QDataStream &
{
    quint8 ver;
    in >> ver;


    bool enabled;
    in >> qobject_cast<Distribution*>(bdv)
       >> enabled;

    bdv->setEnabled(enabled);

    return in;
}
