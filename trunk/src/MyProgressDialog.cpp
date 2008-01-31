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

#include "MyProgressDialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

MyProgressDialog::MyProgressDialog( const QString & labelText, QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    layout->addWidget( new QLabel(labelText), 0, 0, 1, 2);

    m_progressBar = new QProgressBar;
    layout->addWidget( m_progressBar, 1, 0, 1, 2 );

    QPushButton * cancelButton = new QPushButton(tr("Cancel"));
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    layout->addWidget( cancelButton, 2, 1);

    setLayout(layout);

    m_okToContinue = true;
}

const bool * MyProgressDialog::okToContinue() const
{
    return &m_okToContinue;
}

QProgressBar * MyProgressDialog::progressBar() const
{
    return m_progressBar;
}
    
void MyProgressDialog::cancel()
{
    m_okToContinue = false;
    emit canceled();
}
