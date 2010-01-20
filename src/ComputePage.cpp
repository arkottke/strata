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
#include "SiteResponseOutput.h"

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
    connect( m_model, SIGNAL(progressRangeChanged(int,int)), m_progressBar, SLOT(setRange(int,int)));
    connect( m_model, SIGNAL(progressChanged(int)), m_progressBar, SLOT(setValue(int)));

    layout->addWidget( m_progressBar, 0, 0);

    // Completion time
    m_etaLineEdit = new QLineEdit;
    m_etaLineEdit->setReadOnly(true);
    m_etaLineEdit->setFixedWidth(80);

    layout->addWidget( new QLabel(tr("ETC:")), 0, 1);
    layout->addWidget( m_etaLineEdit, 0, 2);

    // Buttons
    m_cancelButton = new QPushButton(QIcon(":/images/process-stop.svg"), tr("Cancel"));
    m_cancelButton->setEnabled(false);
    m_cancelButton->setCursor(Qt::ArrowCursor);
    connect(m_cancelButton, SIGNAL(clicked()), m_model, SLOT(stop()));

    //m_computeButton = new QPushButton(QIcon(":/images/system-run.svg"), tr("Compute"));
    m_computeButton = new QPushButton(tr("Compute"));
    m_computeButton->setDefault(true);
    connect(m_computeButton, SIGNAL(clicked()), SLOT(compute()));
    connect(m_model, SIGNAL(finished()), SLOT(finished()));

    layout->addWidget(m_cancelButton, 0, 3);
    layout->addWidget(m_computeButton, 0, 4);
    
    // Text area
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setTabStopWidth(20);
    connect( m_model->output()->textLog(), SIGNAL(textChanged(QString)), m_logView, SLOT(append(QString)));
    connect( m_model->output()->textLog(), SIGNAL(textCleared()), m_logView, SLOT(clear()));

    layout->addWidget( m_logView, 1, 0, 1, 5);

    // Set stretch row and column
    layout->setColumnStretch(0,1);
    layout->setRowStretch(0,1);

    setLayout(layout);
}

void ComputePage::setReadOnly(bool b)
{
    m_computeButton->setDisabled(b);
}

void ComputePage::reset()
{
    m_logView->clear();
    m_etaLineEdit->clear();
    m_progressBar->reset();
}

void ComputePage::compute()
{
    // Prompt to save the file
    emit saveRequested();

    // Disable the compute button
    m_computeButton->setEnabled(false);
    m_cancelButton->setEnabled(true);
    
    // Start the calculation
    m_model->start();
}

void ComputePage::finished()
{
    m_cancelButton->setEnabled(false);

    // compute button is turned off by setReadOnly() called from MainWindow.cpp
    // if the calculation was successful.
    if (!m_model->wasSucessful()) {
        m_computeButton->setEnabled(true);
    }
}

void ComputePage::updateEta(int value)
{
    if ( value == m_progressBar->minimum() )
        m_timer.restart();
    else {
        // Compute the average time per step of the progress bar
        double avgRate = double(m_timer.elapsed()) / double(value - m_progressBar->minimum());

        // The estimated time of completion is computed by multiplying the average rate by the number of remaining increments
        QTime eta = QTime::currentTime().addMSecs(int(avgRate * (m_progressBar->maximum()-value)));

        m_etaLineEdit->setText(eta.toString(Qt::LocalDate));
    }
}
