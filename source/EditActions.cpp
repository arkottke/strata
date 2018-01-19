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

#include "EditActions.h"

#include <QApplication>
#include <QDebug>

EditActions *EditActions::_instance = nullptr;

EditActions::EditActions( QObject * parent ) : QObject(parent)
{
    // Paste Action
    _pasteAction = new QAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"),this);
    _pasteAction->setShortcut(QKeySequence::Paste);
    connect(_pasteAction, SIGNAL(triggered()), SLOT(paste()));
    
    // Copy Action
    _copyAction = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"),this);
    _copyAction->setShortcut(QKeySequence::Copy);
    connect(_copyAction, SIGNAL(triggered()), SLOT(copy()));

    // Cut Action
    _cutAction = new QAction(QIcon(":/images/edit-cut.svg"), tr("Cu&t"),this);
    _cutAction->setShortcut(QKeySequence::Cut);
    connect(_cutAction, SIGNAL(triggered()), SLOT(cut()));

    // Clear Action
    _clearAction = new QAction(QIcon(":/images/edit-clear.svg"), tr("Clear"),this);
    connect(_clearAction, SIGNAL(triggered()), SLOT(clear()));
}

EditActions * EditActions::instance()
{
    if (_instance == nullptr) {
        _instance = new EditActions;
    }

    return _instance;
}

QAction * EditActions::cutAction()
{
    return _cutAction;
}

QAction * EditActions::copyAction()
{
    return _copyAction;
}

QAction * EditActions::pasteAction()
{
    return _pasteAction;
}

QAction * EditActions::clearAction()
{
    return _clearAction;
}

void EditActions::cut() 
{
    copy();
    clear();
}

void EditActions::copy() 
{
    QMetaObject::invokeMethod(QApplication::focusWidget(), "copy");
}

void EditActions::paste() 
{
    QMetaObject::invokeMethod(QApplication::focusWidget(), "paste");
}

void EditActions::clear() 
{
    QMetaObject::invokeMethod(QApplication::focusWidget(), "clear");
}
