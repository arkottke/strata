//////////////////////////////////////////////////////////////////////////////
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

#include "RvtMotion.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

RvtMotion::RvtMotion(QObject * parent) : AbstractRvtMotion(parent)
{
    _duration = 5.0;
    _name = tr("RVT Motion (M $mag @ $dist km)");
}

auto RvtMotion::flags(const QModelIndex & index) const -> Qt::ItemFlags
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

auto RvtMotion::setData( const QModelIndex & index, const QVariant & value, int role) -> bool
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole) {
        return false;
    }

    bool b;
    const double d = value.toDouble(&b);

    if (b) {
        switch (index.column()) {
        case FrequencyColumn:
            _freq[index.row()] = d;
            break;
        case AmplitudeColumn:
            _fourierAcc[index.row()] = d;
            break;
        }
        dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

auto RvtMotion::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (!count)
        return false;

    emit beginInsertRows( parent, row, row+count-1 );

    _freq.insert(row, count, 0);
    _fourierAcc.insert(row, count, 0);

    emit endInsertRows();

    return true;
}

auto RvtMotion::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (!count)
        return false;

    emit beginRemoveRows( parent, row, row+count-1);

    _freq.remove(row, count);
    _fourierAcc.remove(row, count);

    emit endRemoveRows();
    return true;
}

void RvtMotion::setDuration(double d)
{
    setModified(true);
    _duration = d;
}


//! Basic linear interpolation/extrapolation in log-log space
/*! The function takes the log10 of the values and then uses linear
 * interpolation to determine the values.  If the interpolated x coordinates
 * are beyond the specified x coordinates the then extrapolation is used based
 * on the last two specified points.
 */
auto logLogInterp( const QVector<double> & x, const QVector<double> & y, 
        const QVector<double> & xi, QVector<double> & yi) -> bool
{
    // Check the sizes of xi and yi	
    if ( xi.size() != yi.size() )
        yi.resize(xi.size());

    // Compute the log of the values
    QVector<double> logX(x.size());
    QVector<double> logY(y.size());
    QVector<double> logXi(xi.size());
    QVector<double> logYi(yi.size());

    for (int i = 0; i < x.size(); ++i) {
        logX[i] = log10(x.at(i));
        logY[i] = log10(y.at(i));
    }

    for (int i = 0; i < xi.size(); ++i)
        logXi[i] = log10(xi.at(i));

    // Interpolate -- Extrapolate
    for ( int i = 0; i < logXi.size(); ++i )
    {
        for ( int j = 0; j < logX.size()-1; ++j )
        {
            if ( abs(logXi.at(i)-logX.at(j)) < 0.000001 ) {
                // XI and X are the same value
                logYi[i] = logY.at(j);
                // Exit out of the loop
                break;
            } else if ( logX.at(j) < logXi.at(i) && logXi.at(i) <= logX.at(j+1) ) {
                // X_J < XI <= X_J+1
                // LinearllogY interpolate
                double slope = (logY.at(j+1) - logY.at(j)) / (logX.at(j+1) - logX.at(j));
                logYi[i] = slope * (logXi.at(i) - logX.at(j)) + logY.at(j);

                // Exit out of the loop
                break;
            } else if ( logXi.at(i) < logX.first() ) {
                // Extrapolate based on the first two values
                // Extrapolate based on the first two values
                double slope = (logY.at(1) - logY.first()) / (logX.at(1) - logX.first());
                logYi[i] = slope * (logXi.at(i) - logX.first()) + logY.first();
                // Exit out of the loop
                break;
            } else if (logX.last() < logXi.at(i)) {
                // Extrapolate based on the last two values
                // The second to last point
                int n = logY.size() - 2;
                // Extrapolate based on the first two values
                double slope = (logY.last() - logY.at(n)) / (logX.last() - logX.at(n));
                logYi[i] = slope * (logXi.at(i) - logX.last()) + logY.last();
                // Exit out of the loop
                break;
            }
        }
    }

    // Compute the power of the yi values
    for (int i = 0; i < yi.size(); ++i)
        yi[i] = pow(10, logYi.at(i));

    return true;
}

//! A moving average over the data series
/*!
 * \param data data series to smooth
 * \param window the number of ponits on either side of a given point to average against
 */
void smooth(QVector<double> & data, int window)
{
    QVector<double> smoothData(data.size());
    // Window size is adjusted at tails of the data
    int adjustedWindow = 0;

    for ( int i = 0; i < data.size(); ++i) {
        // Adjust the window based on the number of points
        // Number of indexes to the left of a point
        int left = i;
        // Number of indexes to the right of a point
        int right = data.size() - 1 - i;

        if ( window < left && window < right )
            // Enough room on either side of the given point
            adjustedWindow = window;
        else if ( window >= left && window < right )
            // Not enough room on the left side
            adjustedWindow = left;
        else if ( window < left && window >= right ) {
            // Not enough room on the right side
            adjustedWindow = right;
        } else {
            // Not enough room on either side -- use the short side
            if ( left < right )
                adjustedWindow = left;
            else
                adjustedWindow = right;
        }

        // Average the points
        double sum = 0;

        for (int j = -adjustedWindow; j <= adjustedWindow; ++j)
            sum += data.at(i+j);

        smoothData[i] = sum / ( 1 + 2 * adjustedWindow);
    }
    // Replace the original data with the smoothed data
    data = smoothData;
}

auto RvtMotion::freq() const -> const QVector<double> &
{
    return _freq;
}

auto RvtMotion::toHtml() const -> QString
{
    QString html;
// FIXME
//    html += QString(tr(
//                "<table border=\"0\">"
//                "<tr><th>Type:</th><td>%1</td></tr>"
//                "<tr><th>Duration:</th><td>%2</td></tr>"
//                "<tr><th>Source:</th><td>%3</td></tr>"
//
//                ))
//        .arg(typeList().at(_type))
//        .arg(_duration)
//        .arg(sourceList().at(_source));
//
//    switch (_source)
//    {
//        case DefinedFourierSpectrum:
//            html += tr("</table><table><tr><th>Frequency (Hz)</th><th>FAS (g-s)</th></tr>");
//
//            for ( int i = 0; i < _freq.size(); ++i )
//                html += QString("<tr><td>%1</td><td>%2</td></tr>")
//                    .arg( _freq.at(i) )
//                    .arg( _fourierAcc.at(i) );
//
//            html += "</table>";
//            break;
//        case DefinedResponseSpectrum:
//            html += QString(tr("<tr><th>Damping:</th><td>%1</td></tr>"))
//                .arg(_targetRespSpec->damping());
//
//            html += tr("</table><table><tr><th>Period (s)</th><th>Spectral Accel. (g)</th></tr>");
//
//            for ( int i = 0; i < _targetRespSpec->period().size(); ++i )
//                html += QString("<tr><td>%1</td><td>%2</td></tr>")
//                    .arg( _targetRespSpec->period().at(i) )
//                    .arg( _targetRespSpec->sa().at(i) );
//
//            html += "</table>";
//            break;
//        case CalculatedFourierSpectrum:
//            html += _pointSourceModel->toHtml();
//            break;
//    }
    return html;
}

auto RvtMotion::loadFromTextStream(QTextStream &stream, double scale) -> bool
{
    if (!AbstractRvtMotion::loadFromTextStream(stream, scale))
        return false;

    // Skip the column header line
    stream.readLine();

    _freq.clear();
    _fourierAcc.clear();

    double d;
    bool ok = false;
    QString line = stream.readLine();
    QStringList parts;

    while (!line.isNull()) {
        parts = line.split(',');

        // Break if the line doesn't contain at two columns
        if (parts.size() < 2)
            break;

        d = parts.at(0).toFloat(&ok);
        if (!ok) {
            qWarning() << "Unable to parse frequency in line:" << line;
            return false;
        } else if (d > 0.0001) {
            // FIXME
            // Need to divide by frequency to compute the velocity, so we must
            // limit the lowest frequency. This is terribly sloppy and there
            // should be check in the when data is pasted into the table.
            _freq << d;

            d = parts.at(1).toFloat(&ok);
            if (!ok) {
                qWarning() << "Unable to parse the amplitude in line:" << line;
                return false;
            } else {
                _fourierAcc << (scale * d);
            }

        }
        line = stream.readLine();
    }

    calculate();
    return true;
}

void RvtMotion::fromJson(const QJsonObject &json)
{
    AbstractRvtMotion::fromJson(json);

    _freq.clear();
    foreach (const QJsonValue &v, json["freq"].toArray())
        _freq << v.toDouble();

    calculate();
}

auto RvtMotion::toJson() const -> QJsonObject
{
    QJsonObject json = AbstractRvtMotion::toJson();

    QJsonArray freq;
    foreach (const double &d, _freq)
        freq << QJsonValue(d);
    json["freq"] = freq;

    return json;
}

auto operator<< (QDataStream & out, const RvtMotion* rm) -> QDataStream &
{
    out << (quint8)1;

    out << qobject_cast<const AbstractRvtMotion*>(rm);
    out << rm->_freq;

    return out;
}

auto operator>> (QDataStream & in, RvtMotion* rm) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractRvtMotion*>(rm);
    in >> rm->_freq;

    rm->calculate();

    return in;
}
