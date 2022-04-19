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

#ifndef WANG_RATHJE_PEAK_CALCULATOR_H
#define WANG_RATHJE_PEAK_CALCULATOR_H

#include "BooreThompsonPeakCalculator.h"

struct WangRathjeCoef {
  int mode;
  double a;
  double b;
  double d;
  double e;
  double sd;
};

class WangRathjePeakCalculator : public BooreThompsonPeakCalculator {

public:
  WangRathjePeakCalculator();

  virtual auto
  calcDurationRms(double duration, double oscFreq, double oscDamping,
                  const QVector<std::complex<double>> &siteTransFunc) -> double;

protected:
  QList<WangRathjeCoef> _coefs;
};

#endif // WANG_RATHJE_PEAK_CALCULATOR_H
