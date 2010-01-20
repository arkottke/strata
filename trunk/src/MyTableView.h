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

#ifndef MY_TABLE_VIEW_H_
#define MY_TABLE_VIEW_H_

#include <QTableView>
#include <QContextMenuEvent>
#include <QMenu>

class MyTableView : public QTableView
{
    Q_OBJECT

    public:
        MyTableView( QWidget * parent = 0 );

    signals:
        void dataPasted();

    public slots:
        void copy();
        void paste();

    protected:
        void contextMenuEvent( QContextMenuEvent * event );

    private:
        QMenu * m_contextMenu;
};
#endif
