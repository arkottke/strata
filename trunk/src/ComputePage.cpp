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

#include "SiteResponseModel.h"
#include "OutputCatalog.h"
#include "TextLog.h"

#include <QGridLayout>
#include <QLabel>
#include <QDebug>

ComputePage::ComputePage(QWidget * parent, Qt::WindowFlags f )
    : AbstractPage(parent, f), m_model(0)
{
    QGridLayout * layout = new QGridLayout;

    // Progress bar
    m_progressBar = new QProgressBar;
    connect( m_progressBar, SIGNAL(valueChanged(int)),
             this, SLOT(updateEta(int)));
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

    layout->addWidget(m_cancelButton, 0, 3);

    //m_computeButton = new QPushButton(QIcon(":/images/system-run.svg"), tr("Compute"));
    m_computeButton = new QPushButton(tr("Compute"));
    m_computeButton->setDefault(true);
    connect(m_computeButton, SIGNAL(clicked()),
            this, SLOT(compute()));

    layout->addWidget(m_computeButton, 0, 4);
    
    // Text area
    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setTabStopWidth(20);

    layout->addWidget( m_logView, 1, 0, 1, 5);

    // Set stretch row and column
    layout->setColumnStretch(0,1);
    layout->setRowStretch(0,1);

    setLayout(layout);
}

void ComputePage::setModel(SiteResponseModel *model)
{
    m_model = model;

    connect(model, SIGNAL(progressRangeChanged(int,int)),
            m_progressBar, SLOT(setRange(int,int)));

    connect(model, SIGNAL(progressChanged(int)),
            m_progressBar, SLOT(setValue(int)));

    connect(m_cancelButton, SIGNAL(clicked()),
            model, SLOT(stop()));

    connect(this, SIGNAL(startCalculation()),
            model, SLOT(start()));
    connect(model, SIGNAL(finished()),
            this, SLOT(reset()));

    m_logView->clear();
    foreach (const QString &line, model->outputCatalog()->log()->text())
        m_logView->append(line);

    connect( model->outputCatalog()->log(), SIGNAL(textChanged(QString)),
             m_logView, SLOT(append(QString)));
    connect( model->outputCatalog()->log(), SIGNAL(textCleared()),
             m_logView, SLOT(clear()));
}

void ComputePage::setReadOnly(bool b)
{
    m_computeButton->setDisabled(b || m_model->hasResults());
    // Only enabled by the compute button
    m_cancelButton->setDisabled(true);

    m_cancelButton->setCursor(Qt::ArrowCursor);
}

void ComputePage::compute()
{
    emit saveRequested();

    m_logView->clear();
    // Disable the compute button
    m_computeButton->setEnabled(false);
    m_cancelButton->setEnabled(true);

    m_cancelButton->setCursor(Qt::BusyCursor);
    
    m_model->start();
}

void ComputePage::reset()
{
    setReadOnly(false);
}

void ComputePage::updateEta(int value)
{
    if (value == m_progressBar->minimum()) {
        m_timer.restart();
    } else {
        // Compute the average time per step of the progress bar
        double avgRate = double(m_timer.elapsed()) / double(value - m_progressBar->minimum());

        // The estimated time of completion is computed by multiplying the average rate by the number of remaining increments
        QTime eta = QTime::currentTime().addMSecs(int(avgRate * (m_progressBar->maximum()-value)));

        m_etaLineEdit->setText(eta.toString(Qt::LocalDate));
    }
}
