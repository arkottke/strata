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

#include "OutputCsvSelectionTableModel.h"

OutputCsvSelectionTableModel::OutputCsvSelectionTableModel( QList<Output *> & outputList, QObject *parent )
	: QAbstractTableModel(parent), m_outputList(outputList)
{
}

int OutputCsvSelectionTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
    return m_outputList.size();
}

int OutputCsvSelectionTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 1;
}

QVariant OutputCsvSelectionTableModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Output name
                return QVariant(m_outputList.at(index.row())->name());
			default:
				return QVariant();
		}
	} 
    else if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        if ( m_outputList.at(index.row())->exportEnabled() )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }
	
	return QVariant();
}

bool OutputCsvSelectionTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

    if ( index.column() == 0 && role == Qt::CheckStateRole )
        m_outputList[index.row()]->setExportEnabled(value.toBool());
    
    dataChanged(index, index);
    return true;
}

QVariant OutputCsvSelectionTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
                case 0:
                    // Output name
                    return QVariant(tr("Output"));
			}
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags OutputCsvSelectionTableModel::flags ( const QModelIndex &index ) const
{
    if ( index.column() == 0 )
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}
