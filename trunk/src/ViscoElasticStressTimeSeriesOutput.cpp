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

#include "ViscoElasticStressTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

ViscoElasticStressTimeSeriesOutput::ViscoElasticStressTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractTimeSeriesOutput(catalog)
{
    m_type = AbstractMotion::Within;
}

QString ViscoElasticStressTimeSeriesOutput::name() const
{
    return tr("Visco-Elastic Shear-Stress Time Series");
}

QString ViscoElasticStressTimeSeriesOutput::shortName() const
{
    return tr("viscElasticStressTs");
}

const QString ViscoElasticStressTimeSeriesOutput::yLabel() const
{
    return tr("Visco-Elastic Shear Stress, %1 (%2)")
            .arg(QChar(0x03C4))
            .arg(Units::instance()->stress());
}


void ViscoElasticStressTimeSeriesOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const TimeSeriesMotion* tsm = static_cast<const TimeSeriesMotion*>(calculator->motion());

    Q_ASSERT(tsm);

    Location loc = calculator->site()->depthToLocation(m_depth);

    data = tsm->strainTimeSeries(
            calculator->calcStressTf(
                    calculator->site()->inputLocation(),
                    calculator->motion()->type(), loc), m_baselineCorrect);
}
