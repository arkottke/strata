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

#include "DispTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

DispTimeSeriesOutput::DispTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractTimeSeriesOutput(catalog)
{

}

QString DispTimeSeriesOutput::name() const
{
    return tr("Displacement Time Series");
}

QString DispTimeSeriesOutput::shortName() const
{
    return tr("dispTs");
}

const QString DispTimeSeriesOutput::yLabel() const
{
    return tr("Displacement (%1)").arg(Units::instance()->dispTs());
}


void DispTimeSeriesOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const TimeSeriesMotion* motion = static_cast<const TimeSeriesMotion*>(calculator->motion());

    data = motion->timeSeries(TimeSeriesMotion::Displacement,
                              calculator->calcAccelTf(
                                      calculator->site()->inputLocation(), motion->type(),
                                      calculator->site()->depthToLocation(m_depth), m_type),
                              m_baselineCorrect);
}
