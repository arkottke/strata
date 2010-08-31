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

#include "BedrockDepthVariation.h"

#include "ProfileRandomizer.h"

BedrockDepthVariation::BedrockDepthVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    Distribution(rng), m_profileRandomizer(profileRandomizer)
{
    connect(m_profileRandomizer, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateEnabled()));

    m_enabled = false;
}

bool BedrockDepthVariation::enabled() const
{
    return m_profileRandomizer->enabled() && m_enabled;
}

void BedrockDepthVariation::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();
    }
}

void BedrockDepthVariation::updateEnabled()
{
    emit enabledChanged(enabled());
}


QDataStream & operator<< (QDataStream & out, const BedrockDepthVariation* bdv)

{
    out << (quint8)1;

    out << qobject_cast<const Distribution*>(bdv)
            << bdv->m_enabled;

    return out;
}

QDataStream & operator>> (QDataStream & in, BedrockDepthVariation* bdv)
{
    quint8 ver;
    in >> ver;


    bool enabled;
    in >> qobject_cast<Distribution*>(bdv)
            >> enabled;

    bdv->setEnabled(enabled);

    return in;
}
