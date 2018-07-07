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

#include "SoilTypesOutputCatalog.h"

#include "AbstractOutput.h"
#include "NonlinearPropertyOutput.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "SoilTypeOutput.h"

#include <QJsonObject>
#include <QStringList>

SoilTypesOutputCatalog::SoilTypesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractOutputCatalog(outputCatalog), _soilTypeCatalog(0)
{
}

int SoilTypesOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return _outputs.size();
}

int SoilTypesOutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant SoilTypesOutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent() != QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return _outputs.at(index.row())->name();
    } else if (role == Qt::CheckStateRole) {
        return _outputs.at(index.row())->enabled() ?
                Qt::Checked : Qt::Unchecked;
    }

    return AbstractOutputCatalog::data(index, role);
}

QVariant SoilTypesOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
    case Qt::Horizontal:
        return tr("Name");
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

bool SoilTypesOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent() != QModelIndex() || _readOnly)
        return false;

    if (role == Qt::CheckStateRole) {
        _outputs[index.row()]->setEnabled(value.toBool());
    } else {
        return false;
    }

    emit dataChanged(index, index);
    emit wasModified();
    return false;
}


Qt::ItemFlags SoilTypesOutputCatalog::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);

    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void SoilTypesOutputCatalog::setSoilTypeCatalog(SoilTypeCatalog *soilTypeCatalog)
{
    _soilTypeCatalog = soilTypeCatalog;
    connect(_soilTypeCatalog, SIGNAL(soilTypeAdded(SoilType*)),
            this, SLOT(addOutput(SoilType*)));
    connect(_soilTypeCatalog, SIGNAL(soilTypeRemoved(SoilType*)),
            this, SLOT(removeOutput(SoilType*)));
}

void SoilTypesOutputCatalog::addOutput(SoilType* soilType)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    _outputs << new SoilTypeOutput(soilType, _outputCatalog);
    connect(_outputs.last(), SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    endInsertRows();
}

void SoilTypesOutputCatalog::removeOutput(SoilType *soilType)
{
    // Locate the row
    int row = -1;
    for (int i = 0; i < _outputs.size(); ++i) {
        if (_outputs.at(i)->soilType() == soilType) {
            row = i;
            break;
        }
    }

    // If the soilType is found, remove it.
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        delete _outputs.takeAt(row);
        endRemoveRows();
    }
}

QList<AbstractOutput*> SoilTypesOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach(SoilTypeOutput* sto, _outputs ) {
        if (sto->enabled()) {
            list << static_cast<AbstractOutput*>(sto->modulus())
                    << static_cast<AbstractOutput*>(sto->damping());
        }
    }

    return list;
}

void SoilTypesOutputCatalog::fromJson(const QJsonArray &array)
{
    beginResetModel();

    while (_outputs.size())
        _outputs.takeLast()->deleteLater();

    foreach(const QJsonValue &v, array) {
        const QJsonObject &json = v.toObject();
        int row = json["row"].toInt();
        SoilTypeOutput * sto = new SoilTypeOutput(_soilTypeCatalog->soilType(row), _outputCatalog);
        sto->fromJson(json);
        _outputs << sto;
    }

    endResetModel();
}

QJsonArray SoilTypesOutputCatalog::toJson() const
{
    QJsonArray array;
    foreach (const SoilTypeOutput *sto, _outputs) {
        QJsonObject json = sto->toJson();
        json["row"] = _soilTypeCatalog->rowOf(sto->soilType());
        array << QJsonValue(json);
    }
    return array;
}

QDataStream & operator<< (QDataStream & out, const SoilTypesOutputCatalog* stoc)
{
    out << (quint8)1;

    out << stoc->_outputs.size();

    foreach (SoilTypeOutput* sto, stoc->_outputs) {
        out << stoc->_soilTypeCatalog->rowOf(sto->soilType())
                << sto;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilTypesOutputCatalog* stoc)
{
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    stoc->beginResetModel();
    int row;
    while (stoc->_outputs.size() < size) {
        in >> row;
        stoc->_outputs << new SoilTypeOutput(
                stoc->_soilTypeCatalog->soilType(row), stoc->_outputCatalog);
        in >> stoc->_outputs.last();
    }
    stoc->endResetModel();

    return in;
}
