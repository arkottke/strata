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

#include "ColumnTableModel.h"

#include <QBrush>
#include <QColor>

#include <QDebug>

ColumnTableModel::ColumnTableModel(QObject *parent)
	: QAbstractTableModel(parent)
{
    m_readOnly = true;
}

void ColumnTableModel::addColumn( const QString & title, QVector<double> * data )
{
    m_columnTitles << title;
    m_columnData << data;

    reset();
}

void ColumnTableModel::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;

    reset();
}

int ColumnTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_columnData.first()->size();
}

int ColumnTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return m_columnData.size();
}

QVariant ColumnTableModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();
    
    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
            return QVariant(QBrush(QColor(200,200,200)));

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
        return QString::number(m_columnData.at(index.column())->at(index.row()));
    }
	
	return QVariant();
}

bool ColumnTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
        (*(m_columnData.at(index.column())))[index.row()] = value.toDouble();
	} else 
		return false;
    
    dataChanged(index, index);
    return true;
}

QVariant ColumnTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
            return m_columnTitles.at(section);
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags ColumnTableModel::flags ( const QModelIndex &index ) const
{
    if (m_readOnly)
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

bool ColumnTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i) {
        for ( int j = 0; j < m_columnData.size(); ++j )
            m_columnData[j]->insert( row, 0 );
    }

	emit endInsertRows();

	return true;
}

bool ColumnTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) {
        for ( int j = 0; j < m_columnData.size(); ++j )
            m_columnData[j]->remove(row);
    }
	
	emit endRemoveRows();
	return true;
}
        
void ColumnTableModel::resetModel()
{
    reset();
}

