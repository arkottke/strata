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

#include "SoilTypeTableModel.h"
#include <QDebug>

SoilTypeTableModel::SoilTypeTableModel( NonLinearPropertyLibrary * library, SiteProfile & siteProfile, Units * units, QObject *parent )
	: QAbstractTableModel(parent), m_library(library), m_siteProfile(siteProfile), m_units(units)
{
    connect(&m_siteProfile, SIGNAL(soilTypeChanged()), this, SLOT(resetModel()));
    connect( m_units, SIGNAL(systemChanged()), this, SLOT(resetModel()));
}

int SoilTypeTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_siteProfile.soilTypes().size();
}

int SoilTypeTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 7;
}

QVariant SoilTypeTableModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
				// Name Column
				return QVariant( m_siteProfile.soilTypes().at(index.row())->name() );
            case 1:
                // Unit Weight
                return QVariant( QString::number(m_siteProfile.soilTypes().at(index.row())->untWt() ));
			case 2:
                // Initial damping
                return QVariant( QString::number(m_siteProfile.soilTypes().at(index.row())->initialDamping()));
			case 3:
				// Shear-modulus model
                if ( role == Qt::DisplayRole ) 
				    return QVariant( m_siteProfile.soilTypes().at(index.row())->normShearMod().name() );
                else
                {
                    QMap<QString,QVariant> map;

                    map.insert("list", m_library->modulusList());
                    map.insert("index", QVariant(
                                m_library->modulusList().indexOf(
                                    m_siteProfile.soilTypes().at(index.row())->normShearMod().name())));
                    return QVariant(map);
                }
			case 4:
				// Damping model
                if ( role == Qt::DisplayRole ) 
				    return QVariant( m_siteProfile.soilTypes().at(index.row())->damping().name() );
                else
                {
                    QMap<QString,QVariant> map;
                    
                    map.insert("list", m_library->dampingList());
                    map.insert("index", QVariant(
                                m_library->dampingList().indexOf(
                                    m_siteProfile.soilTypes().at(index.row())->damping().name())));

                    return QVariant(map);
                }
			case 5:
                // Notes
				return QVariant( m_siteProfile.soilTypes().at(index.row())->notes() );
            case 6:
				// Is Varied Check box
			default:
				return QVariant();
		}
	} else if ( index.column() == 6 && role == Qt::CheckStateRole )
    {
        if ( m_siteProfile.soilTypes().at(index.row())->isVaried() )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }
	
	return QVariant();
}

bool SoilTypeTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
				// Name Column
				m_siteProfile.soilTypes()[index.row()]->setName(value.toString());
				break;
            case 1:
                // Unit Weight
                m_siteProfile.soilTypes()[index.row()]->setUntWt(value.toDouble());
                break;
			case 2:
                // Initial damping
                m_siteProfile.soilTypes()[index.row()]->setInitialDamping(value.toDouble());
                break;
			case 3:
				// Shear modulus
                if ( value.toInt() < 0 )
                    return false;

                m_siteProfile.soilTypes()[index.row()]->setNormShearMod(
                        m_library->modulus().at(value.toInt()));

				break;
			case 4:
				// Damping model
                if ( value.toInt() < 0 )
                    return false;

                m_siteProfile.soilTypes()[index.row()]->setDamping(
                        m_library->damping().at(value.toInt()));
				break;
            case 5:
                // Notes
				m_siteProfile.soilTypes()[index.row()]->setNotes(value.toString());
				break;
            case 6:
				// Is Varied Check box
			default:
				return false;
		}
	} 
    else if ( index.column() == 6 && role == Qt::CheckStateRole )
		m_siteProfile.soilTypes()[index.row()]->setIsVaried(value.toBool());
	else 
		return false;
    
    emit dataChanged(index, index);
    return true;
}

QVariant SoilTypeTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
                case 0:
					// Name Column
					return QVariant(tr("Name"));
                case 1:
                    // Unit weight
                    return QVariant(QString(tr("Unit Weight (%1)")).arg(m_units->untWt()));
				case 2:
                    // Initial damping
					return QVariant(tr("Initial Damping (%)"));
				case 3:
					// Shear-modulus model
					return QVariant(tr("G/G_max Model"));
				case 4:
					// Damping model
					return QVariant(tr("Damping Model"));
                case 5:
                    // Notes
                    return QVariant(tr("Notes"));
				case 6:
					// Is varied Check box
					return QVariant(tr("Varied"));
			}
		
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags SoilTypeTableModel::flags ( const QModelIndex &index ) const
{
    if ( index.column() == 6 )
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    else
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool SoilTypeTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1  );
	
    for (int i=0; i < count; ++i) 
		m_siteProfile.soilTypes().insert( row, new SoilType(m_siteProfile.units()) ); 	

	emit endInsertRows();

	return true;
}

bool SoilTypeTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
	
    for (int i=0; i < count; ++i)
		delete m_siteProfile.soilTypes().takeAt(row); 	
	
	emit endRemoveRows();
	
	return true;
}
	
SoilType * SoilTypeTableModel::soilType(int row)
{
	return m_siteProfile.soilTypes()[row];
}

void SoilTypeTableModel::resetModel()
{
    reset();
}
