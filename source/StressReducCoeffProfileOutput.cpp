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

#include "StressReducCoeffProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

StressReducCoeffProfileOutput::StressReducCoeffProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{

}

QString StressReducCoeffProfileOutput::name() const
{

    return tr("Stress Reduction Coefficient (r_d) Profile");
}

QString StressReducCoeffProfileOutput::shortName() const
{
    return tr("stressReducCoeff");
}

const QString StressReducCoeffProfileOutput::xLabel() const
{
    return tr("Stress Reduction Coefficient (r_d)");
}

void StressReducCoeffProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();

    // Populate the reference with the depth to the top of the layers
    ref.clear();

    ref << 0;
    foreach (const SubLayer & sl, subLayers)
        ref << sl.depthToMid();

    // Add the depth at the surface of the bedrock
    ref << subLayers.last().depthToBase();

    data = calculator->site()->stressReducCoeffProfile(calculator->surfacePGA());
}
