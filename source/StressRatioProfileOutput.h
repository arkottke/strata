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

#ifndef STRESS_RATIO_PROFILE_OUTPUT_H
#define STRESS_RATIO_PROFILE_OUTPUT_H

#include "AbstractProfileOutput.h"

class AbstractCalculator;

class StressRatioProfileOutput : public AbstractProfileOutput {
  Q_OBJECT
public:
  explicit StressRatioProfileOutput(OutputCatalog *catalog);

  virtual auto name() const -> QString;

protected:
  virtual auto shortName() const -> QString;
  virtual auto xLabel() const -> const QString;

  void extract(AbstractCalculator *const calculator, QVector<double> &ref,
               QVector<double> &data) const;
};

#endif // STRESS_RATIO_PROFILE_OUTPUT_H
