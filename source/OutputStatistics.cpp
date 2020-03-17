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

#include "OutputStatistics.h"

#include "AbstractOutput.h"

#include <QDebug>

OutputStatistics::OutputStatistics(AbstractOutput* output)
    : QObject(output), _output(output), _distribution(LogNormal)
{
    connect(_output, SIGNAL(cleared()),
            this, SLOT(clear()));
}

void OutputStatistics::calculate()
{
    if (!hasEnoughData())
        return;

    // Resize everything to the appropriate size
    _average.clear();
    _stdev.clear();

    for (int i = 0; i < _output->ref().size(); ++i) {
        int count = 0;
        double sum = 0;
        double sqrSum = 0;

        // Add up all of the known data
        for (int s = 0; s < _output->siteCount(); ++s) {
            for (int m = 0; m < _output->motionCount(); ++m) {
                if (_output->seriesEnabled(s, m)
                    && i < _output->data(s, m).size()) {
                    double dataPt = _output->data(s, m).at(i);

                    if (_distribution == LogNormal)
                        dataPt = log(dataPt);

                    sum += dataPt;
                    sqrSum += dataPt * dataPt;
                    ++count;
                }
            }
        }

        if (count) {
            _average << (sum / count);

            // Compute the standard deviation. The absolute value is used to deal
            // with rounding errors. At times the difference with go negative when
            // all of the values are the same number.
            _stdev << ((count > 2) ? sqrt(abs(sqrSum - (sum * sum) / count) / (count-1)) : 0);
        } else {
            // No more data stop
            break;
        }
    }

    if (_distribution == LogNormal) {
        // Convert the average in log space into normal space
        for (int i = 0; i < _average.size(); ++i)
            _average[i] = exp(_average.at(i));
    }

    // Compute the quantiles    
    _plusStd.resize(_average.size());
    _minusStd.resize(_average.size());

    switch (_distribution) {
    case Normal:
        for (int i = 0; i < _average.size(); ++i) {
            _plusStd[i] = _average.at(i) + _stdev.at(i);
            _minusStd[i] = _average.at(i) - _stdev.at(i);
        }
        break;
    case LogNormal:
        for (int i = 0; i < _average.size(); ++i) {
            _plusStd[i] = _average.at(i) * exp(_stdev.at(i));
            _minusStd[i] = _average.at(i) * exp(-_stdev.at(i));
        }
    }
}

void OutputStatistics::plot(QwtPlot* const qwtPlot) const
{
    if (!hasEnoughData() || _average.isEmpty())
        return;

    QwtPlotCurve* curve = plotCurve(qwtPlot, _average, Qt::SolidLine);
    curve->setTitle(averageLabel());
    curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
    curve->setLegendIconSize(QSize(32, 8));

    // Check if there is enough data for a standard deviaiton to be computed
    if ((_output->siteCount() * _output->motionCount()) > 2) {
        curve = plotCurve(qwtPlot, _plusStd, Qt::DashLine);
        curve->setTitle(averageLabel() + "+/-" + stdevLabel());
        curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
        curve->setLegendIconSize(QSize(32, 8));

        curve = plotCurve(qwtPlot, _minusStd, Qt::DashLine);
        curve->setItemAttribute(QwtPlotItem::Legend, false);
    }
}

auto OutputStatistics::hasEnoughData() const -> bool
{
    return (_output->motionCount() * _output->siteCount()) > 1;
}

auto OutputStatistics::distribution() const -> OutputStatistics::Distribution
{
    return _distribution;
}

void OutputStatistics::setDistribution(Distribution distribution)
{
    if (_distribution != distribution) {
        _distribution = distribution;

        emit distributionChanged(_distribution);
        emit wasModified();
    }
}

void OutputStatistics::setDistribution(int distribution)
{
    setDistribution((Distribution)distribution);
}

auto OutputStatistics::averageLabel() const -> QString
{
    switch (_distribution) {
    case Normal:
        return tr("Mean");
    case LogNormal:
        return tr("Median");
    }

    return "";
}

auto OutputStatistics::stdevLabel() const -> QString
{
    switch (_distribution) {
    case Normal:
        return tr("Stdev");
    case LogNormal:
        return tr("Log Stdev");
    }

    return "";
}

auto OutputStatistics::average() const -> const QVector<double>&
{
    return _average;
}

auto OutputStatistics::stdev() const -> const QVector<double>&
{
    return _stdev;
}

void OutputStatistics::clear()
{
    _average.clear();
    _stdev.clear();
    _plusStd.clear();
    _minusStd.clear();
}

auto OutputStatistics::plotCurve(QwtPlot* const plot, const QVector<double> &vec, Qt::PenStyle penStyle) const -> QwtPlotCurve*
{
    // Figure out what x and y are based on curveType
    const QVector<double> & x = _output->curveType() == AbstractOutput::Yfx ? _output->ref() : vec;
    const QVector<double> & y = _output->curveType() == AbstractOutput::Yfx ? vec : _output->ref();

    // Create the curve
    auto *qpc = new QwtPlotCurve;
    _output->setCurveSamples(qpc, x, y);

    qpc->setPen(QPen(QBrush(Qt::blue), 2, penStyle));
    qpc->setZ(AbstractOutput::zOrder() + 1);    
    qpc->setRenderHint(QwtPlotItem::RenderAntialiased);

    qpc->attach(plot);

    return qpc;
}
