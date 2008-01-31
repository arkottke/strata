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

#include "ResponseSpectrumModel.h"
#include <QString>
#include <QDebug>

ResponseSpectrumModel::ResponseSpectrumModel( ResponseSpectrum * respSpec, bool isEditable, QObject *parent)
	: QAbstractTableModel(parent), m_respSpec(respSpec), m_isEditable(isEditable)
{
    connect( respSpec, SIGNAL(dataChanged()), this, SLOT(resetModel()));
    connect( this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), respSpec, SLOT(setModified()));
}

int ResponseSpectrumModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_respSpec->period().size();
}

int ResponseSpectrumModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 2;
}

QVariant ResponseSpectrumModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// Period
					return QVariant(tr("Period (s)"));
				case 1:
					// Spectral Acceleration
					return QVariant(tr("Spec. Accel. (g)"));
			}
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

QVariant ResponseSpectrumModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Period
                return QVariant(QString::number(m_respSpec->period().at(index.row())));
            case 1:
                // Spectral Acceleration
                return QVariant(QString::number(m_respSpec->sa().at(index.row())));
			default:
				return QVariant();
		}
    }
	
	return QVariant();
}

bool ResponseSpectrumModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Period
                m_respSpec->period()[index.row()] = value.toDouble();
                break;
            case 1:
                // Spectral Acceleration
                m_respSpec->sa()[index.row()] = value.toDouble();
                break;
			default:
				return false;
		}
	} 
	else 
		return false;
    
    dataChanged(index, index);

    return true;
}

Qt::ItemFlags ResponseSpectrumModel::flags ( const QModelIndex &index ) const
{
    if ( m_isEditable )
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

bool ResponseSpectrumModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i) {
        m_respSpec->period().insert(row, 0);
        m_respSpec->sa().insert(row, 0);
    }

	emit endInsertRows();

	return true;
}

bool ResponseSpectrumModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) {
        m_respSpec->period().remove(row);
        m_respSpec->sa().remove(row);
    }
	
	emit endRemoveRows();
	return true;
}

void ResponseSpectrumModel::resetModel()
{
    reset();
}

