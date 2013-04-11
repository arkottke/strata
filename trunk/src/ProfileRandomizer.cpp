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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

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

#include <cmath>
#include <algorithm>

ProfileRandomizer::ProfileRandomizer(gsl_rng * rng, SoilProfile* siteProfile)
    : m_siteProfile(siteProfile)
{
    connect(m_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateEnabled()));

    m_bedrockDepthVariation = new BedrockDepthVariation(rng, this);
    connect(m_bedrockDepthVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    m_layerThicknessVariation = new LayerThicknessVariation(rng, this);
    connect(m_layerThicknessVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    m_velocityVariation = new VelocityVariation(rng, this);
    connect(m_velocityVariation, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    m_enabled = false;
}

ProfileRandomizer::~ProfileRandomizer()
{
    m_bedrockDepthVariation->deleteLater();
    m_layerThicknessVariation->deleteLater();
    m_velocityVariation->deleteLater();
}

bool ProfileRandomizer::enabled() const
{
    return m_siteProfile->isVaried() && m_enabled;
}

void ProfileRandomizer::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

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
    return m_bedrockDepthVariation;
}

LayerThicknessVariation* ProfileRandomizer::layerThicknessVariation()
{
    return m_layerThicknessVariation;
}

VelocityVariation* ProfileRandomizer::velocityVariation()
{
    return m_velocityVariation;
}

QDataStream & operator<< (QDataStream & out, const ProfileRandomizer* pv)
{
    out << (quint8)1;

    out << pv->m_enabled
        << pv->m_velocityVariation
        << pv->m_layerThicknessVariation
        << pv->m_bedrockDepthVariation;

    return out;
}

QDataStream & operator>> (QDataStream & in, ProfileRandomizer* pv)
{
    quint8 ver;
    in >> ver;

    bool enabled;

    in >> enabled
       >> pv->m_velocityVariation
       >> pv->m_layerThicknessVariation
       >> pv->m_bedrockDepthVariation;

    pv->setEnabled(enabled);

    return in;
}
