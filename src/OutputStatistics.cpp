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

#include "OutputStatistics.h"

#include "AbstractOutput.h"

#include <QDebug>

OutputStatistics::OutputStatistics(AbstractOutput* output)
    : QObject(output), m_output(output), m_distribution(LogNormal)
{
    connect(m_output, SIGNAL(cleared()),
            this, SLOT(clear()));
}

void OutputStatistics::calculate()
{
    if (!hasEnoughData())
        return;

    // Resize everything to the appropriate size
    m_average.clear();
    m_stdev.clear();

    for (int i = 0; i < m_output->ref().size(); ++i) {
        int count = 0;
        double sum = 0;
        double sqrSum = 0;

        // Add up all of the known data
        for (int s = 0; s < m_output->siteCount(); ++s) {
            for (int m = 0; m < m_output->motionCount(); ++m) {
                if (m_output->seriesEnabled(s, m)
                    && i < m_output->data(s, m).size()) {
                    double dataPt = m_output->data(s, m).at(i);

                    if (m_distribution == LogNormal)
                        dataPt = log(dataPt);

                    sum += dataPt;
                    sqrSum += dataPt * dataPt;
                    ++count;
                }
            }
        }

        if (count) {
            m_average << (sum / count);

            // Compute the standard deviation. The absolute value is used to deal
            // with rounding errors. At times the difference with go negative when
            // all of the values are the same number.
            m_stdev << ((count > 2) ? sqrt(fabs(sqrSum - (sum * sum) / count) / (count-1)) : 0);
        } else {
            // No more data stop
            break;
        }
    }

    if (m_distribution == LogNormal) {
        // Convert the average in log space into normal space
        for (int i = 0; i < m_average.size(); ++i)
            m_average[i] = exp(m_average.at(i));
    }

    // Compute the quantiles    
    m_plusStd.resize(m_average.size());
    m_minusStd.resize(m_average.size());

    switch (m_distribution) {
    case Normal:
        for (int i = 0; i < m_average.size(); ++i) {
            m_plusStd[i] = m_average.at(i) + m_stdev.at(i);
            m_minusStd[i] = m_average.at(i) - m_stdev.at(i);
        }
        break;
    case LogNormal:
        for (int i = 0; i < m_average.size(); ++i) {
            m_plusStd[i] = m_average.at(i) * exp(m_stdev.at(i));
            m_minusStd[i] = m_average.at(i) * exp(-m_stdev.at(i));
        }
    }
}

void OutputStatistics::plot(QwtPlot* const qwtPlot) const
{
    if (!hasEnoughData() || m_average.isEmpty())
        return;

    QwtPlotCurve* curve = plotCurve(qwtPlot, m_average, Qt::SolidLine);
    curve->setTitle(averageLabel());

    // Check if there is enough data for a standard deviaiton to be computed
    if ((m_output->siteCount() * m_output->motionCount()) > 2) {
        curve = plotCurve(qwtPlot, m_plusStd, Qt::DashLine);
        curve->setTitle(averageLabel() + "+/-" + stdevLabel());

        curve = plotCurve(qwtPlot, m_minusStd, Qt::DashLine);
        curve->setItemAttribute(QwtPlotItem::Legend, false);
    }
}

bool OutputStatistics::hasEnoughData() const
{
    return (m_output->motionCount() * m_output->siteCount()) > 1;
}

OutputStatistics::Distribution OutputStatistics::distribution() const
{
    return m_distribution;
}

void OutputStatistics::setDistribution(Distribution distribution)
{
    if (m_distribution != distribution) {
        m_distribution = distribution;

        emit distributionChanged(m_distribution);
        emit wasModified();
    }
}

void OutputStatistics::setDistribution(int distribution)
{
    setDistribution((Distribution)distribution);
}

QString OutputStatistics::averageLabel() const
{
    switch (m_distribution) {
    case Normal:
        return tr("Mean");
    case LogNormal:
        return tr("Median");
    }

    return "";
}

QString OutputStatistics::stdevLabel() const
{
    switch (m_distribution) {
    case Normal:
        return tr("Stdev");
    case LogNormal:
        return tr("Log Stdev");
    }

    return "";
}

const QVector<double>& OutputStatistics::average() const
{
    return m_average;
}

const QVector<double>& OutputStatistics::stdev() const
{
    return m_stdev;
}

void OutputStatistics::clear()
{
    m_average.clear();
    m_stdev.clear();
    m_plusStd.clear();
    m_minusStd.clear();
}

QwtPlotCurve* OutputStatistics::plotCurve(QwtPlot* const plot, const QVector<double> &vec, Qt::PenStyle penStyle) const
{
    // Figure out what x and y are based on curveType
    const double* x = m_output->curveType() == AbstractOutput::Yfx ?
                      m_output->ref().data() : vec.data();

    const double* y = m_output->curveType() == AbstractOutput::Yfx ?
                      vec.data() : m_output->ref().data();

    // Create the curve
    QwtPlotCurve *qpc = new QwtPlotCurve;

    qpc->setSamples(x + m_output->offset(), y + m_output->offset(),
                 vec.size() - m_output->offset());
    qpc->setPen(QPen(QBrush(Qt::blue), 2, penStyle));
    qpc->setZ(AbstractOutput::zOrder() + 1);    
    qpc->setRenderHint(QwtPlotItem::RenderAntialiased);

    qpc->attach(plot);

    return qpc;
}
