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

#include "RatioLocationTableModel.h"
#include <QBrush>
#include <QDebug>

RatioLocationTableModel::RatioLocationTableModel( SiteResponseOutput * model, QObject *parent )
	: QAbstractTableModel(parent), m_model(model)
{
    connect( m_model, SIGNAL(ratioLocationsChanged()), this, SLOT(resetModel()));
}

int RatioLocationTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_model->ratioLocations().size();
}

int RatioLocationTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 6;
}

QVariant RatioLocationTableModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

    // Color the background to red and green for the check state
    if (role==Qt::BackgroundRole && flags(index) & Qt::ItemIsUserCheckable) {
        if ( data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked) 
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
                    // To Depth
                    double depth = m_model->ratioLocations().at(index.row())->toDepth();
                    if ( role == Qt::DisplayRole && depth < 0)
                        return QVariant(tr("Bedrock"));
                    else
                        return QVariant(QString::number(depth));
                }
            case 1:
                // To Layer type
                if ( role == Qt::DisplayRole ) {
                    switch (m_model->ratioLocations().at(index.row())->toType())
                    {
                        case Motion::Outcrop:
                            return QVariant(tr("Outcrop"));
                        case Motion::Within:
                            return QVariant(tr("Within"));
                    }
                } else {
                    QMap<QString, QVariant> map;
                    // Values to choose from
                    map.insert("list", Motion::typeList());
                    // Selected value
                    map.insert("index", m_model->ratioLocations().at(index.row())->toType());
                    return QVariant(map);
                }
            case 2:
                {
                    // From Depth
                    double depth = m_model->ratioLocations().at(index.row())->fromDepth();
                    if ( role == Qt::DisplayRole && depth < 0)
                        return QVariant(tr("Bedrock"));
                    else
                        return QVariant(QString::number(depth));
                }
            case 3:
                // From Layer type
                if ( role == Qt::DisplayRole ) {
                    switch (m_model->ratioLocations().at(index.row())->fromType())
                    {
                        case Motion::Outcrop:
                            return QVariant(tr("Outcrop"));
                        case Motion::Within:
                            return QVariant(tr("Within"));
                    }
                } else {
                    QMap<QString, QVariant> map;
                    // Values to choose from
                    map.insert("list", Motion::typeList());
                    // Selected value
                    map.insert("index", m_model->ratioLocations().at(index.row())->fromType());
                    return map;
                }
            case 4:
                // Acceleration transfer function
            case 5:
                // Acceleration response spectrum ratio
            default:
                return QVariant();
		}
    } else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
            case 0:
                // To Depth
            case 1:
                // To Layer type
            case 2:
                // From Depth
            case 3:
                // From Layer type
                return QVariant();
            case 4:
                // Acceleration transfer function
                if (m_model->ratioLocations()[index.row()]->transFunc()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            case 5:
                // Acceleration response spectrum ratio
                if (m_model->ratioLocations()[index.row()]->respRatio()->enabled())
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
        }
    }

	return QVariant();
}

bool RatioLocationTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // To Depth
                m_model->ratioLocations()[index.row()]->setToDepth(value.toDouble());
                break;
            case 1:
                // To Layer type
                m_model->ratioLocations()[index.row()]->setToType(
                        (Motion::Type)value.toInt());
                break;
            case 2:
                // To Depth
                m_model->ratioLocations()[index.row()]->setFromDepth(value.toDouble());
                break;
            case 3:
                // To Layer type
                m_model->ratioLocations()[index.row()]->setFromType(
                        (Motion::Type)value.toInt());
                break;
            case 4:
                // Acceleration transfer function
            case 5:
                // Acceleration response spectrum ratio
            default:
				return false;
		}
    } else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
            case 0:
                // To Depth
            case 1:
                // To Layer type
            case 2:
                // From Depth
            case 3:
                // From Layer type
                return false;
            case 4:
                // Acceleration transfer function
                m_model->ratioLocations()[index.row()]->transFunc()->setEnabled(value.toBool());
                break;
            case 5:
                // Acceleration response spectrum ratio
                m_model->ratioLocations()[index.row()]->respRatio()->setEnabled(value.toBool());
                break;
        }
    } else
		return false;

    dataChanged(index, index);
    return true;
}

QVariant RatioLocationTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// To Depth
					return QVariant(tr("Location 1"));
                case 1:
                    // To Layer type
                    return QVariant(tr("Type 1"));
				case 2:
					// From Depth
					return QVariant(tr("Location 2"));
                case 3:
                    // From Layer type
                    return QVariant(tr("Type 2"));
                case 4:
                    // Acceleration transfer function
                    return QVariant(tr("FAS_1 / FAS_2 (accel)"));
                case 5:
                    // Acceleration response spectrum ratio
                    return QVariant(tr("Sa_1 / Sa_2"));
			}
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags RatioLocationTableModel::flags ( const QModelIndex &index ) const
{
    switch (index.column())
    {
        case 0:
            // To Depth
        case 1:
            // To Layer type
        case 2:
            // From Depth
        case 3:
            // From Layer type
            return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
        case 4:
            // Acceleration transfer function
        case 5:
            // Acceleration response spectrum ratio
            return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    }
            
    return QAbstractTableModel::flags(index);
}

bool RatioLocationTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i)
		m_model->ratioLocations().insert( row, new RatioLocation );

	emit endInsertRows();

	return true;
}

bool RatioLocationTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);

	for (int i = 0; i < count; ++i)
        delete m_model->ratioLocations().takeAt(row);

	emit endRemoveRows();

	return true;
}

void RatioLocationTableModel::resetModel()
{
    reset();
}
