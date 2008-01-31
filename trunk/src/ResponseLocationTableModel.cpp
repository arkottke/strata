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

#include "ResponseLocationTableModel.h"
#include <QBrush>
#include <QDebug>

ResponseLocationTableModel::ResponseLocationTableModel( SiteResponseOutput * model, QObject *parent )
	: QAbstractTableModel(parent), m_model(model)
{
    connect( m_model, SIGNAL(responseLocationsChanged()), this, SLOT(resetModel()));
}

int ResponseLocationTableModel::rowCount ( const QModelIndex & /* index */ ) const
{
	return m_model->responseLocations().size();
}

int ResponseLocationTableModel::columnCount ( const QModelIndex & /* index */ ) const
{
	return 9;
}

QVariant ResponseLocationTableModel::data ( const QModelIndex & index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();
    
    // Color the background to red and green for the check state
    if (role==Qt::BackgroundRole && flags(index) & Qt::ItemIsUserCheckable) {
        if ( data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked ) 
            return QVariant(QBrush(QColor(200,200,200)));
        else 
            return QVariant(QBrush(QColor(50,200,50)));
    }

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                {
                    // Location
                    double depth = m_model->responseLocations().at(index.row())->depth();
                    if ( role == Qt::DisplayRole && depth < 0)
                        return QVariant(tr("Bedrock"));
                    else
                        return QVariant(QString::number(depth));
                }
            case 1:
                // Layer type
                if ( role == Qt::DisplayRole ) {
                    switch (m_model->responseLocations().at(index.row())->type())
                    {
                        case Motion::Outcrop:
                            return QVariant("Outcrop");
                        case Motion::Within:
                            return QVariant("Within");
                    }
                } else {
                    QMap<QString, QVariant> map;
                    // Values to choose from
                    map.insert("list", Motion::typeList());
                    // Selected value
                    map.insert("index", m_model->responseLocations().at(index.row())->type());
                    return QVariant(map);
                }
			default:
				return QVariant();
		}
    } else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
            case 0:
                // Location
            case 1:
                // Layer type
                return QVariant();
            case 2:
                // Acceleration response spectra
                if (m_model->responseLocations()[index.row()]->respSpec()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 3:
                // Acceleration time series
                if (m_model->responseLocations()[index.row()]->accelTs()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 4:
                // Velocity time series
                if (m_model->responseLocations()[index.row()]->velTs()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 5:
                // Displacement time series
                if (m_model->responseLocations()[index.row()]->dispTs()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 6:
                // Shear stress time series
                if (m_model->responseLocations()[index.row()]->stressTs()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 7:
                // Shear strain time series
                if (m_model->responseLocations()[index.row()]->strainTs()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 8:
                // Baseline corrected
                if (m_model->responseLocations()[index.row()]->isBaselineCorrected())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
        }
    }
	
	return QVariant();
}

bool ResponseLocationTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Location
                qDebug() << "setting location";
                m_model->responseLocations()[index.row()]->setDepth(value.toDouble());
                break;
            case 1:
                // Layer type
                m_model->responseLocations()[index.row()]->setType(
                        (Motion::Type)value.toInt());
                break;
            default:
				return false;
		}
	} else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
            case 0:
                // Location
            case 1:
                // Layer type
                return false;
            case 2:
                // Acceleration response spectra
                m_model->responseLocations()[index.row()]->respSpec()->setEnabled(value.toBool());
                break;
            case 3:
                // Acceleration time series
                m_model->responseLocations()[index.row()]->accelTs()->setEnabled(value.toBool());
                break;
            case 4:
                // Velocity time series
                m_model->responseLocations()[index.row()]->velTs()->setEnabled(value.toBool());
                break;
            case 5:
                // Displacement time series
                m_model->responseLocations()[index.row()]->dispTs()->setEnabled(value.toBool());
                break;
            case 6:
                // Shear stress time series
                m_model->responseLocations()[index.row()]->stressTs()->setEnabled(value.toBool());
                break;
            case 7:
                // Shear strain time series
                m_model->responseLocations()[index.row()]->strainTs()->setEnabled(value.toBool());
                break;
            case 8:
                // Baseline corrected
                m_model->responseLocations()[index.row()]->setBaselineCorrected(value.toBool());
                break;
        }
    } else 
		return false;
    
    dataChanged(index, index);
    return true;
}

QVariant ResponseLocationTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// Location
					return QVariant(tr("Location"));
                case 1:
                    // Layer type
                    return QVariant(tr("Type"));
                case 2:
                    // Acceleration response spectra
                    return QVariant(tr("Accel. Resp. Spec."));
                case 3:
                    // Acceleration time series
                    return QVariant(tr("Accel-Time"));
                case 4:
                    // Velocity time series
                    return QVariant(tr("Vel-Time"));
                case 5:
                    // Displacement time series
                    return QVariant(tr("Disp-Time"));
                case 6:
                    // Shear stress time series
                    return QVariant(tr("Shear stress-Time"));
                case 7:
                    // Shear strain time series
                    return QVariant(tr("Shear strain-Time"));
                case 8:
                    // Baseline corrected
                    return QVariant(tr("Base-line corrected"));
			}
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags ResponseLocationTableModel::flags ( const QModelIndex &index ) const
{
    switch (index.column())
    {
        case 0:
            // Location
        case 1:
            // Layer type
            return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
        case 2:
            // Acceleration response spectra
        case 3:
            // Acceleration time series
        case 4:
            // Velocity time series
        case 5:
            // Displacement time series
        case 6:
            // Shear stress time series
        case 7:
            // Shear strain time series
        case 8:
            // Baseline corrected
            return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
        default:
            return QAbstractTableModel::flags(index);
    }
}

bool ResponseLocationTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i) 
		m_model->responseLocations().insert( row, new ResponseLocation ); 	

	emit endInsertRows();

	return true;
}

bool ResponseLocationTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
	
	for (int i = 0; i < count; ++i)
        delete m_model->responseLocations().takeAt(row);

	emit endRemoveRows();
	return true;
}

void ResponseLocationTableModel::resetModel()
{
    reset();
}
