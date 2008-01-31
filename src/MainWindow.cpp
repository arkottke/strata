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

#include "NonLinearPropertyLibraryDialog.h"

#include <QDebug>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollArea>
#include <QPrintDialog>
#include <QStatusBar>

MainWindow::MainWindow(QMainWindow * parent) 
	: QMainWindow(parent)
{
    // Initialize the help dialog, but keep it hidden
    m_helpDialog = new HelpDialog;

    m_printer = new QPrinter;

    // Setup up the mainwindow
    createActions();
    createPage();
    createMenus();

    // Set the window icon
    setWindowIcon(QIcon(":/images/application-icon.svg"));

    // Create the settings
    m_settings = new QSettings;
    
    // Set the initial title
    if ( QApplication::arguments().size() > 1 )
        open( QApplication::arguments().last() );
    else
        setCurrentFile("");
}

void MainWindow::createActions()
{
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
    m_exportAction = new QAction(tr("&Export..."), this);
    connect(m_exportAction, SIGNAL(triggered()), this, SLOT(exportData()));
    
    // Print
    m_printAction = new QAction(QIcon(":/images/document-print.svg"), tr("&Print..."), this);
    m_printAction->setShortcut(tr("Ctrl+p"));
    connect(m_printAction, SIGNAL(triggered()), this, SLOT(print()));
    
    // Print to pdf
    m_printToPdfAction = new QAction(tr("&Print to PDF..."), this);
    connect(m_printToPdfAction, SIGNAL(triggered()), this, SLOT(printToPdf()));

    // Exit
    m_exitAction = new QAction(QIcon(":/images/process-stop.svg"), tr("E&xit"),this);
    connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));
    
    // Paste Action
    m_pasteAction = new QAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"),this);
    m_pasteAction->setShortcut(tr("Ctrl+v"));
    connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
    
    // Copy Action
    m_copyAction = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"),this);
    m_copyAction->setShortcut(tr("Ctrl+c"));
    connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    // Non Linear Curve Database
    m_nlCurveDatabaseAction = new QAction(QIcon(":/images/system-file-manager.svg"), tr("Add/Remove Non-Linear Curves"), this);
    connect( m_nlCurveDatabaseAction, SIGNAL(triggered()), this, SLOT(showNonLinearDialog()));

    // Switch view
    m_switchViewAction = new QAction(tr("Output View"), this);
    m_switchViewAction->setShortcut(tr("F2"));
    connect(m_switchViewAction, SIGNAL(triggered()), this, SLOT(switchView()));

    // Help Action
    m_helpAction = new QAction(QIcon(":/images/help-browser.svg"), tr("&User Manual"), this);
    m_helpAction->setShortcut(tr("F1"));
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(help()));

    // About action
    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createPage()
{
    // Create the input and output widgets
    m_inputWidget = new InputWidget;
    m_outputWidget = new OutputWidget( m_inputWidget->model()->output() );
    
    // The input and output widgets are stacked on to of each other.
    m_stackedWidget = new QStackedWidget;

    connect( m_stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(updateMenus(int)));

    m_stackedWidget->addWidget( m_inputWidget );
    m_stackedWidget->addWidget( m_outputWidget );

    // By default the first input widget is selected
    m_stackedWidget->setCurrentIndex(0);

    // Set the layout of the main window
    setCentralWidget(m_stackedWidget);

    // Connections
    connect( m_inputWidget, SIGNAL(linkActivated(QString)), m_helpDialog, SLOT(gotoLink(QString)));
    connect( m_inputWidget, SIGNAL(modified()), this, SLOT(setModified()));
    connect( m_inputWidget, SIGNAL(save()), this, SLOT(save()));
    connect( m_inputWidget, SIGNAL(finishedComputing()), this, SLOT(switchView()));
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
    editMenu->addAction(m_copyAction);
    editMenu->addAction(m_pasteAction);

    QMenu * toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(m_nlCurveDatabaseAction);

    QMenu * windowMenu = menuBar()->addMenu(tr("&Window"));
    windowMenu->addAction(m_switchViewAction);

    QMenu * helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_helpAction);
    helpMenu->addSeparator();
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setCurrentFile(const QString & fileName)
{
    setModified(false);

    QString shownName;
    if (fileName.isEmpty()) {
        if ( m_stackedWidget->currentIndex() == 0 )
        shownName = "untitled.stri";
        else if ( m_stackedWidget->currentIndex() == 1 )
            shownName = "untitled.stro";
    } else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1 [*] - %2").arg(shownName).arg(tr("Strata")));
}

void MainWindow::reset()
{
    // Resets the values of the model
    if ( m_stackedWidget->currentIndex() == InputWindow && okToClose()) {
        m_inputWidget->reset();
        setModified(false);
        // Set the initial title
        setCurrentFile("");
    }
}

void MainWindow::open( const QString & fileName )
{
    if ( fileName.isEmpty() ) {
        // File filter
        QString filter = (m_stackedWidget->currentIndex() == InputWindow ) ?
            "Strata Input File (*.stri);;Strata Output File (*.stro)" :
            "Strata Output File (*.stro);;Strata Input File (*.stri)";

        // Prompt for a fileName
        QFileDialog dialog(this);
        dialog.setFilter(filter);
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setViewMode(QFileDialog::Detail);

        if (m_settings->contains("mainwindow-file-dialog"))
            dialog.restoreState( m_settings->value("mainwindow-file-dialog").toByteArray() );

        if (dialog.exec()) {
            m_fileName = dialog.selectedFiles().at(0);
            // Save the state
            m_settings->setValue( "mainwindow-file-dialog", dialog.saveState() );
        }
    } else
        m_fileName = fileName;
    
    if (!m_fileName.isEmpty()) {
        if (m_fileName.endsWith(".stri")) {
            // Select the input (0) interface if the file ends with .stri 
            m_stackedWidget->setCurrentIndex(0);
        } else if (m_fileName.endsWith(".stro")) {
            // Select the input (1) interface if the file ends with .stro
            m_stackedWidget->setCurrentIndex(1);

            // If the currently opened SiteResponseOutput pointer does not
            // match the pointer in the output file it is from an opened file
            // and should be deleted.
            if ( m_inputWidget->outputModel() != m_outputWidget->model() )
                delete m_outputWidget->model();
        } else
            return;

        setCurrentFile(m_fileName);
        
        QMetaObject::invokeMethod( m_stackedWidget->currentWidget(), "open", Q_ARG( QString, m_fileName));
    }
    setModified(false);
}

bool MainWindow::save()
{
    // If no file name is selected run saveAs
    if (m_fileName.isEmpty())
        return saveAs();

    QMetaObject::invokeMethod( m_stackedWidget->currentWidget(), "save", Q_ARG( QString, m_fileName));

    // Set the filename of the output model
    if ( m_stackedWidget->currentIndex() == 0 ) {
        QString outFileName(m_fileName);
        outFileName.replace("stri", "stro");
        m_inputWidget->outputModel()->setFileName( outFileName );
    }
   
    // Reset the modified flag
    setModified(false);

    return true;
}

bool MainWindow::saveAs()
{
    QString filter = (m_stackedWidget->currentIndex() == InputWindow ) ?
        "Strata Input File (*.stri)" :
        "Strata Output File (*.stro)";

    // Prompt for a fileName
    QFileDialog dialog(this);
    dialog.setFilter(filter);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setDefaultSuffix( 
            (m_stackedWidget->currentIndex() == InputWindow ) ? "stri" : "stro" );

    if (m_settings->contains("mainwindow-file-dialog"))
        dialog.restoreState( m_settings->value("mainwindow-file-dialog").toByteArray() );

    if (dialog.exec()) {
        m_fileName = dialog.selectedFiles().at(0);
        // Save the state
        m_settings->setValue( "mainwindow-file-dialog", dialog.saveState() );
        // Update the document title
        setCurrentFile(m_fileName);
        // Save the model if a file was selected
        return save();
    } else
        return false;
}

void MainWindow::paste()
{
    QMetaObject::invokeMethod( QApplication::focusWidget(), "paste");
}

void MainWindow::exportData()
{
    QMetaObject::invokeMethod( m_stackedWidget->currentWidget(), "exportData");
}

void MainWindow::print()
{
    // Create a dialog to select the printer
    QPrintDialog printDialog(m_printer);

    if ( printDialog.exec() == QDialog::Accepted )
        QMetaObject::invokeMethod( m_stackedWidget->currentWidget(), "print", Q_ARG( QPrinter *, m_printer) );
}

void MainWindow::printToPdf()
{
    // Create a dialog to set the file to save the pdf to
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save PDF as..."), QDir::currentPath(), tr("PDF File (*.pdf)"));

    if ( fileName.isEmpty() )
        return;
    else
        m_printer->setOutputFileName(fileName);

    QMetaObject::invokeMethod( m_stackedWidget->currentWidget(), "print", Q_ARG( QPrinter *, m_printer) );
}

void MainWindow::copy()
{
    QMetaObject::invokeMethod( QApplication::focusWidget(), "copy");
}

void MainWindow::showNonLinearDialog()
{
    NonLinearPropertyLibraryDialog * dialog = 
        new NonLinearPropertyLibraryDialog(m_inputWidget->model()->nlPropertyLibrary());

    dialog->exec();

    // Save the modified library
    m_inputWidget->model()->nlPropertyLibrary()->save();
}

void MainWindow::switchView()
{
    bool modified = true;
            
    if ( okToClose() ) {
        if ( m_stackedWidget->currentIndex() == InputWindow ) {
            // If the calculation was canceled do nothing
            if ( ! m_inputWidget->model()->okToContinue() )
                return;
            // Input --> Output
            m_stackedWidget->setCurrentIndex(OutputWindow);

            // Set the model of the output widget
            m_outputWidget->setModel(m_inputWidget->outputModel(), OutputWidget::Input);

            modified = true; 
            // Change the text of the in 
            // Determine the file name
            m_fileName = m_outputWidget->model()->fileName();
            // Enable the non-linear curve database
            m_nlCurveDatabaseAction->setEnabled(false);
        } else {
            // Output --> Input
            m_stackedWidget->setCurrentIndex(InputWindow);
           
            modified = false;
            // Determine the file name
            m_fileName = m_inputWidget->model()->fileName();
            // Enable the non-linear curve database
            m_nlCurveDatabaseAction->setEnabled(true);
        }
    }
    
    setCurrentFile(m_fileName);

    // Each widget should have a modified flag
    setModified(modified);
}

void MainWindow::help()
{
    m_helpDialog->show();
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

void MainWindow::updateMenus(int index)
{
    if ( index == InputWindow ) {
        m_resetAction->setEnabled(true);
        m_switchViewAction->setText(tr("Output View"));
    } else if ( index == OutputWindow ) {
        m_resetAction->setEnabled(false);
        m_switchViewAction->setText(tr("Input View"));
    }
}

void MainWindow::setModified(bool modified)
{
    m_modified = modified;

    setWindowModified(modified);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (okToClose())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::okToClose()
{
    // Prompt that the input has been modified
    if (m_modified) {
        int ret = QMessageBox::warning(this, tr("Strata"), 
                tr("The form has been modified.\n" 
                    "Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                QMessageBox::Save); 

        if ( ret & QMessageBox::Save ) {
            // Save and close
            if (!save())
                // Unsuccessful save
                return false;
        } else if ( ret & QMessageBox::Cancel ) {
            // Do not close
            return false;
        }
    }

    return true;
}
