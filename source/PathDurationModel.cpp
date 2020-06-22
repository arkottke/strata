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
    MyAbstractTableModel(parent), _source(Default)
{
}

void PathDurationModel::setRegion(AbstractRvtMotion::Region region)
{
    if (_source != Default)
        return;

    beginResetModel();
    // WUS and CEUS amplification from Campbell (2003)
    if (region == AbstractRvtMotion::WUS) {
        _distance = {0};
        _rate = {0.05};
    } else if (region == AbstractRvtMotion::CEUS) {
        _distance = {0., 10., 70., 130.};
        _rate = {0., 0.16, -0.03, 0.04};
    }
    endResetModel();
    emit wasModified();
}

void PathDurationModel::setRegion(int region)
{
    setRegion(static_cast<AbstractRvtMotion::Region>(region));
}

auto PathDurationModel::sourceList() -> QStringList {
    return {tr("Default"), tr("Specified")};
}

void PathDurationModel::setSource(Source source) {
    if (_source != source) {
        _source = source;
        emit sourceChanged(_source);
    }
}

void PathDurationModel::setSource(int source) {
    setSource(static_cast<Source>(source));
}

auto PathDurationModel::source() const -> PathDurationModel::Source {
    return _source;
}

auto PathDurationModel::rowCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);

    return _distance.size();
}

auto PathDurationModel::columnCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);

    return 2;
}

auto PathDurationModel::headerData( int section, Qt::Orientation orientation, int role) const -> QVariant
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

auto PathDurationModel::data(const QModelIndex & index, int role) const -> QVariant
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

auto PathDurationModel::setData( const QModelIndex & index, const QVariant & value, int role) -> bool
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

auto PathDurationModel::flags( const QModelIndex & index) const -> Qt::ItemFlags
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

auto PathDurationModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    emit beginInsertRows( parent, row, row+count-1 );

    _distance.insert(row, count, 0);
    _rate.insert(row, count, 0);

    emit wasModified();
    emit endInsertRows();

    return true;
}

auto PathDurationModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    emit beginRemoveRows(parent, row, row+count-1 );

    _distance.remove(row, count);
    _rate.remove(row, count);

    emit wasModified();
    emit endRemoveRows();

    return true;
}

auto PathDurationModel::duration(double distance) const -> double {
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
        i += 1;
    } while (i < _distance.size() && _distance.at(i) < distance);

    return dur;
}

void PathDurationModel::fromJson(const QJsonObject &json)
{
    beginResetModel();
    setSource(json["source"].toInt(0));
    Serialize::toDoubleVector(json["distance"], _distance);
    Serialize::toDoubleVector(json["rate"], _rate);
    endResetModel();
}

auto PathDurationModel::toJson() const -> QJsonObject
{
    QJsonObject json;
    
    json["source"] = static_cast<int>(_source);
    json["distance"] = Serialize::toJsonArray(_distance);
    json["rate"] = Serialize::toJsonArray(_rate);

    return json;
}

auto operator<< (QDataStream & out, const PathDurationModel* pdm) -> QDataStream&
{
    out << static_cast<quint8>(2);
    out << (int)pdm->_source << pdm->_distance << pdm->_rate;

    return out;
}

auto operator>> (QDataStream & in, PathDurationModel* pdm) -> QDataStream&
{
    quint8 ver;
    in >> ver;

    pdm->beginResetModel();

    if (ver > 1){
        int source;
        in >> source;
        pdm->setSource(source);
    }

    in >> pdm->_distance >> pdm->_rate;

    pdm->endResetModel();

    return in;
}
