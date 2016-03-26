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

#include "StressRatioProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"

#include <QChar>

StressRatioProfileOutput::StressRatioProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    m_offset = 1;
}

QString StressRatioProfileOutput::name() const
{
    return tr("Stress Ratio Profile");
}

QString StressRatioProfileOutput::shortName() const
{
    return tr("stressRatio");
}

const QString StressRatioProfileOutput::xLabel() const
{
    return tr("Stress Ratio, %1_max / %2_v").arg(QChar(0x03C4)).arg(QChar(0x03C3));
}


void StressRatioProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();
    // Uses depth from the center of the layer
    ref.clear();

    ref << 0;
    foreach (const SubLayer & sl, subLayers)
        ref << sl.depthToMid();

    ref << subLayers.last().depthToBase();

    data = calculator->site()->stressRatioProfile();
    data.prepend(0);

    extrap(ref, data, subLayers.last().thickness());
}
