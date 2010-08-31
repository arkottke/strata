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

#include "DepthComboBox.h"
#include <QLineEdit>

#include <QDebug>

DepthComboBox::DepthComboBox( QWidget * parent)
    : QComboBox(parent)
{
    // Add the bedrock and specify depth items
    addItem(tr("Specify depth..."));
    addItem(tr("Bedrock"));

    // Set the insert policy to that "specify depth..." is over-written
    setInsertPolicy(InsertAtCurrent);
    
    setValidator(new QDoubleValidator(this));
    setCurrentIndex(-1);

    connect( this, SIGNAL(currentIndexChanged(int)), SLOT(updateEditable(int)));
    connect( this, SIGNAL(editTextChanged(QString)), SLOT(toDouble(QString)));
}

double DepthComboBox::depth() const
{
    if (currentIndex() == 0 )
        // Depth specified
        return currentText().toDouble();
    else if (currentIndex() == 1 )
        // Bedrock selected
        return -1;

    return -2;
}

void DepthComboBox::setDepth(double location)
{
    if (location < 0)
        // Bedrock layer selected
        setCurrentIndex(1);
    else {
        // Depth specified
        setCurrentIndex(0);
        setItemText(0, QString::number(location));
    }
}

void DepthComboBox::updateEditable(int index)
{
    if ( index == 0 ) {
        // Allow the combo box to be edited
        setEditable(true);
        // Select all of the text
        lineEdit()->selectAll();
        setCursor(Qt::IBeamCursor);
    } else {
        // Depth is the bedrock depth and is uneditable
        setEditable(false);
        unsetCursor();
    }
}

void DepthComboBox::toDouble(const QString & string )
{
    bool ok = false;
    double d = string.toDouble(&ok);

    if (ok) {
        emit depthChanged(d);
    } else {
        emit depthChanged(-1);
    }
}
