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

#ifndef NON_LINEAR_PROPERTY_LIST_MODEL_H_
#define NON_LINEAR_PROPERTY_LIST_MODEL_H_

#include "NonLinearProperty.h"

#include <QAbstractListModel>
#include <QList>

class NonLinearPropertyListModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        NonLinearPropertyListModel( QList<NonLinearProperty> &nlPropertyList, QObject *parent = 0);

        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        QVariant data(const QModelIndex &index, int role) const;
        bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

        Qt::ItemFlags flags ( const QModelIndex & index ) const;

        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

        NonLinearProperty & nlPropertyAt( const QModelIndex &index );

    private:
        QList<NonLinearProperty> &m_nlPropertyList;
};
#endif
