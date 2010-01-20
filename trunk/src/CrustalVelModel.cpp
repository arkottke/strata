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

#include "CrustalVelModel.h"

#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QBrush>

CrustalVelModel::CrustalVelModel( PointSourceModel * model, QObject *parent )
	: MyAbstractTableModel(false, parent), m_thickness(model->crustThickness()), m_velocity(model->crustVelocity()), m_density(model->crustDensity())
{
    connect( model, SIGNAL(crustalVelChanged()), this, SLOT(resetModel()));
    connect( this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), model, SLOT(setCrustAmpNeedsUpdate()));
}

int CrustalVelModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_thickness.size();
}

int CrustalVelModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 3;
}

QVariant CrustalVelModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// Thickness
					return tr("Thickness (km)");
                case 1:
                    // Shear-wave velocity
					return tr("Vs (km/sec)");
				case 2:
					// Density
                    return QString(tr("Density (gm/cm%1)")).arg(QChar(0x00B3));
			}
		case Qt::Vertical:
			return section+1;
		default:
			return QVariant();
	}
}

QVariant CrustalVelModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
            return QBrush(QColor(200,200,200));

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Thickness
                if ( index.row() == rowCount()-1 )
                    return tr("Infinite");
                else
                    return QString::number(m_thickness.at(index.row()));
            case 1:
                // Shear-wave velocity
                return QString::number(m_velocity.at(index.row()));
            case 2:
                // Density
                return QString::number(m_density.at(index.row()));
        }
    }
		
    return QVariant();
}

bool CrustalVelModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Thickness
                m_thickness[index.row()] = value.toDouble();
                break;
            case 1:
                // Shear-wave velocity
                m_velocity[index.row()] = value.toDouble();
                break;
            case 2:
                // Density
                m_density[index.row()] = value.toDouble();
                break;
        }
    } else {
        return false;
    }
        
    dataChanged(index, index);
    return true;
}

Qt::ItemFlags CrustalVelModel::flags ( const QModelIndex &index ) const
{
    if (m_readOnly) {
        return QAbstractTableModel::flags(index);
    }

    if (index.column() == 0 && index.row() == rowCount() - 1 ) {
        // Thickness of final row is not editable
	    return QAbstractTableModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

bool CrustalVelModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i)  {
        m_thickness.insert( row, 0 );
        m_velocity.insert( row, 0 );
        m_density.insert( row, 0 );
    }
    
    emit endInsertRows();
    
	return true;
}

bool CrustalVelModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) {
		m_thickness.remove(row);
		m_velocity.remove(row);
		m_density.remove(row);
    }
    
    emit endRemoveRows();
	
    return true;
}
