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
//
#ifndef ABSTRACTPEAKCALCULATOR_H
#define ABSTRACTPEAKCALCULATOR_H

#include <QMap>
#include <QString>
#include <QVector>

#include <complex>

class AbstractPeakCalculator {
public:
  explicit AbstractPeakCalculator();
  virtual ~AbstractPeakCalculator();

  auto calcPeak(double duration, const QVector<double> &freqs,
                const QVector<double> &fourierAmps, double oscFreq = 0,
                double oscDamping = 0,
                const QVector<std::complex<double>> &siteTransFunc =
                    QVector<std::complex<double>>()) -> double;

  virtual auto
  calcDurationRms(double duration, double oscFreq, double oscDamping,
                  const QVector<std::complex<double>> &siteTransFunc) -> double;

protected:
  virtual auto calcPeakFactor(double duration, double oscFreq,
                              double oscDamping) -> double = 0;

  void initCache(const QVector<double> &freqs,
                 const QVector<double> &fourierAmps);
  void clearCache();

  auto getMoment(int power) -> double;

  auto limitZeroCrossings(double) const -> double;

  QString _name;

  QVector<double> _freqs;
  QVector<double> _squaredAmps;

  QMap<int, double> _momentCache;
};

#endif // ABSTRACTPEAKCALCULATOR_H
