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

#include "MyPlot.h"
#include <QImage>
#include <QApplication>
#include <QClipboard>

MyPlot::MyPlot(QWidget * parent)
    : QwtPlot(parent)
{
    createContextMenu();
}

MyPlot::MyPlot(const QwtText & title, QWidget * parent)
    : QwtPlot(title, parent)
{
    createContextMenu();
}

void MyPlot::contextMenuEvent( QContextMenuEvent * event )
{
    m_contextMenu->popup(event->globalPos());
}

void MyPlot::createContextMenu()
{
    m_contextMenu = new QMenu;
    
    m_contextMenu->addAction(tr("Copy"), this, SLOT(MyPlot::copyPlot()), QKeySequence(tr("Ctrl+c")));
}

void MyPlot::copyPlot()
{
}

