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

#include "VelTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

VelTimeSeriesOutput::VelTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractTimeSeriesOutput(catalog)
{

}

QString VelTimeSeriesOutput::name() const
{
    return tr("Velocity Time Series");
}

QString VelTimeSeriesOutput::shortName() const
{
    return tr("velTs");
}

const QString VelTimeSeriesOutput::yLabel() const
{
    return tr("Velocity (%1)").arg(Units::instance()->velTs());
}


void VelTimeSeriesOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const TimeSeriesMotion* motion = static_cast<const TimeSeriesMotion*>(calculator->motion());

    data = motion->timeSeries(TimeSeriesMotion::Velocity,
                              calculator->calcAccelTf(
                                      calculator->site()->inputLocation(), motion->type(),
                                      calculator->site()->depthToLocation(m_depth), m_type),
                              m_baselineCorrect);
}
