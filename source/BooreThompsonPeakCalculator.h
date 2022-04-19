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

#ifndef BOORETHOMPSONPEAKCALCULATOR_H
#define BOORETHOMPSONPEAKCALCULATOR_H

#include "AbstractRvtMotion.h"
#include "VanmarckePeakCalculator.h"

#include <QMap>
#include <QString>
#include <QVector>

#include <gsl/gsl_interp2d.h>

class BooreThompsonPeakCalculator : public VanmarckePeakCalculator {
public:
  explicit BooreThompsonPeakCalculator();
  ~BooreThompsonPeakCalculator();

  void setScenario(double mag, double dist, AbstractRvtMotion::Region region);

  auto mag() const -> double;
  auto dist() const -> double;
  auto region() const -> AbstractRvtMotion::Region;

protected:
  virtual auto
  calcDurationRms(double duration, double oscFreq, double oscDamping,
                  const QVector<std::complex<double>> &siteTransFunc) -> double;

  auto interpCoeff(double mag, double lnDist,
                   QMap<QString, QVector<double>> &data, const QString &key)
      -> double;

  // Interpolation and dimensions
  gsl_interp2d *_interp;
  gsl_interp_accel *_magAcc;
  gsl_interp_accel *_lnDistAcc;

  int _nmags;
  int _ndists;

  // Model coefficients
  QMap<QString, QMap<QString, QVector<double>>> _tabularData;

  // Selected model coefficients
  QMap<QString, double> _interped;

  // Scenario parameters
  double _mag;
  double _dist;
  AbstractRvtMotion::Region _region;
};

#endif // BOORETHOMPSONPEAKCALCULATOR_H
