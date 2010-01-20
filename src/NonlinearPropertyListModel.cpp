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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "NonlinearPropertyListModel.h"

NonlinearPropertyListModel::NonlinearPropertyListModel(
        QList<NonlinearProperty*> &nlPropertyList, QObject *parent)
    : QAbstractListModel(parent), m_nlPropertyList(nlPropertyList)
{
}
     
int NonlinearPropertyListModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_nlPropertyList.size();
}

QVariant NonlinearPropertyListModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

QVariant NonlinearPropertyListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_nlPropertyList.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        QString name = m_nlPropertyList.at(index.row())->name();

        if ( m_nlPropertyList.at(index.row())->source() != NonlinearProperty::UserDefined )
            return "* " + name;
        else
            return name;
    } else
        return QVariant();
}

bool NonlinearPropertyListModel::setData(const QModelIndex &index,
        const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_nlPropertyList[index.row()]->setName( value.toString() );
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags NonlinearPropertyListModel::flags ( const QModelIndex & index ) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if ( m_nlPropertyList.at(index.row())->source() == NonlinearProperty::UserDefined )
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else
        return QAbstractItemModel::flags(index);
}

bool NonlinearPropertyListModel::insertRows(int position, int rows, const QModelIndex &/*index*/)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row)
        m_nlPropertyList.insert(position, 
                new NonlinearProperty("Untitled", m_nlPropertyList.first()->type(), NonlinearProperty::UserDefined ));

    endInsertRows();
    return true;
}

bool NonlinearPropertyListModel::removeRows(int position, int rows, const QModelIndex &/*index*/)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row)
        m_nlPropertyList.removeAt(position);

    endRemoveRows();
    return true;
}
     
NonlinearProperty * NonlinearPropertyListModel::nlPropertyAt( const QModelIndex &index )
{
    return m_nlPropertyList[index.row()];
}
