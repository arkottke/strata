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

#include "StressTimeSeriesOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

StressTimeSeriesOutput::StressTimeSeriesOutput(OutputCatalog *catalog)
    : AbstractTimeSeriesOutput(catalog) {
  _type = AbstractMotion::Within;
}

auto StressTimeSeriesOutput::name() const -> QString {
  return tr("Shear-Stress Time Series");
}

auto StressTimeSeriesOutput::shortName() const -> QString {
  return tr("stressTs");
}

auto StressTimeSeriesOutput::yLabel() const -> const QString {
  return tr("Shear Stress, %1 (%2)")
      .arg(QChar(0x03C4))
      .arg(Units::instance()->stress());
}

void StressTimeSeriesOutput::extract(AbstractCalculator *const calculator,
                                     QVector<double> &ref,
                                     QVector<double> &data) const {
  Q_UNUSED(ref);

  const auto *tsm = static_cast<const TimeSeriesMotion *>(calculator->motion());

  Q_ASSERT(tsm);

  Location loc = calculator->site()->depthToLocation(_depth);

  data = tsm->strainTimeSeries(
      calculator->calcStrainTf(calculator->site()->inputLocation(),
                               calculator->motion()->type(), loc),
      _baselineCorrect);

  // Convert to appropriate units
  const double shearMod = calculator->site()->shearMod(loc.layer());

  for (int i = 0; i < data.size(); ++i)
    data[i] *= shearMod;
}
