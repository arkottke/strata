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

#include "SoilTypeOutputTableModel.h"
#include <QColor>
#include <QBrush>

SoilTypeOutputTableModel::SoilTypeOutputTableModel( QList<SoilType*> & soilTypes, QObject * parent )
    : QAbstractTableModel(parent), m_soilTypes(soilTypes)
{
}

int SoilTypeOutputTableModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_soilTypes.size();
}

int SoilTypeOutputTableModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant SoilTypeOutputTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case 0:
            // Name Column
            return tr("Name");
        }
    case Qt::Vertical:
        return section+1;
    default:
        return QVariant();
    }
}

QVariant SoilTypeOutputTableModel::data(const QModelIndex &index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();
    
    if (role==Qt::BackgroundRole) {
        // Color the background light gray for cells that are not editable
        if (!( flags(index) & Qt::ItemIsEditable || flags(index) & Qt::ItemIsUserCheckable )) {
            return QVariant(QBrush(QColor(200,200,200)));
        }

        // Color the background to red and green for the check state
        if ( flags(index) & Qt::ItemIsUserCheckable) {
            if ( data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked) 
                return QBrush(QColor(200,200,200));
            else 
                return QBrush(QColor(50,200,50));
        }
    }

    if (role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
        case 0:
            // Name Column
            return QVariant( m_soilTypes.at(index.row())->name() );
        default:
            return QVariant();
        }
    } else if (index.column() == 0 && role == Qt::CheckStateRole) {
        if ( m_soilTypes.at(index.row())->saveData() )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }

    return QVariant();
}

bool SoilTypeOutputTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( index.column() == 0 && role == Qt::CheckStateRole ) {
        m_soilTypes[index.row()]->setSaveData(value.toBool());
        emit dataChanged( index, index);
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags SoilTypeOutputTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
}
