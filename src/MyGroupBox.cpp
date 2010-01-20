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

#include "MyGroupBox.h"
#include <QDebug>

MyGroupBox::MyGroupBox( const QString & title, QWidget * parent)
    : QGroupBox( title, parent )
{
    // Box is checkable
    setCheckable(true);

    connect( this, SIGNAL(clicked(bool)), this, SIGNAL(wasModified()));
    connect( this, SIGNAL(toggled(bool)), this, SLOT(setChildrenEnabled(bool)));
}

void MyGroupBox::setChildrenEnabled(bool b)
{
    // Create a list of the children
    QList<QWidget *> children = findChildren<QWidget *>();

    // Enable/disable each of those
    foreach (QWidget * child, children) {
        // Only enable immediate children and don't enable group boxes
        //if ( child->parent() != this || child->metaObject()->className() != "QGroupBox" ) {
        if ( child->parent() != this )
            continue;

        child->setEnabled(b);
    }
}

