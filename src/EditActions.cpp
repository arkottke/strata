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

#include "EditActions.h"
#include <QApplication>

EditActions * EditActions::m_instance = 0;

EditActions::EditActions( QObject * parent ) : QObject(parent)
{
    // Paste Action
    m_pasteAction = new QAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"),this);
    m_pasteAction->setShortcut(tr("Ctrl+v"));
    connect(m_pasteAction, SIGNAL(triggered()), SLOT(paste()));
    
    // Copy Action
    m_copyAction = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"),this);
    m_copyAction->setShortcut(tr("Ctrl+c"));
    connect(m_copyAction, SIGNAL(triggered()), SLOT(copy()));

    // Cut Action
    m_cutAction = new QAction(QIcon(":/images/edit-cut.svg"), tr("Cu&t"),this);
    m_cutAction->setShortcut(tr("Ctrl+x"));
    connect(m_cutAction, SIGNAL(triggered()), SLOT(cut()));

    // Clear Action
    m_clearAction = new QAction(QIcon(":/images/edit-clear.svg"), tr("Clear"),this);
    connect(m_clearAction, SIGNAL(triggered()), SLOT(clear()));
}

EditActions * EditActions::instance()
{
    if ( m_instance == 0 ) {
        m_instance = new EditActions;
    }

    return m_instance;
}

QAction * EditActions::cutAction()
{
    return m_cutAction;
}

QAction * EditActions::copyAction()
{
    return m_copyAction;
}

QAction * EditActions::pasteAction()
{
    return m_pasteAction;
}

QAction * EditActions::clearAction()
{
    return m_clearAction;
}

void EditActions::cut() 
{
    copy();
    clear();
}

void EditActions::copy() 
{
    QMetaObject::invokeMethod( QApplication::focusWidget(), "copy" );
}

void EditActions::paste() 
{
    QMetaObject::invokeMethod( QApplication::focusWidget(), "paste" );
}

void EditActions::clear() 
{
    QMetaObject::invokeMethod( QApplication::focusWidget(), "clear" );
}
