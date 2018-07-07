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

#include <QDebug>

#include "LinearElasticCalculator.h"

#include "RockLayer.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

LinearElasticCalculator::LinearElasticCalculator(QObject *parent)
    : AbstractCalculator(parent)
{
}

bool LinearElasticCalculator::run(AbstractMotion *motion, SoilProfile *site)
{
    init(motion, site);

    // Complex shear modulus for all layers.
    // The shear modulus is constant over the frequency range.
    for (int i = 0; i < _nsl; ++i)
        _shearMod[i].fill(calcCompShearMod(_site->shearMod(i), _site->damping(i) / 100.));

    // Compute the bedrock properties -- these do not change during the process.
    // The shear modulus is constant over the frequency range.
    _shearMod[_nsl].fill(calcCompShearMod(
            _site->bedrock()->shearMod(), _site->bedrock()->damping() / 100.));

    // Compute upgoing and downgoing waves
    bool success = calcWaves();

    if (success) {
        QVector<std::complex<double> > strainTf;

        // Compute the maximum strain predicted in the layers
        for (int i = 0; i < _nsl; ++i) {
            strainTf = calcStrainTf(_site->inputLocation(), _motion->type(),
                                    Location(i, _site->subLayers().at(i).thickness() / 2));
            // Compute maximum shear strain in percent
            const double strainMax = 100 * _motion->calcMaxStrain(strainTf);

            _site->subLayers()[i].setStrain(strainMax, strainMax, false);
        }
    }

    return success;
}
