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

#include "StrainTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

StrainTimeSeriesOutput::StrainTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractTimeSeriesOutput(catalog)
{
    _type = AbstractMotion::Within;
}

auto StrainTimeSeriesOutput::name() const -> QString
{
    return tr("Shear-Strain Time Series");
}

auto StrainTimeSeriesOutput::shortName() const -> QString
{
    return tr("strainTs");
}

auto StrainTimeSeriesOutput::yLabel() const -> const QString
{
    return tr("Shear Strain, %1 (%)")
            .arg(QChar(0x03B3));
}

void StrainTimeSeriesOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const auto* tsm = static_cast<const TimeSeriesMotion*>(calculator->motion());

    Q_ASSERT(tsm);

    data = tsm->strainTimeSeries(calculator->calcStrainTf(
            calculator->site()->inputLocation(), calculator->motion()->type(),
            calculator->site()->depthToLocation(_depth)), _baselineCorrect);

    // Convert to percent
    for (int i = 0; i < data.size(); ++i)
        data[i] *= 100;
}
