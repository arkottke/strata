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

#include "CrustalModel.h"

#include "Serialize.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

CrustalModel::CrustalModel(QObject *parent) :
        MyAbstractTableModel(parent)
{

}

int CrustalModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return _thickness.size();
}

int CrustalModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QVariant CrustalModel::headerData( int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation)
    {
    case Qt::Horizontal:
        switch (section)
        {
        case ThicknessColumn:
            return tr("Thickness (km)");
        case VelocityColumn:
            return tr("Vs (km/sec)");
        case DensityColumn:
            return QString(tr("Density (gm/cm%1)")).arg(QChar(0x00B3));
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}

QVariant CrustalModel::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex()) {
        return QVariant();
    }

    if ( role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column())
        {
        case ThicknessColumn:
            return QString::number(_thickness.at(index.row()));
        case VelocityColumn:
            return QString::number(_velocity.at(index.row()));
        case DensityColumn:
            return QString::number(_density.at(index.row()));
        }
    }

    return QVariant();
}

bool CrustalModel::setData( const QModelIndex & index, const QVariant & value, int role)
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole) {
        return false;
    }

    bool b;
    const double d = value.toDouble(&b);

    if (b) {
        switch (index.column()) {
        case ThicknessColumn:
            _thickness[index.row()] = d;
            break;
        case VelocityColumn:
            _velocity[index.row()] = d;
            break;
        case DensityColumn:
            _density[index.row()] = d;
            break;
        }
        dataChanged(index, index);
        emit wasModified();
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags CrustalModel::flags( const QModelIndex & index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool CrustalModel::insertRows(int row, int count, const QModelIndex &parent)
{
    emit beginInsertRows( parent, row, row+count-1 );

    _thickness.insert(row, count, 0);
    _velocity.insert(row, count, 0);
    _density.insert(row, count, 0);

    emit wasModified();
    emit endInsertRows();

    return true;
}

bool CrustalModel::removeRows(int row, int count, const QModelIndex &parent)
{
    emit beginRemoveRows(parent, row, row+count-1 );

    _thickness.remove(row, count);
    _velocity.remove(row, count);
    _density.remove(row, count);

    emit wasModified();
    emit endRemoveRows();

    return true;
}


// Compute the average value of a property over a given depth range.
double CrustalModel::averageValue(const QVector<double> & thickness,
                                  const QVector<double> & property, const double maxDepth)
{
    // Depth to the base of the current layer
    double depth = 0;
    double sum = 0;

    for (int i = 0; i < thickness.size(); ++i) {
        depth += thickness.at(i);

        // Partial layer
        if (maxDepth < depth) {
            sum += (thickness.at(i) - (depth - maxDepth)) * property.at(i);
            break;
        }
        // Final infinite layer
        if (i == thickness.size()-1) {
            sum += (maxDepth - depth) * property.last();
            break;
        }

        sum += thickness.at(i) * property.at(i);
    }
    return sum / maxDepth;
}

QVector<double> CrustalModel::calculate(const QVector<double> &freq) const
{
    QVector<double> amp(freq.size());
    // Slowness (inverse of the crustal velocity
    QVector<double> slowness(_velocity.size());

    for (int i = 0; i < slowness.size(); ++i) {
        slowness[i] = 1./_velocity.at(i);
    }

    // average slowness over a depth range (1/velocity)
    QVector<double> avgSlow(freq.size(), slowness.first());
    // Frequency dependent depth
    QVector<double> depth_f(freq.size(), 0.);

    for (int i = 0; i < freq.size(); ++i) {
        double error = 0;
        int count = 0;

        do {
            ++count;
            depth_f[i] = 1. / (4 * freq.at(i) * avgSlow.at(i));
            const double oldValue = avgSlow.at(i);
            avgSlow[i] = averageValue(_thickness, slowness, depth_f.at(i));
            error = fabs((oldValue - avgSlow.at(i)) / avgSlow.at(i));
        } while (error > 0.005 && count < 10);
    }

    for (int i = 0; i < freq.size(); ++i) {
        // Average density for the depth range
        const double avgDensity = averageValue(_thickness, _density, depth_f.at(i));
        amp[i] = sqrt((_velocity.at(i) * _density.at(i)) / (avgDensity / avgSlow.at(i)));
    }

    return amp;
}

void CrustalModel::fromJson(const QJsonObject &json)
{
    beginResetModel();
    Serialize::toDoubleVector(json["thickness"], _thickness);
    Serialize::toDoubleVector(json["velocity"], _velocity);
    Serialize::toDoubleVector(json["density"], _density);
    endResetModel();
}

QJsonObject CrustalModel::toJson() const
{
    QJsonObject json;

    json["thickness"] = Serialize::toJsonArray(_thickness);
    json["velocity"] = Serialize::toJsonArray(_velocity);
    json["density"] = Serialize::toJsonArray(_density);

    return json;
}

QDataStream& operator<< (QDataStream & out, const CrustalModel* cm)
{
    out << (quint8)1;

    out << cm->_thickness << cm->_velocity << cm->_density;

    return out;
}

QDataStream& operator>> (QDataStream & in, CrustalModel* cm)
{
    quint8 ver;
    in >> ver;

    cm->beginResetModel();

    in >> cm->_thickness >> cm->_velocity >> cm->_density;

    cm->endResetModel();

    return in;
}
