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

#include "PathDurationModel.h"

#include "Serialize.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

PathDurationModel::PathDurationModel(QObject *parent) :
        MyAbstractTableModel(parent)
{

}

int PathDurationModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return _distance.size();
}

int PathDurationModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant PathDurationModel::headerData( int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation)
    {
    case Qt::Horizontal:
        switch (section)
        {
        case DistanceColumn:
            return tr("Distance (km)");
        case RateColumn:
            return tr("Dur. Incr. (sec)");
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}

QVariant PathDurationModel::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex()) {
        return QVariant();
    }

    if ( role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column())
        {
        case DistanceColumn:
            return QString::number(_distance.at(index.row()));
        case RateColumn:
            return QString::number(_rate.at(index.row()));
        }
    }

    return QVariant();
}

bool PathDurationModel::setData( const QModelIndex & index, const QVariant & value, int role)
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole) {
        return false;
    }

    bool b;
    const double d = value.toDouble(&b);

    if (b) {
        switch (index.column()) {
        case DistanceColumn:
            _distance[index.row()] = d;
            break;
        case RateColumn:
            _rate[index.row()] = d;
            break;
        }
        dataChanged(index, index);
        emit wasModified();
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags PathDurationModel::flags( const QModelIndex & index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool PathDurationModel::insertRows(int row, int count, const QModelIndex &parent)
{
    emit beginInsertRows( parent, row, row+count-1 );

    _distance.insert(row, count, 0);
    _rate.insert(row, count, 0);

    emit wasModified();
    emit endInsertRows();

    return true;
}

bool PathDurationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    emit beginRemoveRows(parent, row, row+count-1 );

    _distance.remove(row, count);
    _rate.remove(row, count);

    emit wasModified();
    emit endRemoveRows();

    return true;
}

void PathDurationModel::setRegion(AbstractRvtMotion::Region region) {
    switch (region) {
        case AbstractRvtMotion::WUS:
            _distance = {0.,};
            _rate = {0.05};
            break;
        case AbstractRvtMotion::CEUS:
            _distance = {0., 10., 70., 130.};
            _rate = {0., 0.16, -0.03, 0.04};
            break;
        default:
            break;
    }
    emit wasModified();
}

double PathDurationModel::duration(double distance) const {
    double dur = 0;
    double incrDist;
    int i = 0; 
    do {
        if (i < (_distance.size() - 1)) {
            incrDist = std::min(distance, _distance.at(i + 1));
        } else {
            incrDist = distance;
        }
        incrDist -= _distance.at(i);
        dur += incrDist * _rate.at(i);
    } while (i < _distance.size() && _distance.at(i) < distance);

    return dur;
}

void PathDurationModel::fromJson(const QJsonObject &json)
{
    beginResetModel();
    Serialize::toDoubleVector(json["distance"], _distance);
    Serialize::toDoubleVector(json["rate"], _rate);
    endResetModel();
}

QJsonObject PathDurationModel::toJson() const
{
    QJsonObject json;

    json["distance"] = Serialize::toJsonArray(_distance);
    json["rate"] = Serialize::toJsonArray(_rate);

    return json;
}

QDataStream& operator<< (QDataStream & out, const PathDurationModel* pdm)
{
    out << (quint8)1;

    out << pdm->_distance << pdm->_rate;

    return out;
}

QDataStream& operator>> (QDataStream & in, PathDurationModel* pdm)
{
    quint8 ver;
    in >> ver;

    pdm->beginResetModel();

    in >> pdm->_distance >> pdm->_rate;

    pdm->endResetModel();

    return in;
}
