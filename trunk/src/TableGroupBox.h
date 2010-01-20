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

#include "MyAbstractTableModel.h"
#include "MyTableView.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QItemDelegate>
#include <QModelIndex>
#include <QPushButton>
#include <QString>

class TableGroupBox : public QGroupBox
{
    Q_OBJECT

    public:
        TableGroupBox( MyAbstractTableModel * model, const QString & title, QWidget * parent = 0 );

        MyTableView * table();
        QAbstractTableModel * model();

        bool readOnly() const;

        bool lastRowFixed() const;
        void setLastRowFixed(bool lastRowFixed);

        void addButton( QPushButton * pushButton);

    public slots:
        void setReadOnly(bool readOnly);

    private slots:
        void addRow();
        void insertRow();
        void removeRow();

        void cellSelected(); //const QModelIndex & current, const QModelIndex & previous);

    signals:
        void rowRemoved();
        void dataChanged();

    private:
        //! If table is read only
        bool m_readOnly;

        //! If the last row of the table should not be removed
        bool m_lastRowFixed;

        QHBoxLayout * m_buttonRow;

        QPushButton * m_addButton;
        QPushButton * m_insertButton;
        QPushButton * m_removeButton;

        QList<QPushButton*> m_addedButtons;

        MyTableView * m_table;
        MyAbstractTableModel * m_model;
};
#endif
