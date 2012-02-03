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

#include "DissipatedEnergyProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

DissipatedEnergyProfileOutput::DissipatedEnergyProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    m_offset = 1;
}

QString DissipatedEnergyProfileOutput::name() const
{
    return tr("Dissipated Energy Profile");
}

QString DissipatedEnergyProfileOutput::shortName() const
{
    return tr("dissipatedEnergy");
}

const QString DissipatedEnergyProfileOutput::xLabel() const
{
    return tr("Dissipated Energy (??)");
}

void DissipatedEnergyProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();

    const TimeSeriesMotion* tsm = static_cast<const TimeSeriesMotion*>(calculator->motion());

    // Uses depth from the center of the layer
    ref.clear();
    data.clear();
    
    // No values at the surface
    ref << 0;
    data << 0;
    
    // Gravity is needed to convert the strain time series into meaningful values
    double gravity = Units::instance()->gravity();

    foreach (const SubLayer & sl, subLayers) {
        const double depth = sl.depthToMid();

        // Compute the strain and visco-elastic stress time series without baseline correction
        const QVector<double> strainTs = tsm->strainTimeSeries(calculator->calcStrainTf(
                calculator->site()->inputLocation(), calculator->motion()->type(),
                calculator->site()->depthToLocation(depth)), false);

        const QVector<double> stressTs = tsm->strainTimeSeries(calculator->calcStressTf(
                calculator->site()->inputLocation(), calculator->motion()->type(),
                calculator->site()->depthToLocation(depth)), false);

        // Integrate the loop using the trapezoid rule
        double sum = 0;

        for (int i = 1; i < strainTs.size(); ++i)
            sum += 0.5 * (stressTs.at(i) + stressTs.at(i-1)) 
                    * (strainTs.at(i) - strainTs.at(i-1));
        
        // Save the values
        ref << depth;
        data << sum;
    }

    ref << subLayers.last().depthToBase();
    // Linearly extrap for the final data value
    extrap(ref, data, subLayers.last().thickness());
}
