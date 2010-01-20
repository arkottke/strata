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

#include "CrustalAmpModel.h"

#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QBrush>

CrustalAmpModel::CrustalAmpModel( PointSourceModel * model, QObject *parent )
	: MyAbstractTableModel(true, parent), m_freq(model->freq()), m_amp(model->crustAmp())
{
    connect( model, SIGNAL(crustalAmpChanged()), this, SLOT(resetModel()));
}

int CrustalAmpModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_freq.size();
}

int CrustalAmpModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 2;
}

QVariant CrustalAmpModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
				case 0:
					// Frequency 
					return tr("Freq. (Hz.)");
                case 1:
                    // Amplification
					return tr("Amp.");
			}
		case Qt::Vertical:
			return section+1;
		default:
			return QVariant();
	}
}

QVariant CrustalAmpModel::data ( const QModelIndex &index, int role ) const
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
                // Frequency 
                return QString::number(m_freq.at(index.row()));
            case 1:
                // Amplification
                return QString::number(m_amp.at(index.row()));
        }
    }
		
    return QVariant();
}

bool CrustalAmpModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Frequency 
                m_freq[index.row()] = value.toDouble();
                break;
            case 1:
                // Amplification
                m_amp[index.row()] = value.toDouble();
                break;
        }
    } else {
        return false;
    }
        
    dataChanged(index, index);
    return true;
}

Qt::ItemFlags CrustalAmpModel::flags ( const QModelIndex &index ) const
{
    if (m_readOnly) {
        return QAbstractTableModel::flags(index);
    }

    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool CrustalAmpModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i)  {
        m_freq.insert( row, 0 );
        m_amp.insert( row, 0 );
    }
    
    emit endInsertRows();
    
	return true;
}

bool CrustalAmpModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) {
		m_freq.remove(row);
		m_amp.remove(row);
    }
    
    emit endRemoveRows();
	
    return true;
}
