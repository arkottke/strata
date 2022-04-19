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

#ifndef STRESS_TIME_SERIES_OUTPUT_H
#define STRESS_TIME_SERIES_OUTPUT_H

#include "AbstractTimeSeriesOutput.h"

class AbstractCalculator;

class StressTimeSeriesOutput : public AbstractTimeSeriesOutput {
  Q_OBJECT
public:
  explicit StressTimeSeriesOutput(OutputCatalog *catalog);

  virtual auto name() const -> QString;

protected:
  virtual auto shortName() const -> QString;
  virtual auto yLabel() const -> const QString;

  void extract(AbstractCalculator *const calculator, QVector<double> &ref,
               QVector<double> &data) const;
};
#endif // STRESS_TIME_SERIES_OUTPUT_H
