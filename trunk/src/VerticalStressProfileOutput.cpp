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

#include "VerticalStressProfileOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

VerticalStressProfileOutput::VerticalStressProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    // Skip the zero at the surface
    m_offset = 1;
}

QString VerticalStressProfileOutput::name() const
{
    return tr("Vertical Stress Profile");
}

QString VerticalStressProfileOutput::shortName() const
{
    return tr("vStress");
}

const QString VerticalStressProfileOutput::xLabel() const
{
    return tr("Vertical Stress (%1)").arg(Units::instance()->stress());
}


void VerticalStressProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();
    // Populate the reference with the depth to middle of the layers
    ref.clear();
    data.clear();
    ref << 0.;
    data << 0.;

    foreach (const SubLayer & sl, subLayers) {
        ref << sl.depthToMid();
        data << sl.vTotalStress();
    }

    // Add the depth at the surface of the bedrock
    ref << subLayers.last().depthToBase();
    data << subLayers.last().vTotalStress(1.0);
}
