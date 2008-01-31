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

#include "SoilProfileModel.h"
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QBrush>

SoilProfileModel::SoilProfileModel( SiteProfile & siteProfile, Units * units, QObject *parent )
	: QAbstractTableModel(parent), m_siteProfile(siteProfile), m_units(units)
{
    connect( &m_siteProfile, SIGNAL(soilLayersChanged()), this, SLOT(resetModel()));
    connect( m_units, SIGNAL(systemChanged()), this, SLOT(resetModel()));
}

int SoilProfileModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_siteProfile.soilLayers().size() + 1;
}

int SoilProfileModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 8;
}

QVariant SoilProfileModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
                case 0:
                    // Depth
                    return QVariant(QString(tr("Depth (%1)")).arg(m_units->length()));
				case 1:
					// Thickness
					return QVariant(QString(tr("Thickness (%1)")).arg(m_units->length()));
                case 2:
                    // Soil Type
					return QVariant(tr("Soil Type"));
				case 3:
					// Shear-wave Velocity
					return QVariant(QString(tr("Average Vs (%1)")).arg(m_units->vel()));
				case 4:
					// Standard deviation
					return QVariant(tr("Stdev."));
				case 5:
					// Minimum
					return QVariant(QString(tr("Minimum (%1)")).arg(m_units->vel()));
				case 6:
					// Maximum
					return QVariant(QString(tr("Maximum (%1)")).arg(m_units->vel()));
				case 7:
					// Variation control
					return QVariant(tr("Varied"));
			}
		
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

QVariant SoilProfileModel::data ( const QModelIndex &index, int role ) const
{
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
                // Depth
                if (index.row() == rowCount()-1)
                    return QVariant(m_siteProfile.bedrock()->depth());
                else
                    return QVariant(m_siteProfile.soilLayers().at(index.row())->depth());

			case 1:
                // Thickness
                if (index.row() == rowCount()-1)
                    return QVariant(tr("Half-space"));
                else
                    return QVariant(QString::number(m_siteProfile.soilLayers().at(index.row())->thickness()));
            case 2:
                // Soil Type
                if (index.row() == rowCount()-1)
                    return QVariant(tr("Bedrock"));
                else {
                    if ( role == Qt::DisplayRole ) 
                    {
                        if ( m_siteProfile.soilLayers().at(index.row())->soilType() )
                            // A SoilType is defined
                            return QVariant( m_siteProfile.soilTypes().at(
                                        m_siteProfile.soilTypes().indexOf(
                                            const_cast<SoilType*>(m_siteProfile.soilLayers().at(index.row())->soilType())))->name());
                        else
                            // No SoilType defined
                            return QVariant();
                    }
                    else
                    {
                        // Map for a string list delegate
                        QMap<QString,QVariant> map;
                        map.insert("list", QVariant(m_siteProfile.soilTypeNameList()));
                        map.insert("index", QVariant( m_siteProfile.soilTypes().indexOf(
                                        const_cast<SoilType*>(m_siteProfile.soilLayers().at(index.row())->soilType()))));
                        return QVariant(map);
                    }
                }
			case 3:
				// Shear-wave Velocity
                if (index.row() == rowCount()-1) {
                    return QVariant(QString::number(m_siteProfile.bedrock()->avg()));
                }
                else
				    return QVariant(QString::number(m_siteProfile.soilLayers().at(index.row())->avg()));
			case 4:
				// Standard deviation
                if (index.row() == rowCount()-1)
                    return QVariant(QString::number(m_siteProfile.bedrock()->stdev()));
                else
				    return QVariant(QString::number(m_siteProfile.soilLayers().at(index.row())->stdev()));
			case 5:
				// Maximum
                if (index.row() == rowCount()-1)
                    return QVariant(QString::number(m_siteProfile.bedrock()->max()));
                else
				    return QVariant(QString::number(m_siteProfile.soilLayers().at(index.row())->max()));
			case 6:
				// Minimum
                if (index.row() == rowCount()-1)
                    return QVariant(QString::number(m_siteProfile.bedrock()->min()));
                else
				    return QVariant(QString::number(m_siteProfile.soilLayers().at(index.row())->min()));
			case 7:
				// Variation control
			default:
				return QVariant();
		}
	} else if ( role == Qt::CheckStateRole ) {
        if ( index.column() == 5 ) { // Minimum shear-wave velocity
            if (index.row() == rowCount()-1) {
                // Bedrock Layer
                if ( m_siteProfile.bedrock()->hasMin() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            } else {
                // Soil Layers
                if ( m_siteProfile.soilLayers().at(index.row())->hasMin() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            }
        } else if ( index.column() == 6 ) { // Maximum shear-wave velocity
            if (index.row() == rowCount()-1)
            {
                // Bedrock Layer
                if ( m_siteProfile.bedrock()->hasMax() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            } else {
                // Soil Layers
                if ( m_siteProfile.soilLayers().at(index.row())->hasMax() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            }
        } else if ( index.column() == 7 ) { // Varied
            if (index.row() == rowCount()-1) {
                // Bedrock Layer
                if ( m_siteProfile.bedrock()->isVaried() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            } else {
                // Soil Layers
                if ( m_siteProfile.soilLayers().at(index.row())->isVaried() )
                    return QVariant( Qt::Checked );
                else
                    return QVariant( Qt::Unchecked );
            }
        }
    }
		
    return QVariant();
}

bool SoilProfileModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Depth -- shouldn't be set
                return false;
			case 1:
                // Thickness
                if (index.row() != rowCount()-1) {
                    m_siteProfile.setThicknessAt( index.row(), value.toDouble());
                    // Update depths of layers below
                    emit dataChanged( index.sibling( index.row(), 0 ), index.sibling( rowCount()-1, 0 ));
                }
                break;
            case 2:
                // Soil Type
                if ( value.toInt() >= 0 ) {
                    if (index.row() == rowCount()-1)
                        // Bedrock -- do nothing
                        return false;
                    else
                        m_siteProfile.soilLayers()[index.row()]->setSoilType(
                                m_siteProfile.soilTypes().at(value.toInt()));
                } else
                    return false;
                break;
			case 3:
				// Shear-wave Velocity
                if (index.row() == rowCount()-1)
                    m_siteProfile.bedrock()->setAvg(value.toDouble());
                else
				    m_siteProfile.soilLayers()[index.row()]->setAvg(value.toDouble());
                break;
			case 4:
				// Standard deviation
                if (index.row() == rowCount()-1)
                    m_siteProfile.bedrock()->setStdev(value.toDouble());
                else
                    m_siteProfile.soilLayers()[index.row()]->setStdev(value.toDouble());
                break;
			case 5:
				// Maximum
                if (index.row() == rowCount()-1)
                    m_siteProfile.bedrock()->setMax(value.toDouble());
                else
                    m_siteProfile.soilLayers()[index.row()]->setMax(value.toDouble());
                break;
			case 6:
				// Minimum
                if (index.row() == rowCount()-1)
                    m_siteProfile.bedrock()->setMin(value.toDouble());
                else
                    m_siteProfile.soilLayers()[index.row()]->setMin(value.toDouble());
                break;
			case 7:
				// Variation control
			default:
				return false;
		}
    } else if ( role == Qt::CheckStateRole ) {
        if ( index.column() == 5 ) { // Minimum shear-wave velocity
            if (index.row() == rowCount()-1)
                m_siteProfile.bedrock()->setHasMin(value.toBool());
            else
                m_siteProfile.soilLayers()[index.row()]->setHasMin(value.toBool());
        } else if ( index.column() == 6 ) {  // Maximum shear-wave velocity
            if (index.row() == rowCount()-1)
                m_siteProfile.bedrock()->setHasMax(value.toBool());
            else
                m_siteProfile.soilLayers()[index.row()]->setHasMax(value.toBool());
        } else if ( index.column() == 7 ) { // Varied
            if (index.row() == rowCount()-1)
                m_siteProfile.bedrock()->setIsVaried(value.toBool());
            else
                m_siteProfile.soilLayers()[index.row()]->setIsVaried(value.toBool());
        } else 
            return false;
    } 
        
    dataChanged(index, index);
    return true;
}

Qt::ItemFlags SoilProfileModel::flags ( const QModelIndex &index ) const
{
    if (index.column() == 0 )
        // Depth column -- not editable
	    return QAbstractTableModel::flags(index);

    if (index.row() == rowCount() - 1 &&
            ( index.column() == 1 || index.column() == 2 ) )
        // Bedrock thickness and soil type -- not editable
	    return QAbstractTableModel::flags(index);

    if (index.column() == 5 ) {
        // Min column
        if ( index.row() == rowCount() - 1 )
        {
            // Bedrock 
            if ( m_siteProfile.bedrock()->hasMin() )
                return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | QAbstractTableModel::flags(index);
            else
                return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        } 
        else
        {
            // Soil
            if ( m_siteProfile.soilLayers().at(index.row())->hasMin())
                return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | QAbstractTableModel::flags(index);
            else
                return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        }
    }
    
    if (index.column() == 6 ) {
        // Max column
        if ( index.row() == rowCount() - 1 )
        {
            // Bedrock 
            if ( m_siteProfile.bedrock()->hasMax() )
                return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | QAbstractTableModel::flags(index);
            else
                return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        } 
        else
        {
            // Soil
            if ( m_siteProfile.soilLayers().at(index.row())->hasMax())
                return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | QAbstractTableModel::flags(index);
            else
                return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        }
    }

    if (index.column() == 7)
        // Variation control column -- checkbox only
	    return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);

    // Default editable
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool SoilProfileModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

    // Because of the extra layer in the model the row might need to be reduced by one
    if ( row > m_siteProfile.soilLayers().size() )
        row = m_siteProfile.soilLayers().size();

	for (int i=0; i < count; ++i) 
		m_siteProfile.soilLayers().insert( row, new SoilLayer ); 	
    
    // Update the depths of the layers
    m_siteProfile.updateDepths();

    emit endInsertRows();
    
	return true;
}

bool SoilProfileModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) 
		delete m_siteProfile.soilLayers().takeAt(row); 	
    
    // Update the depths of the layers
    m_siteProfile.updateDepths();
	
    emit endRemoveRows();
	
    return true;
}

void SoilProfileModel::resetModel()
{
    reset();
}
