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

#include <qwt_text.h>
#include <qwt_scale_engine.h>

AbstractOutput::AbstractOutput(OutputCatalog* catalog)
<<<<<<< HEAD
    : QAbstractTableModel(catalog),
      _catalog(catalog),
      _statistics(nullptr),
      _interp(nullptr),
      _offset(0)
=======
    : QAbstractTableModel(catalog), _catalog(catalog), _statistics(nullptr), _interp(nullptr), _offset(0)
>>>>>>> master
{
    _exportEnabled = false;
    _motionIndex = 0;
    _maxSize = 0;
}

int AbstractOutput::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    int count;
    if (_data.size() == 0) {
        // Empty data
        count = 0;
    } else {
        if (needsTime()) {
            count = ref(_motionIndex).size();
        } else {
            count = _maxSize - _offset;
        }
    }
    return count;
 }

int AbstractOutput::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    int count = needsTime() ?
                (1 + siteCount()) : (1 + siteCount() * motionCount());

    if (_statistics && _statistics->hasEnoughData())
        count += 2;

    return count;
}

QVariant AbstractOutput::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    int site, motion;
    columnToSiteMotion(index.column(), &site, &motion);

    if (role==Qt::DisplayRole) {
        if (index.column() == 0) {
            return ref(motion).at(index.row());
        } else if (_statistics && _statistics->hasEnoughData()
            && index.column() == columnCount() - 2) {
            return index.row() < _statistics->average().size()?
                    QVariant(_statistics->average().at(index.row())) : QVariant();
        } else if (_statistics && _statistics->hasEnoughData()
            && index.column() == columnCount() - 1) {
            return index.row() < _statistics->stdev().size() ?
                    QVariant(_statistics->stdev().at(index.row())) : QVariant();
        } else {
            return index.row() < _data.at(site).at(motion).size() ?
                    QVariant(_data.at(site).at(motion).at(index.row())) : QVariant("NaN");
        }
    }

    return QVariant();
}

QVariant AbstractOutput::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        if (section == 0) {
            return (curveType() == Yfx) ?
                    xLabel() : yLabel();
        } else if (_statistics && _statistics->hasEnoughData()
            && section == columnCount() - 2) {
            return _statistics->averageLabel();
        } else if (_statistics && _statistics->hasEnoughData()
            && section == columnCount() - 1) {
            return _statistics->stdevLabel();
        } else {
            int site, motion;
            columnToSiteMotion(section, &site, &motion);

            if (motionIndependent()) {
                return QString("S-%1%2")
                            .arg(site + 1) // Start at 1
                            .arg(_catalog->siteEnabled(site) ?
                                 "" : " (disabled)");
            } else if (siteIndependent()) {
                return QString("M-%1%2")
                            .arg(_catalog->motionNameAt(motion))
                            .arg(_catalog->motionEnabled(motion) ?
                                 "" : " (disabled)");
            } else {
                return QString("S-%1-M-%2%3")
                        .arg(site + 1) // Start at 1
                        .arg(_catalog->motionNameAt(motion))
                        .arg(_catalog->enabledAt(section - 1) ?
                             "" : " (disabled)");
            }
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

void AbstractOutput::addData(int motion, AbstractCalculator* const calculator)
{
    QVector<double> ref;
    QVector<double> data;

    extract(calculator, ref, data);

    if (_interp)
        data = _interp->calculate(ref, data, this->ref(motion));

    if (motion == 0)
        _data << QList<QVector<double > >();

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

void AbstractOutput::plot(QwtPlot* const qwtPlot, QList<QwtPlotCurve*> & curves) const
{
    curves.clear();
    qwtPlot->detachItems();

    // Set the scale engine of the axis
    qwtPlot->setAxisScaleEngine(QwtPlot::xBottom, xScaleEngine());
    qwtPlot->setAxisScaleEngine(QwtPlot::yLeft, yScaleEngine());

    // Label the axes
    labelAxes(qwtPlot);

    // Create the curves
    for (int i = 0; i < siteCount(); ++i) {
        for (int j = 0; j < motionCount(); ++j) {

            const QVector<double> & x = (curveType() == Yfx) ?
                                        ref(j) : _data.at(i).at(j);

            const QVector<double> & y = (curveType() == Yfx) ?
                                        _data.at(i).at(j) : ref(j);

            const int n = _data.at(i).at(j).size();

            QwtPlotCurve* curve = new QwtPlotCurve;

            curve->setSamples(
                    x.data() + _offset,
                    y.data() + _offset,
                    n - _offset);
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

    for (int m = 0; m < motionCount(); ++m) { 
        _motionIndex = m;
        QFile file(QDir(path).absoluteFilePath(prefix + fileName(m) + ".csv"));

        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qDebug() << "Error opening:" << file.fileName();
            return;
        }
        QTextStream out(&file);

        // Header data
        out << "# Strata Output: " << name() << endl;
        out << "# Project:" << _catalog->title() << endl;

        // Column names
        out << "# ";        
        const int colCount = columnCount();
        for (int i = 0; i < colCount; ++i)
            out << headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() << separator;

        out << endl;

        // Data
        for (int r = 0; r < rowCount(); ++r) {
            for (int c = 0; c < colCount; ++c) {
                out << data(index(r, c), Qt::DisplayRole).toString();

                if (c < colCount - 1)
                    out << separator;
            }

            out << endl;
        }

        file.close();

        // Only loop over output that needs time
        if (!needsTime())
            break;
    }

    _motionIndex = oldMotionIndex;
}

void AbstractOutput::intToSiteMotion(int i, int* site, int* motion) const
{
    *site = intToSite(i);
    *motion = intToMotion(i);
}

int AbstractOutput::intToSite(int i) const
{
    return i / motionCount();
}

int AbstractOutput::intToMotion(int i) const
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

bool AbstractOutput::seriesEnabled(int site, int motion)
{
    return _catalog->enabledAt(site, motion);
}

bool AbstractOutput::exportEnabled() const
{
    return _exportEnabled;
}

void AbstractOutput::setExportEnabled(bool exportEnabled)
{
    if ( _exportEnabled != exportEnabled ) {
        _exportEnabled = exportEnabled;

        emit exportEnabledChanged(_exportEnabled);
        emit wasModified();
    }
}

int AbstractOutput::motionIndex() const
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

bool AbstractOutput::needsDepth() const
{
    return false;
}

bool AbstractOutput::needsFreq() const
{
    return false;
}

bool AbstractOutput::needsPeriod() const
{
    return false;
}

bool AbstractOutput::needsTime() const
{
    return false;
}

bool AbstractOutput::timeSeriesOnly() const
{
    return false;
}

void AbstractOutput::labelAxes(QwtPlot* const qwtPlot) const
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

AbstractOutput::CurveType AbstractOutput::curveType() const
{
    return Yfx;
}

int AbstractOutput::zOrder()
{
    return 20;
}

int AbstractOutput::offset() const
{
    return _offset;
}

const QVector<double>& AbstractOutput::data(int site, int motion) const
{
    Q_ASSERT(site < _data.size());
    Q_ASSERT(motion < _data.at(site).size());

    return _data.at(site).at(motion);
}

bool AbstractOutput::siteIndependent() const
{
    return false;
}

bool AbstractOutput::motionIndependent() const
{
    return false;
}

int AbstractOutput::siteCount() const
{
    return siteIndependent() ? 1 : _catalog->siteCount();
}

int AbstractOutput::motionCount() const
{
    return motionIndependent() ? 1 : _catalog->motionCount();
}

void AbstractOutput::columnToSiteMotion(const int column, int *site, int *motion) const
{
    *site = -1;
    *motion = _motionIndex;
    const int end = columnCount() -
                    ((_statistics && _statistics->hasEnoughData()) ? 2 : 0);

    if (0 < column && column < end) {
        // Only find values of the site and motion indices for the data columns (column != 0)
        if (needsTime()) {
            *site = column - 1;
            *motion = _motionIndex;
        } else {
            intToSiteMotion(column - 1, site, motion);
        }
    }
}

const QString AbstractOutput::prefix() const
{
    return "";
}

const QString AbstractOutput::suffix() const
{
    return "";
}

void AbstractOutput::fromJson(const QJsonObject &json)
{
    _exportEnabled = json["exportEnabled"].toBool();

    QJsonArray data = json["data"].toArray();
    for (const QJsonValue &site : data) {
        QList<QVector<double> > l;
        for (const QJsonValue &motion : site.toArray()) {
            QVector<double> v;
            for (const QJsonValue &qjv : motion.toArray()) {
                v << qjv.toDouble();
            }
            l << v;
        }

        if (l.size() > 0)
            _data << l;
    }

    _maxSize = 0;
    for (const QList<QVector<double> > &l : _data) {
        for (const QVector<double> &v : l) {
            if (_maxSize < v.size())
                _maxSize = v.size();
        }
    }
}

QJsonObject AbstractOutput::toJson() const
{
    QJsonObject json;
    json["className"] = metaObject()->className();
    json["exportEnabled"] = _exportEnabled;

    QJsonArray data;
    for (const QList<QVector<double> > &l : _data) {
        QJsonArray site;
        for (const QVector<double> &v : l) {
            QJsonArray motion;
            for (const double &d : v) {
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


QDataStream & operator<< (QDataStream & out, const AbstractOutput* ao)
{
    out << static_cast<quint8>(1);

    out << ao->_exportEnabled << ao->_data;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractOutput* ao)
{
    quint8 ver;
    in >> ver;

    in >> ao->_exportEnabled >> ao->_data;

    // Find the maximum length of all data vectors
    for (const QList<QVector<double> > &l : ao->_data) {
        for (const QVector<double> &v : l) {
            if (ao->_maxSize < v.size())
                ao->_maxSize = v.size();
        }
    }

    return in;
}
