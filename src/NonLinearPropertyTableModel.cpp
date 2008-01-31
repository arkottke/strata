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

#include "NonLinearPropertyTableModel.h"
#include <QBrush>
#include <QDebug>

NonLinearPropertyTableModel::NonLinearPropertyTableModel( NonLinearProperty * nonLinearProperty, QObject *parent )
	: QAbstractTableModel(parent), m_nonLinearProperty(nonLinearProperty)
{
    m_editable = true;
}

void NonLinearPropertyTableModel::setNonLinearProperty( NonLinearProperty * nonLinearProperty )
{
    m_nonLinearProperty = nonLinearProperty;
    reset();
}

int NonLinearPropertyTableModel::rowCount ( const QModelIndex& /* index */ ) const
{   
    if (m_nonLinearProperty)
        return m_nonLinearProperty->strain().size();
    else
        return 0;
}

int NonLinearPropertyTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 2;
}

QVariant NonLinearPropertyTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if (!m_nonLinearProperty)
        return QVariant();

	if ( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// Strain
					return QVariant("Strain (%)");
				case 1:
					return QVariant(m_nonLinearProperty->typeLabel());
			}
		
		case Qt::Vertical:
			return QVariant(section+1);

		default:
			return QVariant();
	}
}

QVariant NonLinearPropertyTableModel::data ( const QModelIndex &index, int role ) const
{
    if (!m_nonLinearProperty)
        return QVariant();

	if (index.parent()!=QModelIndex())
		return QVariant();
    
    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
            return QVariant(QBrush(QColor(200,200,200)));

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
			case 0:
				// Strain
				return QVariant( QString::number(m_nonLinearProperty->strain().at(index.row())));
			case 1:
				// Normalized Shear Modulus
				return QVariant( QString::number(m_nonLinearProperty->avg().at(index.row())));
			default:
				return QVariant();
		}
	}
	
	return QVariant();
}

bool NonLinearPropertyTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if (!m_nonLinearProperty)
        return false;

	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
			case 0:
				// Strain
				m_nonLinearProperty->strain()[index.row()] = value.toDouble();
				break;
			case 1:
				// Normalized Shear Modulus
				m_nonLinearProperty->avg()[index.row()] = value.toDouble();
				break;
			default:
				return false;
		}
		dataChanged(index, index);
		return true;
	} 
	else 
	{
		return false;
	}
}

Qt::ItemFlags NonLinearPropertyTableModel::flags ( const QModelIndex &index ) const
{
    if (m_editable)
	    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

bool NonLinearPropertyTableModel::insertRows ( int row, int count, const QModelIndex & /*parent*/ )
{
    if ( count < 1 || row < 0 )
        return false;

	emit beginInsertRows( QModelIndex(), row, row + count - 1 );
	
	// Insert rows
	for (int i=0; i < count; ++i) {
		m_nonLinearProperty->strain().insert( row, 0.0 ); 	
		m_nonLinearProperty->avg().insert( row, 0.0 ); 	
	}
	
	emit endInsertRows();
	return true;
}

bool NonLinearPropertyTableModel::removeRows ( int row, int count, const QModelIndex & /*parent*/ )
{
    if ( count < 1 || row < 0 )
        return false;

	emit beginRemoveRows( QModelIndex(), row, row + count - 1);
	
	// Remove rows
	for (int i=0; i < count; ++i) {
		m_nonLinearProperty->strain().removeAt( row ); 	
		m_nonLinearProperty->avg().removeAt( row ); 	
	}
	
	emit endRemoveRows();
	
	return true;
}
        
void NonLinearPropertyTableModel::resetModel()
{
    reset();
}

void NonLinearPropertyTableModel::setEditable(bool editable)
{
    m_editable = editable;
    // Reset the table for coloring 
    reset();
}

