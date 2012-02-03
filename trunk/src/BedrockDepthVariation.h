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

#ifndef BEDROCK_DEPTH_VARIATION_H
#define BEDROCK_DEPTH_VARIATION_H

#include "Distribution.h"

#include <gsl/gsl_rng.h>

class ProfileRandomizer;

class BedrockDepthVariation : public Distribution
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const BedrockDepthVariation* bdv);
    friend QDataStream & operator>> (QDataStream & in, BedrockDepthVariation* bdv);

public:
    explicit BedrockDepthVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer);

    bool enabled() const;

signals:
    void enabledChanged(bool enabled);

public slots:
    void setEnabled(bool enabled);

protected slots:
    void updateEnabled();

private:
    //! Enable variation of the profile
    bool m_enabled;

    //! Profile randomizer that controls if the option is available
    ProfileRandomizer* m_profileRandomizer;
};

#endif // BEDROCK_DEPTH_VARIATION_H
