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

#include "ComputePage.h"

#include <QGridLayout>
#include <QLabel>
#include <QDebug>

ComputePage::ComputePage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f), m_model(model)
{
    QGridLayout * layout = new QGridLayout;

    // Progress bar
    m_progressBar = new QProgressBar;
    connect( m_progressBar, SIGNAL(valueChanged(int)), this, SLOT(updateEta(int)));

    layout->addWidget( m_progressBar, 0, 0);

    // Completion time
    m_etaLineEdit = new QLineEdit;
    m_etaLineEdit->setReadOnly(true);
    m_etaLineEdit->setFixedWidth(80);

    layout->addWidget( new QLabel(tr("ETA:")), 0, 1);
    layout->addWidget( m_etaLineEdit, 0, 2);

    // Buttons
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_cancelButton->setEnabled(false);
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    m_computeButton = new QPushButton(tr("Compute"));
    m_computeButton->setDefault(true);
    connect(m_computeButton, SIGNAL(clicked()), this, SLOT(started()));

    layout->addWidget(m_cancelButton, 0, 3);
    layout->addWidget(m_computeButton, 0, 4);
    
    // Text area
    layout->addWidget( &(m_model->textLog()), 1, 0, 1, 5);

    // Set stretch row and column
    layout->setColumnStretch(0,1);
    layout->setRowStretch(0,1);

    setLayout(layout);
}

void ComputePage::setModel( SiteResponseModel * model )
{
    m_model = model;
}

void ComputePage::reset()
{
    m_model->textLog().clear();
    m_etaLineEdit->clear();
    m_progressBar->reset();
}

void ComputePage::started()
{
    emit computing();
    // Set that is okay to continue
    m_model->setOkToContinue(true);
    // Disable the compute button
    m_computeButton->setEnabled(false);
    m_cancelButton->setEnabled(true);

    emit busy(true);

    // Start the computation
    m_model->compute(m_progressBar);

    // Update the page reflecting that the computation is finished.
    stopped();
}

void ComputePage::stopped()
{
    // Disable the compute button
    m_computeButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    
    emit busy(false);
    emit finished();
}

void ComputePage::cancel() 
{
    m_model->setOkToContinue(false);

    // Update the page reflecting that the computation has stopped
    stopped();
}

void ComputePage::updateEta(int value)
{
    if ( value == m_progressBar->minimum() )
        m_timer.restart();
    else {
        double ratio = (m_progressBar->maximum() - m_progressBar->minimum()) /
            ( value - m_progressBar->minimum());
    
        QTime eta = QTime::currentTime().addMSecs( int(m_timer.elapsed() * ratio) );

        m_etaLineEdit->setText(eta.toString("h:mm:ss ap"));
    }
}
