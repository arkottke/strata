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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef OUTPUTSTATISTICS_H
#define OUTPUTSTATISTICS_H

#include <QObject>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class AbstractOutput;

class OutputStatistics : public QObject
{
    Q_OBJECT

public:
    explicit OutputStatistics(AbstractOutput* output);

    enum Distribution {
        Normal, //!< Normal Distribution (average=mean)
        LogNormal //!< Log Normal Distribution (average=median, stdev=lnStd)
    };

    void calculate();
    void plot(QwtPlot* qwtPlot) const;

    //! If the output has enough data to compute statistics
    bool hasEnoughData() const;

    Distribution distribution() const;
    void setDistribution(Distribution distribution);

    QString averageLabel() const;
    QString stdevLabel() const;

    const QVector<double>& average() const;
    const QVector<double>& stdev() const;

public slots:
    void setDistribution(int distribution);

signals:
    void distributionChanged(int distribution);
    void wasModified();

protected slots:
    void clear();

protected:
    QwtPlotCurve* plotCurve(QwtPlot* const plot, const QVector<double> & data, Qt::PenStyle penStyle) const;

    //! Parent AbstractOutput
    AbstractOutput* m_output;

    //! Assumed distribution
    Distribution m_distribution;

    //! Average value
    QVector<double> m_average;

    //! Standard deviation of the mode
    QVector<double> m_stdev;

    //! Median plus one standard deviation
    QVector<double> m_plusStd;

    //! Median minus one standard deviation
    QVector<double> m_minusStd;
};

#endif // OUTPUTSTATISTICS_H
