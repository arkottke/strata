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

#ifndef TABLE_GROUP_BOX_H_
#define TABLE_GROUP_BOX_H_

#include "MyTableView.h"

#include <QGroupBox>
#include <QString>
#include <QAbstractTableModel>
#include <QPushButton>
#include <QItemDelegate>
#include <QModelIndex>

class TableGroupBox : public QGroupBox
{
    Q_OBJECT

    public:
        TableGroupBox( QAbstractTableModel * model, const QString & title, QWidget * parent = 0 );

        MyTableView * table();
        QAbstractTableModel * model();

        bool editable() const;

        bool lastRowFixed() const;
        void setLastRowFixed(bool lastRowFixed);

    public slots:
        void setEditable(bool editable);

    private slots:
        void addRow();
        void insertRow();
        void removeRow();

        void cellSelected(); //const QModelIndex & current, const QModelIndex & previous);

    signals:
        void rowRemoved();
        void dataChanged();

    private:
        //! If the size of the table can be adjusted
        bool m_editable;

        //! If the last row of the table should not be removed
        bool m_lastRowFixed;

        QPushButton * m_addButton;
        QPushButton * m_insertButton;
        QPushButton * m_removeButton;

        QAbstractTableModel * m_model;
        MyTableView * m_table;
};
#endif
