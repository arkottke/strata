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

#include "AbstractOutput.h"

#include "OutputCatalog.h"
#include "AbstractOutputInterpolater.h"
#include "OutputStatistics.h"

#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QJsonArray>
#include <QPen>

#include <qwt_text.h>
#include <qwt_scale_engine.h>

AbstractOutput::AbstractOutput(OutputCatalog *catalog)
    : QAbstractTableModel(catalog),
      _catalog(catalog),
      _statistics(nullptr),
      _interp(nullptr),
      _offset_top(0),
      _offset_bot(0)
{
    _exportEnabled = false;
    _motionIndex = 0;
    _maxSize = 0;
}

auto AbstractOutput::rowCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent);

    int count;
    if (_data.size() == 0)
    {
        // Empty data
        count = 0;
    }
    else
    {
        if (needsTime())
        {
            count = ref(_motionIndex).size();
        }
        else
        {
            count = _maxSize;
        }
    }
    return count;
}

auto AbstractOutput::columnCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent);

    int count = needsTime() ? (1 + siteCount()) : (1 + siteCount() * motionCount());

    if (_statistics && _statistics->hasEnoughData())
        count += 2;

    return count;
}

auto AbstractOutput::data(const QModelIndex &index, int role) const -> QVariant
{
    if (index.parent() != QModelIndex())
        return QVariant();

    int site, motion;
    columnToSiteMotion(index.column(), &site, &motion);

    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return ref(motion).at(index.row());
        }
        else if (_statistics && _statistics->hasEnoughData() && index.column() == columnCount() - 2)
        {
            return index.row() < _statistics->average().size() ? QVariant(_statistics->average().at(index.row())) : QVariant();
        }
        else if (_statistics && _statistics->hasEnoughData() && index.column() == columnCount() - 1)
        {
            return index.row() < _statistics->stdev().size() ? QVariant(_statistics->stdev().at(index.row())) : QVariant();
        }
        else
        {
            return index.row() < _data.at(site).at(motion).size() ? QVariant(_data.at(site).at(motion).at(index.row())) : QVariant("NaN");
        }
    }

    return QVariant();
}

auto AbstractOutput::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation)
    {
    case Qt::Horizontal:
        if (section == 0)
        {
            return (curveType() == Yfx) ? xLabel() : yLabel();
        }
        else if (_statistics && _statistics->hasEnoughData() && section == columnCount() - 2)
        {
            return _statistics->averageLabel();
        }
        else if (_statistics && _statistics->hasEnoughData() && section == columnCount() - 1)
        {
            return _statistics->stdevLabel();
        }
        else
        {
            int site, motion;
            columnToSiteMotion(section, &site, &motion);

            if (motionIndependent())
            {
                return QString("S-%1%2")
                    .arg(site + 1) // Start at 1
                    .arg(_catalog->siteEnabled(site) ? "" : " (disabled)");
            }
            else if (siteIndependent())
            {
                return QString("M-%1%2")
                    .arg(_catalog->motionNameAt(motion))
                    .arg(_catalog->motionEnabled(motion) ? "" : " (disabled)");
            }
            else
            {
                return QString("S-%1-M-%2%3")
                    .arg(site + 1) // Start at 1
                    .arg(_catalog->motionNameAt(motion))
                    .arg(_catalog->enabledAt(section - 1) ? "" : " (disabled)");
            }
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}

void AbstractOutput::addData(int motion, AbstractCalculator *const calculator)
{
    QVector<double> ref;
    QVector<double> data;

    extract(calculator, ref, data);

    if (_interp)
        data = _interp->calculate(ref, data, this->ref(motion));

    if (motion == 0)
        _data << QList<QVector<double>>();

    if (!motionIndependent() || motion == 0)
        // Save the data for the first motion or for motion depedent results
        _data.last() << data;

    if (_maxSize < data.size())
        _maxSize = data.size();
}

void AbstractOutput::removeLastSite()
{
    if (_data.size())
        _data.takeLast();
}

void AbstractOutput::finalize()
{
    if (_statistics)
        _statistics->calculate();
}

void AbstractOutput::plot(QwtPlot *const qwtPlot, QList<QwtPlotCurve *> &curves) const
{
    if (!isComplete())
        return;

    curves.clear();
    qwtPlot->detachItems();

    // Set the scale engine of the axis
    qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, xScaleEngine());
    qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, yScaleEngine());

    // Label the axes
    labelAxes(qwtPlot);

    // Create the curves
    int n;
    int offset;
    for (int i = 0; i < siteCount(); ++i)
    {
        for (int j = 0; j < motionCount(); ++j)
        {
            const QVector<double> &x = (curveType() == Yfx) ? ref(j) : _data.at(i).at(j);

            const QVector<double> &y = (curveType() == Yfx) ? _data.at(i).at(j) : ref(j);

            auto *curve = new QwtPlotCurve;
            setCurveSamples(curve, x, y);

            curve->setPen(QPen(Qt::darkGray));
            curve->setZ(zOrder());

            curve->setItemAttribute(QwtPlotItem::Legend, false);

            curve->setRenderHint(QwtPlotItem::RenderAntialiased);

            curve->attach(qwtPlot);

            curves << curve;
        }
    }

    if (_statistics)
        _statistics->plot(qwtPlot);

    qwtPlot->replot();
}

void AbstractOutput::exportData(const QString &path, const QString &separator, const QString &prefix)
{
    const int oldMotionIndex = _motionIndex;

    for (int m = 0; m < motionCount(); ++m)
    {
        _motionIndex = m;
        QFile file(QDir(path).absoluteFilePath(prefix + fileName(m) + ".csv"));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qDebug() << "Error opening:" << file.fileName();
            return;
        }
        QTextStream out(&file);

        // Header data
        out << "# Strata Output: " << name() << Qt::endl;
        out << "# Project:" << _catalog->title() << Qt::endl;

        // Column names
        out << "# ";
        const int colCount = columnCount();
        for (int i = 0; i < colCount; ++i)
            out << headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() << separator;

        out << Qt::endl;

        // Data
        for (int r = 0; r < rowCount(); ++r)
        {
            for (int c = 0; c < colCount; ++c)
            {
                out << data(index(r, c), Qt::DisplayRole).toString();

                if (c < colCount - 1)
                    out << separator;
            }

            out << Qt::endl;
        }

        file.close();

        // Only loop over output that needs time
        if (!needsTime())
            break;
    }

    _motionIndex = oldMotionIndex;
}

void AbstractOutput::intToSiteMotion(int i, int *site, int *motion) const
{
    *site = intToSite(i);
    *motion = intToMotion(i);
}

auto AbstractOutput::intToSite(int i) const -> int
{
    return i / motionCount();
}

auto AbstractOutput::intToMotion(int i) const -> int
{
    return i % motionCount();
}

void AbstractOutput::clear()
{
    emit beginResetModel();
    _data.clear();
    _motionIndex = 0;
    _maxSize = 0;
    emit endResetModel();
}

auto AbstractOutput::seriesEnabled(int site, int motion) -> bool
{
    return _catalog->enabledAt(site, motion);
}

auto AbstractOutput::exportEnabled() const -> bool
{
    return _exportEnabled;
}

void AbstractOutput::setExportEnabled(bool exportEnabled)
{
    if (_exportEnabled != exportEnabled)
    {
        _exportEnabled = exportEnabled;

        emit exportEnabledChanged(_exportEnabled);
        emit wasModified();
    }
}

auto AbstractOutput::motionIndex() const -> int
{
    return _motionIndex;
}

void AbstractOutput::setMotionIndex(int motionIndex)
{
    Q_ASSERT(motionIndex < motionCount());
    beginResetModel();
    _motionIndex = motionIndex;

    // Reset the table model
    endResetModel();
}

auto AbstractOutput::needsDepth() const -> bool
{
    return false;
}

auto AbstractOutput::needsFreq() const -> bool
{
    return false;
}

auto AbstractOutput::needsPeriod() const -> bool
{
    return false;
}

auto AbstractOutput::needsTime() const -> bool
{
    return false;
}

auto AbstractOutput::timeSeriesOnly() const -> bool
{
    return false;
}

void AbstractOutput::labelAxes(QwtPlot *const qwtPlot) const
{
    // Label the axes
    QFont font = QApplication::font();
    qwtPlot->setAxisFont(QwtPlot::xBottom, font);
    qwtPlot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text;
    text.setFont(font);

    text.setText(xLabel());
    qwtPlot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(yLabel());
    qwtPlot->setAxisTitle(QwtPlot::yLeft, text);
}

auto AbstractOutput::curveType() const -> AbstractOutput::CurveType
{
    return Yfx;
}

void AbstractOutput::setCurveSamples(QwtPlotCurve *curve, const QVector<double> &x, const QVector<double> &y) const
{
    int n = std::min(x.size(), y.size());
    n -= (_offset_top + _offset_bot);
    curve->setSamples(x.data() + _offset_top, y.data() + _offset_top, n);
}

auto AbstractOutput::zOrder() -> int
{
    return 20;
}

auto AbstractOutput::isComplete() const -> bool
{
    return (_data.size() == siteCount()) && (_data.at(0).size() == motionCount());
}

auto AbstractOutput::data(int site, int motion) const -> const QVector<double> &
{
    Q_ASSERT(site < _data.size());
    Q_ASSERT(motion < _data.at(site).size());

    return _data.at(site).at(motion);
}

auto AbstractOutput::siteIndependent() const -> bool
{
    return false;
}

auto AbstractOutput::motionIndependent() const -> bool
{
    return false;
}

auto AbstractOutput::siteCount() const -> int
{
    return siteIndependent() ? 1 : _catalog->siteCount();
}

auto AbstractOutput::motionCount() const -> int
{
    return motionIndependent() ? 1 : _catalog->motionCount();
}

void AbstractOutput::columnToSiteMotion(const int column, int *site, int *motion) const
{
    *site = -1;
    *motion = _motionIndex;
    const int end = columnCount() -
                    ((_statistics && _statistics->hasEnoughData()) ? 2 : 0);

    if (0 < column && column < end)
    {
        // Only find values of the site and motion indices for the data columns (column != 0)
        if (needsTime())
        {
            *site = column - 1;
            *motion = _motionIndex;
        }
        else
        {
            intToSiteMotion(column - 1, site, motion);
        }
    }
}

auto AbstractOutput::prefix() const -> const QString
{
    return "";
}

auto AbstractOutput::suffix() const -> const QString
{
    return "";
}

void AbstractOutput::fromJson(const QJsonObject &json)
{
    _exportEnabled = json["exportEnabled"].toBool();

    QJsonArray data = json["data"].toArray();
    for (const QJsonValue &site : data)
    {
        QList<QVector<double>> l;
        for (const QJsonValue &motion : site.toArray())
        {
            QVector<double> v;
            for (const QJsonValue &qjv : motion.toArray())
            {
                v << qjv.toDouble();
            }
            l << v;
        }

        if (l.size() > 0)
            _data << l;
    }

    _maxSize = 0;
    for (const QList<QVector<double>> &l : _data)
    {
        for (const QVector<double> &v : l)
        {
            if (_maxSize < v.size())
                _maxSize = v.size();
        }
    }
}

auto AbstractOutput::toJson() const -> QJsonObject
{
    QJsonObject json;
    json["className"] = metaObject()->className();
    json["exportEnabled"] = _exportEnabled;

    QJsonArray data;
    for (const QList<QVector<double>> &l : _data)
    {
        QJsonArray site;
        for (const QVector<double> &v : l)
        {
            QJsonArray motion;
            for (const double &d : v)
            {
                motion << QJsonValue(d);
            }
            // FIXME: Need the QJV?
            site << QJsonValue(motion);
        }
        data << site;
    }

    json["data"] = data;
    return json;
}

auto operator<<(QDataStream &out, const AbstractOutput *ao) -> QDataStream &
{
    out << static_cast<quint8>(1);

    out << ao->_exportEnabled << ao->_data;

    return out;
}

auto operator>>(QDataStream &in, AbstractOutput *ao) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> ao->_exportEnabled >> ao->_data;

    // Find the maximum length of all data vectors
    for (const QList<QVector<double>> &l : ao->_data)
    {
        for (const QVector<double> &v : l)
        {
            if (ao->_maxSize < v.size())
                ao->_maxSize = v.size();
        }
    }

    return in;
}
