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

#include "MainWindow.h"

#include "ComputePage.h"
#include "ConfigurePlotDialog.h"
#include "ConfiningStressDialog.h"
#include "EditActions.h"
#include "GeneralPage.h"
#include "MotionPage.h"
#include "HelpDialog.h"
#include "NonlinearPropertyCatalogDialog.h"
#include "OutputCatalog.h"
#include "OutputPage.h"
#include "OutputExportDialog.h"
#include "ResultsPage.h"
#include "SoilProfile.h"
#include "SiteResponseModel.h"
#include "SoilTypePage.h"
#include "SoilTypeCatalog.h"
#include "SoilProfilePage.h"
#include "UpdateDialog.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QPrintDialog>
#include <QStatusBar>
#include <QScrollArea>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QMainWindow * parent) 
    : QMainWindow(parent), m_model(0)
{
    // Initialize the help dialog, but keep it hidden
    m_helpDialog = new HelpDialog(this);
    m_confiningStressDialog = 0;

    m_printer = new QPrinter;
    
    // Create the settings
    m_settings = new QSettings(this);

    // Setup up the mainwindow
    createActions();
    createPages();
    createToolbar();
    createMenus();

    // Set the initial title
    if (QApplication::arguments().size() > 1) {
        open(QApplication::arguments().last());
    } else {
        newModel();
    }   
}

MainWindow::~MainWindow()
{
    delete m_printer;
}

void MainWindow::createActions()
{
    // Read-only
    m_readOnlyAction = new QAction(QIcon(":/images/edit-delete.svg"), tr("Delete results"), this);
    m_readOnlyAction->setShortcut(tr("F2"));
    m_readOnlyAction->setEnabled(false);


    // New
    m_newAction = new QAction(QIcon(":/images/document-new.svg"), tr("&New"),this);
    m_newAction->setShortcut(tr("Ctrl+n"));
    connect(m_newAction, SIGNAL(triggered()), this, SLOT(newModel()));

    // Open
    m_openAction = new QAction(QIcon(":/images/document-open.svg"), tr("&Open..."),this);
    m_openAction->setShortcut(tr("Ctrl+o"));
    connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

    // Save
    m_saveAction = new QAction(QIcon(":/images/document-save.svg"), tr("&Save"),this);
    m_saveAction->setShortcut(tr("Ctrl+s"));
    connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));
    // Save As
    m_saveAsAction = new QAction(QIcon(":/images/document-save-as.svg"), tr("Save &As..."),this);
    connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
    
    // Export the data to text file
    m_exportAction = new QAction(QIcon(":/images/document-export.svg"), tr("&Export..."), this);
    m_exportAction->setEnabled(false);
    connect(m_exportAction, SIGNAL(triggered()), this, SLOT(exportData()));
    
    // Print
    m_printAction = new QAction(QIcon(":/images/document-print.svg"), tr("&Print..."), this);
    m_printAction->setShortcut(tr("Ctrl+p"));
    connect(m_printAction, SIGNAL(triggered()), this, SLOT(print()));
    
    // Print to pdf
    m_printToPdfAction = new QAction(tr("&Print to PDF..."), this);
    connect(m_printToPdfAction, SIGNAL(triggered()), this, SLOT(printToPdf()));

    // Exit
    m_exitAction = new QAction(tr("E&xit"),this);
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    // Non Linear Curve Database
    m_nlCurveDatabaseAction = new QAction(QIcon(":/images/system-file-manager.svg"), tr("Add/Remove Nonlinear Curves"), this);
    connect( m_nlCurveDatabaseAction, SIGNAL(triggered()), this, SLOT(showNonlinearDialog()));

    // Confining Stress Calculator
    m_confiningStressDialogAction = new QAction(QIcon(":/images/accessories-calculator.svg"), tr("Confining Stress Calculator"), this);
    connect( m_confiningStressDialogAction, SIGNAL(triggered()), this, SLOT(showConfiningStressDialog()));

    // Help Action
    m_helpAction = new QAction(QIcon(":/images/help-browser.svg"), tr("&User Manual"), this);
    m_helpAction->setShortcut(tr("F1"));
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(help()));

    // Check for updates
    m_updateAction = new QAction(tr("&Check for updates..."), this);
    connect(m_updateAction, SIGNAL(triggered()), this, SLOT(update()));

    // About action
    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createPages()
{
    m_tabWidget = new QTabWidget;
    connect( m_tabWidget, SIGNAL(currentChanged(int)),
             this, SLOT(tabChanged(int)));

    // Create the pages
    m_pages << new GeneralPage;
    m_tabWidget->addTab(m_pages.last(), tr("General Settings"));

    m_pages << new SoilTypePage;
    connect(m_pages.last(), SIGNAL(linkActivated(QString)),
            m_helpDialog, SLOT(gotoLink(QString)));
    m_tabWidget->addTab(m_pages.last(), tr("Soil Types"));

    m_pages << new SoilProfilePage;
    m_tabWidget->addTab(m_pages.last(), tr("Soil Profile"));

    m_pages <<  new MotionPage;
    m_tabWidget->addTab(m_pages.last(), tr("Motion(s)"));

    m_pages << new OutputPage;
    m_tabWidget->addTab(m_pages.last(), tr("Output Specification"));

    ComputePage* cp = new ComputePage;
    connect(cp, SIGNAL(saveRequested()),
            this, SLOT(save()));
    m_pages << cp;
    m_tabWidget->addTab(m_pages.last(), tr("Compute"));

    m_resultsPage = new ResultsPage;
    m_pages << m_resultsPage;
    m_tabWidget->addTab(m_pages.last(), tr("Results"));

    // Place the tab widget in a scroll area
    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidget(m_tabWidget);
    scrollArea->setWidgetResizable(true);
    
    setCentralWidget(scrollArea);
}

void MainWindow::createMenus()
{
    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_newAction);
    fileMenu->addAction(m_openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exportAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_printAction);
    fileMenu->addAction(m_printToPdfAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu * editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(EditActions::instance()->cutAction());
    editMenu->addAction(EditActions::instance()->copyAction());
    editMenu->addAction(EditActions::instance()->pasteAction());
    editMenu->addAction(EditActions::instance()->clearAction());

    QMenu * toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(m_readOnlyAction);
    toolsMenu->addSeparator();
    toolsMenu->addAction(m_nlCurveDatabaseAction);
    toolsMenu->addAction(m_confiningStressDialogAction);

    QMenu * windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(m_toolBar->toggleViewAction());

    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_helpAction);
    helpMenu->addSeparator();
    // FIXME: Update is currently disabled until hosting service can be figured out.
    //helpMenu->addAction(m_updateAction);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolbar()
{
    m_toolBar = new QToolBar(tr("Main ToolBar"), this);

    m_toolBar->addAction( m_readOnlyAction );

    m_toolBar->addSeparator();
    m_toolBar->addAction(m_newAction);
    m_toolBar->addAction(m_openAction);
    m_toolBar->addAction(m_saveAction);
    m_toolBar->addAction(m_saveAsAction);

    m_toolBar->addSeparator();
    m_toolBar->addAction(m_printAction);
    m_toolBar->addAction(m_exportAction);
    
    m_toolBar->addSeparator();
    m_toolBar->addAction(m_nlCurveDatabaseAction);

    m_toolBar->addSeparator();
    m_toolBar->addAction(m_helpAction );

    addToolBar(m_toolBar);
}

void MainWindow::newModel()
{
    if (!okToClose()) {
        return;
    }

    setModel(new SiteResponseModel);
}

void MainWindow::open(QString fileName)
{
    if (!okToClose()) {
        return;
    }

    if (fileName.isEmpty()) {

        // setup multiple filters
        QString selfilter = tr("strata (*.strata);;strata human readable (*.stratahr)");

        // Prompt for a fileName
        fileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Open strata file..."),
                    (m_model->fileName().isEmpty() ?
                     m_settings->value("projectDirectory",
                                       QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
                                       ).toString()
                                           : m_model->fileName()),
                    "Strata File (*.strata);;strata human readable (*.stratahr)",
                    &selfilter);

        if (!fileName.isEmpty()) {
            // Save the state
            m_settings->setValue("projectDirectory", QFileInfo(fileName).filePath());
        } else {
            return;
        }
    }   

    SiteResponseModel* srm = new SiteResponseModel;
    if (fileName.endsWith(".strata"))
    {
        srm->load(fileName);
    }
    else
    {
        srm->loadReadable(fileName);
    }
    setModel(srm);
}

bool MainWindow::save()
{
    // If no file name is selected run saveAs
    if (m_model->fileName().isEmpty())
        return saveAs();

    // Save the model
    if (m_model->fileName().endsWith(".strata"))
    {
       m_model->save();
    }
    else
    {
       m_model->saveReadable();
    }

    return true;
}

bool MainWindow::saveAs()
{
    // setup multiple filters
    QString selfilter = tr("strata (*.strata);;strata human readable (*.stratahr)");
    // Prompt for a fileName
    QString fileName = QFileDialog::getSaveFileName(
            this,
            tr("Save file as..."),
            (m_model->fileName().isEmpty() ?
             m_settings->value("projectDirectory",
                               QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
                               ).toString()
                                   : m_model->fileName()),
            "Strata File (*.strata);;strata human readable (*.stratahr)",
            &selfilter);

    if (!fileName.isEmpty()) {
        // Make sure that the file name ends with .strata or .stratahr
        if (!fileName.endsWith(".strata", Qt::CaseInsensitive) && !fileName.endsWith(".stratahr", Qt::CaseInsensitive)) {
            fileName.append(".strata");
        }

        // Save the state
        m_settings->setValue("projectDirectory", QFileInfo(fileName).path());
        // Update the document title
        m_model->setFileName(fileName);
        // Save the model if a file was selected
        return save();
    } else {
        return false;
    }
}

void MainWindow::exportData()
{
    m_resultsPage->exportData();
}

void MainWindow::print()
{
    // Create a dialog to select the printer
    QPrintDialog printDialog(m_printer);

    if ( printDialog.exec()) {
        if ( m_tabWidget->currentIndex() == RESULTS_PAGE ) {
            m_resultsPage->print(m_printer);
        } else {
            QTextDocument doc;
            doc.setHtml(m_model->toHtml());
            // Print the report
            doc.print(m_printer);
        }
    }
}

void MainWindow::printToPdf()
{
    // Prompt for a fileName
    QString fileName = QFileDialog::getSaveFileName(
            this,
            tr("Select output file..."),
            m_settings->value("exportDirectory",
                              QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
                              ).toString(),
            "PDF File (*.pdf)");

    if (!fileName.isEmpty()) {
        // Save the state
        m_settings->setValue("exportDirectory", QFileInfo(fileName).filePath());
        m_printer->setOutputFileName(fileName);
        
        if ( m_tabWidget->currentIndex() == RESULTS_PAGE ) {
            m_resultsPage->print(m_printer);
        } else {
            QTextDocument doc;
            doc.setHtml(m_model->toHtml());
            // Print the report
            doc.print(m_printer);
        }
    }
}

void MainWindow::showNonlinearDialog()
{   
    NonlinearPropertyCatalogDialog dialog(m_model->siteProfile()->soilTypeCatalog()->nlCatalog());

    dialog.addAction(EditActions::instance()->cutAction());
    dialog.addAction(EditActions::instance()->copyAction());
    dialog.addAction(EditActions::instance()->pasteAction());

    dialog.exec();

    // Save the modified library
    m_model->siteProfile()->soilTypeCatalog()->nlCatalog()->save();
}

void MainWindow::showConfiningStressDialog()
{
    if (!m_confiningStressDialog) {
        m_confiningStressDialog = new ConfiningStressDialog(this);
    }
    m_confiningStressDialog->show();
    m_confiningStressDialog->raise();
    m_confiningStressDialog->activateWindow();
}

void MainWindow::help()
{
    m_helpDialog->show();
}

void MainWindow::update()
{
    UpdateDialog ud(this);

    ud.exec();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Strata"),
                       tr("<p><b>Strata</b> was written by Albert Kottke working with Professor"
                          " Ellen Rathje at The University of Texas at Austin.</p>"
                          "<p>For comments and suggestions contact Albert at "
                          "<a href=\"mailto:albert.kottke@gmail.com\">albert.kottke@gmail.com</a></p>"
                          "<p>Version: alpha, revision: %1</p>"
#ifdef ADVANCED_OPTIONS
                          "<p>Compiled with advanced options</p>"
#endif
                          ).arg(REVISION));
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (okToClose()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::updateTabs()
{
    const bool isBusy = m_model->isRunning();

    // Enable/Disable all actions and menus
    foreach (QObject* object, children()) {
        if (QAction* action = qobject_cast<QAction*>(object))
            action->setDisabled(isBusy);
    }

    foreach (QObject* object, menuBar()->children()) {
        if (QMenu* menu = qobject_cast<QMenu*>(object))
            menu->setDisabled(isBusy);
    }

    setCursor(isBusy ?
              Qt::WaitCursor : Qt::ArrowCursor);

    // Set all tabs to be enabled or disabled
    for (int i = 0; i < m_tabWidget->count(); ++i)
        m_tabWidget->setTabEnabled(i, !isBusy || i == COMPUTE_PAGE);
}

void MainWindow::setReadOnly(bool readOnly)
{
    // Set all of the pages to be read only
    foreach (AbstractPage* page, m_pages)
        page->setReadOnly(readOnly);

    // Change the icon and label of the button
    m_readOnlyAction->setEnabled(readOnly);

    // readOnly means that the model has results and
    // therefore the Results tab needs to be enabled.
    m_tabWidget->setTabEnabled(RESULTS_PAGE, readOnly);
    if (readOnly) {
        m_tabWidget->setCurrentIndex(RESULTS_PAGE);
        m_resultsPage->setModel(m_model);
    }

    m_exportAction->setEnabled(readOnly);
}

void MainWindow::tabChanged(int tab)
{
    if ( m_settings->value("mainwindow/warnAboutReadOnly", true).toBool() 
        && m_model && m_model->hasResults() && tab != RESULTS_PAGE) {

        // Create a dialog box that reminds the user that they are in a
        // read-only environment.

        QVBoxLayout* layout = new QVBoxLayout;

        QLabel* label = new QLabel(tr(
                "Strata input fields are locked in a read-only state. "
                "To edit the fields you must unlock the input by clicking "
                "on the lock button in the upper left portion of the window. "
                "Unlocking Strata will delete all results."));

        label->setWordWrap(true);
        layout->addWidget(label);

        QCheckBox * checkBox = new QCheckBox(tr("Stop reminding me."));
        layout->addWidget( checkBox );

        QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
        layout->addWidget(buttons);

        QDialog dialog(this);
        connect( buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
        dialog.setLayout(layout);

        if (dialog.exec())
            m_settings->setValue("mainwindow/warnAboutReadOnly", !checkBox->isChecked());
    }
}

void MainWindow::setModel(SiteResponseModel *model)
{
    if (m_model)
        m_model->deleteLater();

    m_model = model;

    // Update all of the pages
    foreach (AbstractPage* page, m_pages)
        page->setModel(m_model);

    updateTabs();
    connect(m_readOnlyAction, SIGNAL(triggered()),
            m_model, SLOT(clearResults()));
    setReadOnly(m_model->hasResults());
    connect(m_model, SIGNAL(hasResultsChanged(bool)),
            this, SLOT(setReadOnly(bool)));

    setWindowModified(false);
    connect(m_model, SIGNAL(modifiedChanged(bool)),
            this, SLOT(setWindowModified(bool)));

    updateWindowTitle(m_model->fileName());
    connect(m_model, SIGNAL(fileNameChanged(QString)),
            this, SLOT(updateWindowTitle(QString)));

    connect(m_model, SIGNAL(started()),
            this, SLOT(updateTabs()));
    connect(m_model, SIGNAL(finished()),
            this, SLOT(updateTabs()));

    // During this process clear() is called on the OutputCatalog which flags
    // the SiteResponseModel as being modified. We should reset this flag.
    // m_model->setModified(false);
    // FIXME
}

void MainWindow::updateWindowTitle(const QString &fileName)
{
    setWindowTitle(tr("%1[*] - %2").arg(fileName.isEmpty() ? "untitled.strata" : fileName).arg("Strata"));
}

bool MainWindow::okToClose()
{
    // Prompt that the input has been modified
    if (m_model && m_model->modified()) {
        QMessageBox msgBox(this);
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Save:
            // Save was clicked
            return save();
        case QMessageBox::Discard:
            // Don't Save was clicked
            return true;
        case QMessageBox::Cancel:
        default:
            // Cancel was clicked
            return false;
        }
    }
    return true;
}
