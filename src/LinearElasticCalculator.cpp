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

#include "LinearElasticCalculator.h"

#include "SoilProfile.h"

LinearElasticCalculator::LinearElasticCalculator(QObject *parent)
    : AbstractCalculator(parent)
{
}

bool LinearElasticCalculator::run(AbstractMotion *motion, SoilProfile *site)
{
    init(motion, site);

    // Complex shear modulus for all layers.
    // The shear modulus is constant over the frequency range.
    for (int i = 0; i < m_shearMod.size(); ++i)
        m_shearMod[i].fill(calcCompShearMod(m_site->shearMod(i), m_site->damping(i) / 100.));

    // Compute upgoing and downgoing waves
    return calcWaves();
}
