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
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef MY_PLOT_H_
#define MY_PLOT_H_

#include <qwt_plot.h>

#include <QMenu>
#include <QContextMenuEvent>

class MyPlot : public QwtPlot
{
    public:
        MyPlot(QWidget * parent = 0);
        MyPlot(const QwtText & title, QWidget * parent = 0);

    protected:
        void contextMenuEvent( QContextMenuEvent * event );

    protected slots:
        //! Copy the plot the clipboard
        void copyPlot();

    private:
        QwtPlot * plot;

        QMenu * _contextMenu;

        //! Create the context menu
        void createContextMenu();
};
#endif
