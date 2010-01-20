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

#include "NonlinearPropertyTableModel.h"
#include "SoilType.h"

#include <QBrush>
#include <QDebug>

NonlinearPropertyTableModel::NonlinearPropertyTableModel( NonlinearProperty * nonLinearProperty, QObject *parent )
	: MyAbstractTableModel(true,parent), m_nonLinearProperty(nonLinearProperty)
{
}

void NonlinearPropertyTableModel::setNonlinearProperty( NonlinearProperty * nonLinearProperty )
{
    // Disconnect the model for updates
    if (m_nonLinearProperty && m_nonLinearProperty->parent()) {
        if (m_nonLinearProperty->type() == NonlinearProperty::ModulusReduction ) {
            disconnect( m_nonLinearProperty->parent(), SIGNAL(shearModModelChanged()), 0, 0);
        } else {
            disconnect( m_nonLinearProperty->parent(), SIGNAL(dampingModelChanged()), 0, 0);
        }
    }

    m_nonLinearProperty = nonLinearProperty;

    // Connect the model for updates
    if (m_nonLinearProperty && m_nonLinearProperty->parent()) {
        if (m_nonLinearProperty->type() == NonlinearProperty::ModulusReduction ) {
            connect( m_nonLinearProperty->parent(), SIGNAL(shearModModelChanged()), 
                    SLOT(resetModel()));
        } else {
            connect( m_nonLinearProperty->parent(), SIGNAL(dampingModelChanged()), 
                    SLOT(resetModel()));
        }
    }
    reset();
}

int NonlinearPropertyTableModel::rowCount ( const QModelIndex& /* index */ ) const
{   
    if (m_nonLinearProperty)
        return m_nonLinearProperty->strain().size();
    else
        return 0;
}

int NonlinearPropertyTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 2;
}

QVariant NonlinearPropertyTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
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
					return tr("Strain (%)");
				case 1:
					return m_nonLinearProperty->typeLabel();
			}
		
		case Qt::Vertical:
			return section + 1;

		default:
			return QVariant();
	}
}

QVariant NonlinearPropertyTableModel::data( const QModelIndex &index, int role ) const
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
				return QString::number(m_nonLinearProperty->strain().at(index.row()), 'e', 3);
			case 1:
				// Property
				return QString::number(m_nonLinearProperty->avg().at(index.row()), 'f', 3);
			default:
				return QVariant();
		}
	}
	
	return QVariant();
}

bool NonlinearPropertyTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
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

Qt::ItemFlags NonlinearPropertyTableModel::flags ( const QModelIndex &index ) const
{
    if (m_readOnly)
        return QAbstractTableModel::flags(index);
    else
	    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool NonlinearPropertyTableModel::insertRows ( int row, int count, const QModelIndex & /*parent*/ )
{
    if ( count < 1 || row < 0 || m_readOnly )
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

bool NonlinearPropertyTableModel::removeRows ( int row, int count, const QModelIndex & /*parent*/ )
{
    if ( count < 1 || row < 0 || m_readOnly )
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
