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

#ifndef ABSTRACT_TIME_SERIES_OUTPUT_H
#define ABSTRACT_TIME_SERIES_OUTPUT_H

#include "AbstractLocationOutput.h"

#include <QDataStream>
#include <QJsonObject>

class OutputCatalog;

class AbstractTimeSeriesOutput : public AbstractLocationOutput {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const AbstractTimeSeriesOutput *atso)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, AbstractTimeSeriesOutput *atso)
      -> QDataStream &;

public:
  explicit AbstractTimeSeriesOutput(OutputCatalog *catalog);

  virtual auto fullName() const -> QString;

  virtual auto needsTime() const -> bool;

  auto baselineCorrect() const -> bool;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

public slots:
  void setBaselineCorrect(bool baseLineCorrect);

signals:
  void baselineCorrectChanged(int baseLineCorrect);

protected:
  virtual auto fileName(int motion = 0) const -> QString;
  virtual auto xScaleEngine() const -> QwtScaleEngine *;
  virtual auto yScaleEngine() const -> QwtScaleEngine *;
  virtual auto xLabel() const -> const QString;
  virtual auto ref(int motion) const -> const QVector<double> &;
  virtual auto suffix() const -> const QString;

  //! Field width of the motion index
  auto fieldWidth() const -> int;

  //! If the time series is baseline corrected
  bool _baselineCorrect;
};
#endif // ABSTRACT_TIME_SERIES_OUTPUT_H
