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

#ifndef VANMARCKEPEAKCALCULATOR_H
#define VANMARCKEPEAKCALCULATOR_H

#include "AbstractPeakCalculator.h"

#include <gsl/gsl_integration.h>

class VanmarckePeakCalculator : public AbstractPeakCalculator {
public:
  explicit VanmarckePeakCalculator();
  ~VanmarckePeakCalculator();

protected:
  auto calcPeakFactor(double duration, double oscFreq, double oscDamping)
      -> double;

  gsl_integration_workspace *_workspace;
};

#endif // VANMARCKEPEAKCALCULATOR_H
