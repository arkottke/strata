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

#include "InputWidget.h"
#include <QFileDialog>
#include <QTabWidget>
#include <QTextStream>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QDebug>
        
InputWidget::InputWidget( QWidget * parent)
    : QScrollArea(parent)
{
    m_model = new SiteResponseModel;
    // Create the page
    createPage();
    
    connect( m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(refreshTab(int)));
}

SiteResponseModel * InputWidget::model() const
{
    return m_model;
}

SiteResponseOutput * InputWidget::outputModel()
{
    return m_model->output();
}

void InputWidget::disableTabs(bool disable)
{
    // Change all but the last tab which is the computation tab
    for ( int i = 0; i < m_tabWidget->count() - 1; ++i)
        m_tabWidget->setTabEnabled( i, !disable);
}

void InputWidget::refreshTab(int /*tab*/)
{
    QMetaObject::invokeMethod( m_tabWidget->currentWidget(), "refresh");
}

void InputWidget::reset()
{
    m_model->reset();

    // For each tab in the widget load from the new model
    for(int i=0; i < m_tabWidget->count(); ++i)
        QMetaObject::invokeMethod( m_tabWidget->widget(i), "load");
}

void InputWidget::open(const QString fileName)
{
    // Open the file
    m_model->load(fileName);

    // For each tab in the widget load from the new model
    for(int i=0; i < m_tabWidget->count(); ++i)
        QMetaObject::invokeMethod( m_tabWidget->widget(i), "load");

    m_computePage->reset();
}
    
void InputWidget::save(const QString fileName)
{
    // For each tab in the widget save the data to the model
    for(int i=0; i < m_tabWidget->count(); ++i)
        QMetaObject::invokeMethod( m_tabWidget->widget(i), "save");

    // Save the model
    m_model->save(fileName);
}

void InputWidget::exportData()
{
    // Prompt for a fileName
    QString fileName = 
        QFileDialog::getSaveFileName( this, tr("Save report as..."), QDir::currentPath(), tr("HTML File (*.html)"));

    if ( !fileName.isEmpty() ) {
        // Make sure the extension is on the end of the file name
        if ( !fileName.endsWith(".html") )
            fileName += ".html";

        QFile file(fileName);

        // Try opening the file for writing
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            qCritical() << QString("Unable to open '%1' for writing!").arg(fileName);
            return;
        }

        QTextStream textStream(&file);
        textStream.setCodec("UTF-8");

        // For each tab in the widget save the data to the model
        for(int i=0; i < m_tabWidget->count(); ++i)
            QMetaObject::invokeMethod( m_tabWidget->widget(i), "save");

        textStream << m_model->toHtml();

        // Close the file
        file.close();
    }
}

void InputWidget::print(QPrinter * printer)
{
    // For each tab in the widget save the data to the model
    for(int i=0; i < m_tabWidget->count(); ++i)
        QMetaObject::invokeMethod( m_tabWidget->widget(i), "save");

    QTextDocument doc;
    doc.setHtml(m_model->toHtml());
    // Print the report
    doc.print(printer);
}

void InputWidget::createPage() 
{
    m_tabWidget = new QTabWidget;

    // Create the pages
    m_generalPage = new GeneralPage(m_model);
    m_soilTypePage = new SoilTypePage(m_model);
    m_soilProfilePage = new SoilProfilePage(m_model);
    m_motionPage = new MotionPage(m_model);
    m_outputPage = new OutputPage(m_model);
    m_computePage = new ComputePage(m_model);

    m_tabWidget->addTab( m_generalPage, tr("General Settings"));
    m_tabWidget->addTab( m_soilTypePage, tr("Soil Types"));
    m_tabWidget->addTab( m_soilProfilePage, tr("Soil Profile"));
    m_tabWidget->addTab( m_motionPage, tr("Motion(s)"));
    m_tabWidget->addTab( m_outputPage, tr("Output Specification"));
    m_tabWidget->addTab( m_computePage, tr("Compute"));
    
    // Set the input widget inside a scroll area
    setWidget(m_tabWidget);    
    setWidgetResizable(true);

    // Connections
    connect( m_generalPage, SIGNAL(isSoilVariedChanged(bool)), m_soilTypePage, SLOT(setIsVaried(bool)));
    connect( m_generalPage, SIGNAL(isVelocityVariedChanged(bool)), m_soilProfilePage, SLOT(setIsVaried(bool)));
    connect( m_generalPage, SIGNAL(methodChanged(int)), m_motionPage, SLOT(setMethod(int)));
    connect( m_generalPage, SIGNAL(methodChanged(int)), m_outputPage, SLOT(setMethod(int)));
    connect( m_generalPage, SIGNAL(hasChanged()), this, SIGNAL(modified()));
    connect( m_soilTypePage, SIGNAL(hasChanged()), this, SIGNAL(modified()));
    connect( m_soilTypePage, SIGNAL(linkActivated(QString)), this, SIGNAL(linkActivated(QString)));
    connect( m_soilProfilePage, SIGNAL(hasChanged()), this, SIGNAL(modified()));
    connect( m_motionPage, SIGNAL(hasChanged()), this, SIGNAL(modified()));
    connect( m_outputPage, SIGNAL(hasChanged()), this, SIGNAL(modified()));
    connect( m_computePage, SIGNAL(computing()), this, SIGNAL(save()));
    connect( m_computePage, SIGNAL(busy(bool)), this, SLOT(disableTabs(bool)));
    connect( m_computePage, SIGNAL(finished()), this, SIGNAL(finishedComputing()));
}
