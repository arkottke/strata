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

#ifndef DEPTH_COMBO_BOX_DELEGATE_H_
#define DEPTH_COMBO_BOX_DELEGATE_H_

#include <QItemDelegate>

//! Delegate to allow for the DepthComboBox to be used in a table view.

class DepthComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT

    public:
        DepthComboBoxDelegate( QObject * parent = 0 );

        QWidget * createEditor ( QWidget *parent, const QStyleOptionViewItem
                &option, const QModelIndex &index ) const;

        void setEditorData( QWidget *editor, const QModelIndex &index ) const;
        void setModelData( QWidget *editor, QAbstractItemModel *model, const
                QModelIndex &index ) const;
};
#endif
