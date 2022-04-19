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

#ifndef OUTPUTSTATISTICS_H
#define OUTPUTSTATISTICS_H

#include <QObject>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class AbstractOutput;

class OutputStatistics : public QObject {
  Q_OBJECT

public:
  explicit OutputStatistics(AbstractOutput *output);

  enum Distribution {
    Normal,   //!< Normal Distribution (average=mean)
    LogNormal //!< Log Normal Distribution (average=median, stdev=lnStd)
  };

  void calculate();
  void plot(QwtPlot *qwtPlot) const;

  //! If the output has enough data to compute statistics
  auto hasEnoughData() const -> bool;

  auto distribution() const -> Distribution;
  void setDistribution(Distribution distribution);

  auto averageLabel() const -> QString;
  auto stdevLabel() const -> QString;

  auto average() const -> const QVector<double> &;
  auto stdev() const -> const QVector<double> &;

public slots:
  void setDistribution(int distribution);

signals:
  void distributionChanged(int distribution);
  void wasModified();

protected slots:
  void clear();

protected:
  auto plotCurve(QwtPlot *const plot, const QVector<double> &data,
                 Qt::PenStyle penStyle) const -> QwtPlotCurve *;

  //! Parent AbstractOutput
  AbstractOutput *_output;

  //! Assumed distribution
  Distribution _distribution;

  //! Average value
  QVector<double> _average;

  //! Standard deviation of the mode
  QVector<double> _stdev;

  //! Median plus one standard deviation
  QVector<double> _plusStd;

  //! Median minus one standard deviation
  QVector<double> _minusStd;
};

#endif // OUTPUTSTATISTICS_H
