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

#include "ResponseSpectrumOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Dimension.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilProfile.h"
#include "Units.h"

#include <QDebug>

ResponseSpectrumOutput::ResponseSpectrumOutput(OutputCatalog *catalog)
    : AbstractLocationOutput(catalog) {
  _statistics = new OutputStatistics(this);
  connect(_statistics, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
}

auto ResponseSpectrumOutput::needsPeriod() const -> bool { return true; }

auto ResponseSpectrumOutput::name() const -> QString {
  return tr("Acceleration Response Spectrum");
}

auto ResponseSpectrumOutput::shortName() const -> QString {
  return tr("respSpec");
}

auto ResponseSpectrumOutput::xScaleEngine() const -> QwtScaleEngine * {
  return logScaleEngine();
}

auto ResponseSpectrumOutput::yScaleEngine() const -> QwtScaleEngine * {
  return logScaleEngine();
}

auto ResponseSpectrumOutput::xLabel() const -> const QString {
  return tr("Period (s)");
}

auto ResponseSpectrumOutput::yLabel() const -> const QString {
  return tr("Spectral Accel. (g)");
}

auto ResponseSpectrumOutput::ref(int motion) const -> const QVector<double> & {
  Q_UNUSED(motion);

  return _catalog->period()->data();
}

void ResponseSpectrumOutput::extract(AbstractCalculator *const calculator,
                                     QVector<double> &ref,
                                     QVector<double> &data) const {
  Q_UNUSED(ref);

  data = calculator->motion()->computeSa(
      _catalog->period()->data(), _catalog->damping(),
      calculator->calcAccelTf(
          calculator->site()->inputLocation(), calculator->motion()->type(),
          calculator->site()->depthToLocation(_depth), _type));
}
