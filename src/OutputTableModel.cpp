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

#include "OutputTableModel.h"
#include <QDebug>

OutputTableModel::OutputTableModel( SiteResponseOutput * model, QObject *parent )
	: QAbstractTableModel(parent), m_model(NULL), m_selectedOutput(NULL)
{
    setModel(model);
}

int OutputTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
    if ( !m_model || !m_selectedOutput )
        return 0;
    else if ( m_selectedOutput->isMotionIndependent() )
        return m_model->siteCount();
    else if ( m_selectedOutput->isSiteIndependent() )
        return m_model->motionCount();
    else 
        return m_model->totalCount();
}

int OutputTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 2;
}

QVariant OutputTableModel::data ( const QModelIndex &index, int role ) const
{
	if ( !m_model || !m_selectedOutput || index.parent()!=QModelIndex())
		return QVariant();

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
				// Site number
                if ( m_selectedOutput->isMotionIndependent() ) 
                    return QVariant( 1 + index.row() );
                else if ( m_selectedOutput->isSiteIndependent() )
                    return QVariant("---");
                else
                    return QVariant( 1 + index.row() % m_model->siteCount() );
            case 1:
                // Motion number
                if ( m_selectedOutput->isMotionIndependent() ) 
                    return QVariant("---");
                else if ( m_selectedOutput->isSiteIndependent() )
                    return QVariant( m_model->motionNames().at(index.row()) );
                else
                return QVariant( m_model->motionNames().at(index.row() % m_model->motionCount()) );
			default:
				return QVariant();
		}
	} else if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        bool enabled;

        if ( m_selectedOutput->isMotionIndependent() ) 
            enabled = m_model->siteEnabledAt( index.row() );
        else if ( m_selectedOutput->isSiteIndependent() )
            enabled = m_model->motionEnabledAt( index.row() );
        else
            enabled = m_model->seriesEnabled().at(index.row());

        if ( enabled )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }
	
	return QVariant();
}

bool OutputTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

    if ( index.column() == 0 && role == Qt::CheckStateRole ) {
        if ( m_selectedOutput->isMotionIndependent() ) 
            m_model->setSiteEnabledAt( index.row(), value.toBool() );
        else if ( m_selectedOutput->isSiteIndependent() )
            m_model->setMotionEnabledAt( index.row(), value.toBool() );
        else
            m_model->seriesEnabled()[index.row()] = value.toBool();

        emit enabledChanged();
    } else 
		return false;
    
    emit dataChanged(index, index);

    return true;
}

QVariant OutputTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
                case 0:
                    // Site number
                    return QVariant(tr("Site Realization"));
                case 1:
                    // Motion number
                    return QVariant(tr("Input Motion"));
			}
		case Qt::Vertical:
			return QVariant(section+1);
		default:
			return QVariant();
	}
}

Qt::ItemFlags OutputTableModel::flags ( const QModelIndex &index ) const
{
    if ( index.column() == 0 )
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

void OutputTableModel::setModel( SiteResponseOutput * model )
{
    // Disconnect old connections
    if (m_model)
        disconnect(m_model, 0, this, 0);

    // Save the model
    m_model = model;

    // Reform connections
    connect( m_model, SIGNAL(enabledChanged()), this, SLOT(updateEnabled()));

    // Signal that the data must be reset
    reset();
}

void OutputTableModel::setSelectedOutput( const Output * selectedOutput )
{
    m_selectedOutput = selectedOutput;
    reset();
}

void OutputTableModel::updateEnabled()
{
    emit dataChanged( index(0,0), index(0, rowCount()));
}
