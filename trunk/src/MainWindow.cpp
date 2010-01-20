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
#include "UpdateDialog.h"

#include "NonlinearPropertyLibraryDialog.h"

#include "EditActions.h"

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
	: QMainWindow(parent)
{
    m_model = new SiteResponseModel;

    // Initialize the help dialog, but keep it hidden
    m_helpDialog = new HelpDialog(this);
    m_confiningStressDialog = 0;

    m_printer = new QPrinter;
    
    // Create the settings
    m_settings = new QSettings;

    // Setup up the mainwindow
    createActions();
    createPage();
    createToolbar();
    createMenus();

    // Reset the properties to the default values
    reset();
    
    
    // Set the initial title
    if ( QApplication::arguments().size() > 1 ) {
        open( QApplication::arguments().last() );
    } else {
        setCurrentFile("");
    }
    
    connect( m_model, SIGNAL(modifiedChanged(bool)), SLOT(setWindowModified(bool)));
}
        
void MainWindow::createActions()
{
    // Read-only
    m_readOnlyAction = new QAction(QIcon(":/images/edit-delete.svg"), tr("Delete results"), this);
    m_readOnlyAction->setShortcut(tr("F2"));
    m_readOnlyAction->setEnabled(false);
    connect(m_readOnlyAction, SIGNAL(triggered()), this, SLOT(setReadOnly()));

    // New
    m_resetAction = new QAction(QIcon(":/images/document-new.svg"), tr("&New"),this);
    m_resetAction->setShortcut(tr("Ctrl+n"));
    connect(m_resetAction, SIGNAL(triggered()), this, SLOT(reset()));

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

void MainWindow::createPage()
{
    m_tabWidget = new QTabWidget;
    connect( m_tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    // Create the pages
    m_generalPage = new GeneralPage(m_model);
    m_soilTypePage = new SoilTypePage(m_model);
    m_soilProfilePage = new SoilProfilePage(m_model);
    m_motionPage = new MotionPage(m_model);
    m_outputPage = new OutputPage(m_model);
    m_computePage = new ComputePage(m_model);
    m_resultsPage = new ResultsPage(m_model->output());

    m_tabWidget->addTab( m_generalPage, tr("General Settings"));
    m_tabWidget->addTab( m_soilTypePage, tr("Soil Types"));
    m_tabWidget->addTab( m_soilProfilePage, tr("Soil Profile"));
    m_tabWidget->addTab( m_motionPage, tr("Motion(s)"));
    m_tabWidget->addTab( m_outputPage, tr("Output Specification"));
    m_tabWidget->addTab( m_computePage, tr("Compute"));
    m_tabWidget->addTab( m_resultsPage, tr("Results"));

    m_tabWidget->setTabEnabled( RESULTS_PAGE, false);

    // Place the tab widget in a scroll area
    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidget(m_tabWidget);
    scrollArea->setWidgetResizable(true);
    
    setCentralWidget(scrollArea);


    // Connections
    connect( m_generalPage, SIGNAL(isSoilVariedChanged(bool)), m_soilTypePage, SLOT(setIsVaried(bool)));
    connect( m_generalPage, SIGNAL(isVelocityVariedChanged(bool)), m_soilProfilePage, SLOT(setIsVaried(bool)));
    connect( m_generalPage, SIGNAL(methodChanged(int)), m_motionPage, SLOT(setMethod(int)));
    connect( m_generalPage, SIGNAL(methodChanged(int)), m_outputPage, SLOT(setMethod(int)));
    connect( m_soilTypePage, SIGNAL(linkActivated(QString)), m_helpDialog, SLOT(gotoLink(QString)));
    connect( m_soilTypePage, SIGNAL(soilTypesChanged()), m_outputPage, SLOT(refresh()));

    connect( m_computePage, SIGNAL(saveRequested()), SLOT(save()));
    connect( m_model, SIGNAL(isWorkingChanged(bool)), SLOT(setIsWorking(bool)));
}

void MainWindow::createMenus()
{
    QMenu * fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_resetAction);
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
    helpMenu->addAction(m_updateAction);
    helpMenu->addAction(m_aboutAction);
}
    
void MainWindow::createToolbar()
{
    m_toolBar = new QToolBar(tr("Main ToolBar"), this);

    m_toolBar->addAction( m_readOnlyAction );

    m_toolBar->addSeparator();
    m_toolBar->addAction( m_resetAction );
    m_toolBar->addAction( m_openAction );
    m_toolBar->addAction( m_saveAction );
    m_toolBar->addAction( m_saveAsAction );

    m_toolBar->addSeparator();
    m_toolBar->addAction( m_printAction );
    m_toolBar->addAction( m_exportAction );
    
    m_toolBar->addSeparator();
    m_toolBar->addAction( m_nlCurveDatabaseAction );

    m_toolBar->addSeparator();
    m_toolBar->addAction( m_helpAction );

    addToolBar(m_toolBar);
}

void MainWindow::setCurrentFile(const QString & fileName)
{
    m_fileName = fileName;
    setWindowTitle(tr("%1 [*] - %2").arg(m_fileName.isEmpty() ? "untitled.strata" : m_fileName ).arg(tr("Strata")));
    setWindowModified(false);
}

void MainWindow::reset()
{
    // Set the initial title
    setCurrentFile("");

    m_model->reset();

    // Reload all of the values from the model
    m_generalPage->load();
    m_soilTypePage->load();
    m_soilProfilePage->load();
    m_motionPage->load();
    m_outputPage->load();
    // FIXME
    //m_computePage->load();
    //m_resultsPage->load();
}

void MainWindow::open( const QString & fileName )
{
    if (!okToClose()) {
        return;
    }

    if (fileName.isEmpty()) {
        // Prompt for a fileName
        QString _fileName = QFileDialog::getOpenFileName(
                this,
                tr("Select a Strata file..."),
                m_settings->value("projectDirectory",
                                  QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
                                  ).toString(),
                "Strata File (*.strata)");

        if (!_fileName.isEmpty()) {
            m_fileName = _fileName;
            // Save the state
            m_settings->setValue( "projectDirectory", QFileInfo(_fileName).filePath());
        } else {
            return;
        }
    } else {
        m_fileName = fileName;
    }
    
    if (!m_fileName.isEmpty()) {
        setCurrentFile(m_fileName);

        m_model->load(m_fileName);
        
        m_generalPage->load();
        m_soilTypePage->load();
        m_soilProfilePage->load();
        m_motionPage->load();
        m_outputPage->load();

        if (m_model->output()->hasResults()) {
            setReadOnly(true);
            // Refresh the data on the output tab
            m_resultsPage->refreshWidget();
            // Switch to the output tab
            m_tabWidget->setCurrentIndex(RESULTS_PAGE);
        } else {
            setReadOnly(false);
        }
    
        setWindowModified(false);
    }
}

bool MainWindow::save()
{
    // If no file name is selected run saveAs
    if (m_fileName.isEmpty())
        return saveAs();
    
    // Save the model
    m_model->save(m_fileName);

    // Reset the modified flag
    setWindowModified(false);

    return true;
}

bool MainWindow::saveAs()
{
    // Prompt for a fileName
    QString fileName = QFileDialog::getSaveFileName(
            this,
            tr("Select a Strata file..."),
            m_settings->value("projectDirectory",
                              QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)
                              ).toString(),
            "Strata File (*.strata)");

    if (!fileName.isEmpty()) {
        // Save the state
        m_settings->setValue("projectDirectory", QFileInfo(fileName).filePath());
        // Update the document title
        setCurrentFile(fileName);
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
    NonlinearPropertyLibraryDialog dialog(m_model->nlPropertyLibrary(), this);

    dialog.addAction( EditActions::instance()->cutAction() );
    dialog.addAction( EditActions::instance()->copyAction() );
    dialog.addAction( EditActions::instance()->pasteAction() );

    dialog.exec();

    // Save the modified library
    m_model->nlPropertyLibrary()->save();
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
    qDebug() << "Update!";
    UpdateDialog ud;

    ud.exec();
    //UpdateDialog * ud = new UpdateDialog;
    //ud->start();

}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Strata"),
             QString(tr("<p><b>Strata</b> was written by Albert Kottke working with Professor"
                 " Ellen Rathje at The University of Texas at Austin.</p>"
                 "<p>For comments and suggestions contact Albert at "
                 "<a href=\"mailto:albert@mail.utexas.edu\">albert@mail.utexas.edu</a></p>"
                 "<p>Version: alpha, revision: %1</p>")).arg(REVISION));
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (okToClose()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::okToClose()
{
    // Prompt that the input has been modified
    if (m_model->modified()) {
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
                // Cancel was clicked
                return false;
            default:
                // should never be reached
                break;
        }
    }
    
    return true;
}

void MainWindow::setIsWorking(bool isWorking)
{
    if (isWorking) {
        setCursor(Qt::WaitCursor);
    } else {
        setCursor(Qt::ArrowCursor);

        if (m_model->wasSucessful()) {
            setReadOnly(true);
            // Refresh the data on the output tab
            m_resultsPage->refreshWidget();
            // Switch to the output tab
            m_tabWidget->setCurrentIndex(RESULTS_PAGE);
        }
    }

    // Enable/Disable all actions and menus
    foreach (QObject * object, children()) {
        QAction * action = qobject_cast<QAction*>(object);
        if (action) {
            action->setDisabled(isWorking);
        }

        QMenu * menu = qobject_cast<QMenu*>(object);

        if (menu) {
            menu->setDisabled(isWorking);
        }
    }
    
    // Enable/Disable the input tabs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (i != COMPUTE_PAGE && i != RESULTS_PAGE ) {
            m_tabWidget->setTabEnabled( i, !isWorking);
        }
    }
}
        
void MainWindow::setReadOnly(bool readOnly)
{
    if (!readOnly && !okToClose()) {
        return;
    }

    m_readOnly = readOnly;
    // Change the icon and label of the button
    m_readOnlyAction->setEnabled(readOnly);

    // Disable the widgets on the pages
    m_generalPage->setReadOnly(readOnly);
    m_soilTypePage->setReadOnly(readOnly);
    m_soilProfilePage->setReadOnly(readOnly);
    m_motionPage->setReadOnly(readOnly);
    m_outputPage->setReadOnly(readOnly);
    m_computePage->setReadOnly(readOnly);

    // Move away from final tab
    if ( !readOnly && m_tabWidget->currentIndex() == RESULTS_PAGE ) {
        m_tabWidget->setCurrentIndex(0);
    }

    // Enable the output page and switch to it
    m_tabWidget->setTabEnabled( RESULTS_PAGE, readOnly);
    m_exportAction->setEnabled(readOnly);

    if ( !readOnly ) {
        // Remove the results
        m_model->output()->clear();
    }
}

void MainWindow::tabChanged(int tab)
{
    if ( m_settings->value("mainwindow/warnAboutReadOnly", true).toBool() 
            && m_readOnly && m_model->output()->hasResults() && tab != RESULTS_PAGE  ) {

        // Create a dialog box that reminds the user that they are in a
        // read-only environment.

        QVBoxLayout * layout = new QVBoxLayout;
    
        layout->addWidget( new QLabel(tr(
                        "Strata input fields are locked in a read-only state. "
                        "To edit the fields\nyou must unlock the input by clicking "
                        "on the lock button in the upper\nleft portion of the window. "
                        "Unlocking Strata will delete all results.")));
    
        QCheckBox * checkBox = new QCheckBox(tr("Stop reminding me."));
        layout->addWidget( checkBox );

        QDialogButtonBox * buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
        layout->addWidget(buttons);

        QDialog dialog(this);
        connect( buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
        dialog.setLayout(layout);

        if (dialog.exec()) {
            m_settings->setValue("mainwindow/warnAboutReadOnly", !checkBox->isChecked() );
        }
    }
}
