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

#include "RecordedMotionModel.h"
#include "RecordedMotionDialog.h"
#include <QBrush>
#include <QDebug>

RecordedMotionModel::RecordedMotionModel(  QList<RecordedMotion*> & motions, QObject * parent)
    : QAbstractTableModel(parent), m_motions(motions)
{
}

int RecordedMotionModel::rowCount ( const QModelIndex & /*parent*/) const
{
    return m_motions.size();
}

int RecordedMotionModel::columnCount ( const QModelIndex & /*parent*/) const
{
    return 5;
}

QVariant RecordedMotionModel::data ( const QModelIndex & index, int role) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !m_motions.at(index.row())->isEnabled())
            return QVariant(QBrush(QColor(170,170,170)));
    else if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
            return QVariant(QBrush(QColor(200,200,200)));

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Filename
                return QVariant(m_motions.at(index.row())->toString());
            case 1:
                // Description
                return QVariant(m_motions.at(index.row())->description());
            case 2:
                // Type
                if ( role == Qt::DisplayRole )
                    // Return a simple label
                    return QVariant( Motion::typeList().at((int)m_motions.at(index.row())->type()));
                else {
                    // Return a map that can be edited with the StringListDelegate
                    QMap<QString,QVariant> map;
                    map.insert("list", QVariant(Motion::typeList()));
                    map.insert("index", QVariant((int) m_motions.at(index.row())->type()));
                    return QVariant(map);
                }
            case 3:
                // Scale
                return QVariant(QString::number(m_motions.at(index.row())->scale()));
            case 4:
                // PGA
                return QVariant(QString::number(m_motions.at(index.row())->pga()));
			default:
				return QVariant();
		}
	} else if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        if ( m_motions.at(index.row())->isEnabled() )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }
	
	return QVariant();
}

bool RecordedMotionModel::setData ( const QModelIndex &index, const QVariant &value, int role)
{
	if(index.parent()!=QModelIndex())
		return false;

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Filename
                return false;
            case 1:
                // Description
                return false;
            case 2:
                // Type
                m_motions[index.row()]->setType((Motion::Type)(value.toInt()));
                break;
            case 3:
                // Scale
                m_motions[index.row()]->setScale(value.toDouble());
                break;
            case 4:
                // PGA
                m_motions[index.row()]->setPga(value.toDouble());
                break;
            default:
                return false;
		}
	} else if ( index.column() == 0 && role == Qt::CheckStateRole ) 
		m_motions[index.row()]->setIsEnabled(value.toBool());

	else 
		return false;
   
    if ( role == Qt::CheckStateRole )
        // Change entire row information
        dataChanged(index.sibling(index.row(), 0), index.sibling(index.row(), columnCount()));
    else
        dataChanged(index, index);
    return true;
}

QVariant RecordedMotionModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
    {
        case Qt::Horizontal:
            switch (section)
            {
                case 0:
                    // Filename
                    return QVariant(tr("Filename"));
                case 1:
                    // Description
                    return QVariant(tr("Description"));
                case 2:
                    // Type
                    return QVariant(tr("Type"));
                case 3:
                    // Scale
                    return QVariant(tr("Scale Factor"));
                case 4:
                    // PGA
                    return QVariant(tr("PGA (g)"));
            }
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags RecordedMotionModel::flags ( const QModelIndex & index ) const
{
    if (index.column() == 0)
	    return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    else if ( index.column() == 2 || index.column() == 3 || index.column() == 4)
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

bool RecordedMotionModel::insertRows ( int row, int count, const QModelIndex & parent)
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i) {
        RecordedMotion * motion = new RecordedMotion;
        // Create a dialog to define the motion properties
        RecordedMotionDialog * dialog = new RecordedMotionDialog(motion, m_workingDir);
        //dialog->setWindowModality(Qt::WindowModal); 

        if (dialog->exec())
            // Insert the row
            m_motions.insert( row, motion); 	
        else {
            delete motion;
            return false;
        }
    }

	emit endInsertRows();

	return true;
}

bool RecordedMotionModel::removeRows ( int row, int count, const QModelIndex & parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
	
    // Remove each of the rows    
	for (int i=0; i < count; ++i) 
		delete m_motions.takeAt(row); 	
	
	emit endRemoveRows();
	
	return true;
}

void RecordedMotionModel::resetModel()
{
    reset();
}

