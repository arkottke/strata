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

#include "MyQwtCompatibility.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double MinPositiveLogLimit = 1e-12;
constexpr double MaxLogDecades = 12.0;

void sanitizeScaleBounds(double &x1, double &x2);

#if QWT_VERSION < 0x060100
class SafeLogScaleEngine : public QwtLog10ScaleEngine {
public:
  void autoScale(int maxNumSteps, double &x1, double &x2,
                 double &stepSize) const override {
    QwtLog10ScaleEngine::autoScale(maxNumSteps, x1, x2, stepSize);
    sanitizeScaleBounds(x1, x2);
  }
};
#else
class SafeLogScaleEngine : public QwtLogScaleEngine {
public:
  SafeLogScaleEngine() : QwtLogScaleEngine(10) {}

  void autoScale(int maxNumSteps, double &x1, double &x2,
                 double &stepSize) const override {
    QwtLogScaleEngine::autoScale(maxNumSteps, x1, x2, stepSize);
    sanitizeScaleBounds(x1, x2);
  }
};
#endif

void sanitizeScaleBounds(double &x1, double &x2) {
  if (!std::isfinite(x1) || !std::isfinite(x2)) {
    x1 = MinPositiveLogLimit;
    x2 = 1.0;
    return;
  }

  if (x2 < x1)
    std::swap(x1, x2);

  if (x2 <= 0.0) {
    x1 = MinPositiveLogLimit;
    x2 = 1.0;
    return;
  }

  const double minFromUpper = x2 / std::pow(10.0, MaxLogDecades);
  x1 = std::max(x1, std::max(MinPositiveLogLimit, minFromUpper));

  if (x2 <= x1)
    x2 = x1 * 10.0;
}

} // namespace

auto logScaleEngine() -> QwtScaleEngine * { return new SafeLogScaleEngine; }
