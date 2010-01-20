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

#ifndef SOIL_TYPE_TABLE_MODEL_H_
#define SOIL_TYPE_TABLE_MODEL_H_

#include "MyAbstractTableModel.h"
#include "NonlinearPropertyLibrary.h"
#include "SiteProfile.h"

#include <QAbstractTableModel>
#include <QList>

class SoilTypeTableModel : public MyAbstractTableModel
{
	Q_OBJECT
	
	public:
		SoilTypeTableModel( NonlinearPropertyLibrary * library, SiteProfile * siteProfile, QObject *parent = 0);

		int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
		int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		bool setData ( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

		QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
		Qt::ItemFlags flags ( const QModelIndex &index ) const;

		bool insertRows ( int row, int count, const QModelIndex &parent = QModelIndex() );
		bool removeRows ( int row, int count, const QModelIndex &parent = QModelIndex() );
		
		SoilType * soilType(int row);
    
	private:
        //! Factory for creating the nonlinear properties
        NonlinearPropertyLibrary * m_library;

        //! Data for the model
		SiteProfile * m_siteProfile;
};
#endif
