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

#include "AccelTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

AccelTimeSeriesOutput::AccelTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractTimeSeriesOutput(catalog)
{

}

auto AccelTimeSeriesOutput::name() const -> QString
{
    return tr("Acceleration Time Series");
}

auto AccelTimeSeriesOutput::shortName() const -> QString
{
    return tr("accelTs");
}

auto AccelTimeSeriesOutput::yLabel() const -> const QString
{
    return tr("Acceleration (%1)").arg(Units::instance()->accel());
}

void AccelTimeSeriesOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const auto* motion = qobject_cast<const TimeSeriesMotion*>(calculator->motion());
    Q_ASSERT(motion);

    data = motion->timeSeries(TimeSeriesMotion::Acceleration,
                              calculator->calcAccelTf(
                                      calculator->site()->inputLocation(), motion->type(),
                                      calculator->site()->depthToLocation(_depth), _type),
                              _baselineCorrect);
}
