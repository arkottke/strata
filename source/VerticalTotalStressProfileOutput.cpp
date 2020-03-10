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

#include "VerticalTotalStressProfileOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

VerticalTotalStressProfileOutput::VerticalTotalStressProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    // Skip the zero at the surface
    _offset = 1;
}

auto VerticalTotalStressProfileOutput::name() const -> QString
{
    return tr("Vertical Total Stress Profile");
}

auto VerticalTotalStressProfileOutput::shortName() const -> QString
{
    return tr("vTotalStress");
}

auto VerticalTotalStressProfileOutput::xLabel() const -> const QString
{
    return tr("Vertical Total Stress (%1)").arg(Units::instance()->stress());
}


void VerticalTotalStressProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();
    // Populate the reference with the depth to middle of the layers
    ref.clear();
    data.clear();
    ref << 0.;
    data << 0.;

    for (const SubLayer &sl : subLayers) {
        ref << sl.depthToMid();
        data << sl.vTotalStress();
    }

    // Add the depth at the surface of the bedrock
    ref << subLayers.last().depthToBase();
    data << subLayers.last().vTotalStress(1.0);
}
