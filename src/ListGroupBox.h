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

#ifndef LIST_GROUP_BOX_H_
#define LIST_GROUP_BOX_H_

#include <QGroupBox>
#include <QString>
#include <QListView>
#include <QAbstractListModel>
#include <QPushButton>

class ListGroupBox : public QGroupBox
{
    Q_OBJECT

    public:
        ListGroupBox( QAbstractListModel * model, const QString & title, QWidget * parent = 0 );

        QListView * view();
        QAbstractListModel * model();

    public slots:
        void removeEnabled(bool b);

    protected slots:
        void addRow();
        void removeRow();

        void selectionChanged();

    signals:
        void rowRemoved();
        void dataChanged();
        void rowSelected(const QModelIndex &current);

    private:
        QPushButton * m_addButton;
        QPushButton * m_removeButton;

        QAbstractListModel * m_model;
        QListView * m_view;
};
#endif
