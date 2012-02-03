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

#include "MaxStrainProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

MaxStrainProfileOutput::MaxStrainProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    m_offset = 1;
}

QString MaxStrainProfileOutput::name() const
{
    return tr("Maximum Shear-Strain Profile");
}

QString MaxStrainProfileOutput::shortName() const
{
    return tr("strain");
}

const QString MaxStrainProfileOutput::xLabel() const
{
    return tr("Maximum Shear Strain (%)");
}

void MaxStrainProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();
    // Uses depth from the center of the layer
    ref.clear();

    ref << 0;
    foreach (const SubLayer & sl, subLayers)
        ref << sl.depthToMid();

    ref << subLayers.last().depthToBase();

    data = calculator->site()->maxShearStrainProfile();    
    data.prepend(0);

    extrap(ref, data, subLayers.last().thickness());
}
