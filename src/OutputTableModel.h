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

#ifndef OUTPUT_TABLE_MODEL_H_
#define OUTPUT_TABLE_MODEL_H_

#include "SiteResponseOutput.h"
#include "Output.h"
#include <QAbstractTableModel>

class OutputTableModel : public QAbstractTableModel 
{
	Q_OBJECT
	
	public:
		OutputTableModel( SiteResponseOutput * model, QObject *parent = 0);

		int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
		int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		bool setData ( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

		QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
		Qt::ItemFlags flags ( const QModelIndex &index ) const;

        void setModel( SiteResponseOutput * model );
        void setSelectedOutput( const Output * selectedOutput );

    signals:
        void enabledChanged();

    public slots:
        //! Refresh all values in the enabled column
        void updateEnabled();

	private:
        //! Data for the model
		SiteResponseOutput * m_model;

        //! Selected output
        const Output * m_selectedOutput;
};
#endif
