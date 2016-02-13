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

#include "SoilTypesOutputCatalog.h"

#include "AbstractOutput.h"
#include "NonlinearPropertyOutput.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "SoilTypeOutput.h"

#include <QJsonObject>
#include <QStringList>

SoilTypesOutputCatalog::SoilTypesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractOutputCatalog(outputCatalog), m_soilTypeCatalog(0)
{
}

int SoilTypesOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();
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
        return m_outputs.at(index.row())->name();
    } else if (role == Qt::CheckStateRole) {
        return m_outputs.at(index.row())->enabled() ?
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
    if (index.parent() != QModelIndex() || m_readOnly)
        return false;

    if (role == Qt::CheckStateRole) {
        m_outputs[index.row()]->setEnabled(value.toBool());
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
    m_soilTypeCatalog = soilTypeCatalog;
    connect(m_soilTypeCatalog, SIGNAL(soilTypeAdded(SoilType*)),
            this, SLOT(addOutput(SoilType*)));
    connect(m_soilTypeCatalog, SIGNAL(soilTypeRemoved(SoilType*)),
            this, SLOT(removeOutput(SoilType*)));
}

void SoilTypesOutputCatalog::addOutput(SoilType* soilType)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_outputs << new SoilTypeOutput(soilType, m_outputCatalog);
    connect(m_outputs.last(), SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    endInsertRows();
}

void SoilTypesOutputCatalog::removeOutput(SoilType *soilType)
{
    // Locate the row
    int row = -1;
    for (int i = 0; i < m_outputs.size(); ++i) {
        if (m_outputs.at(i)->soilType() == soilType) {
            row = i;
            break;
        }
    }

    // If the soilType is found, remove it.
    if (row >= 0) {
        beginRemoveRows(QModelIndex(), row, row);
        delete m_outputs.takeAt(row);
        endRemoveRows();
    }
}

QList<AbstractOutput*> SoilTypesOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach(SoilTypeOutput* sto, m_outputs ) {
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

    while (m_outputs.size())
        m_outputs.takeLast()->deleteLater();

    foreach(const QJsonValue &v, array) {
        const QJsonObject &json = v.toObject();
        int row = json["row"].toInt();
        SoilTypeOutput * sto = new SoilTypeOutput(m_soilTypeCatalog->soilType(row), m_outputCatalog);
        sto->fromJson(json);
        m_outputs << sto;
    }

    endResetModel();
}

QJsonArray SoilTypesOutputCatalog::toJson() const
{
    QJsonArray array;
    foreach (const SoilTypeOutput *sto, m_outputs) {
        QJsonObject json = sto->toJson();
        json["row"] = m_soilTypeCatalog->rowOf(sto->soilType());
        array << QJsonValue(json);
    }
    return array;
}

QDataStream & operator<< (QDataStream & out, const SoilTypesOutputCatalog* stoc)
{
    out << (quint8)1;

    out << stoc->m_outputs.size();

    foreach (SoilTypeOutput* sto, stoc->m_outputs) {
        out << stoc->m_soilTypeCatalog->rowOf(sto->soilType())
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
    while (stoc->m_outputs.size() < size) {
        in >> row;
        stoc->m_outputs << new SoilTypeOutput(
                stoc->m_soilTypeCatalog->soilType(row), stoc->m_outputCatalog);
        in >> stoc->m_outputs.last();
    }
    stoc->endResetModel();

    return in;
}
