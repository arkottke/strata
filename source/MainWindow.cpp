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
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"

#include "ComputePage.h"
#include "ConfigurePlotDialog.h"
#include "ConfiningStressDialog.h"
#include "EditActions.h"
#include "GeneralPage.h"
#include "HelpDialog.h"
#include "MotionPage.h"
#include "NonlinearPropertyCatalogDialog.h"
#include "OutputCatalog.h"
#include "OutputExportDialog.h"
#include "OutputPage.h"
#include "ResultsPage.h"
#include "SiteResponseModel.h"
#include "SoilProfile.h"
#include "SoilProfilePage.h"
#include "SoilTypeCatalog.h"
#include "SoilTypePage.h"
#include "defines.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QPrintDialog>
#include <QScrollArea>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent), _model(nullptr) {
  // Initialize the help dialog, but keep it hidden
  _helpDialog = new HelpDialog(this);
  _confiningStressDialog = nullptr;

  _printer = new QPrinter;

  // Create the settings
  _settings = new QSettings(this);

  // Setup up the mainwindow
  createActions();
  createPages();
  createToolbar();
  createMenus();

  newModel();
}

MainWindow::~MainWindow() { delete _printer; }

void MainWindow::createActions() {
  // Read-only
  _readOnlyAction = new QAction(QIcon(":/images/edit-delete.svg"),
                                tr("Delete results"), this);
  _readOnlyAction->setShortcut(tr("F2"));
  _readOnlyAction->setEnabled(false);

  // New
  _newAction =
      new QAction(QIcon(":/images/document-new.svg"), tr("&New"), this);
  _newAction->setShortcut(tr("Ctrl+n"));
  connect(_newAction, SIGNAL(triggered()), this, SLOT(newModel()));

  // Open
  _openAction =
      new QAction(QIcon(":/images/document-open.svg"), tr("&Open..."), this);
  _openAction->setShortcut(tr("Ctrl+o"));
  connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

  // Save
  _saveAction =
      new QAction(QIcon(":/images/document-save.svg"), tr("&Save"), this);
  _saveAction->setShortcut(tr("Ctrl+s"));
  connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));
  // Save As
  _saveAsAction = new QAction(QIcon(":/images/document-save-as.svg"),
                              tr("Save &As..."), this);
  connect(_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  // Export the data to text file
  _exportAction = new QAction(QIcon(":/images/document-export.svg"),
                              tr("&Export..."), this);
  _exportAction->setEnabled(false);
  connect(_exportAction, SIGNAL(triggered()), this, SLOT(exportData()));

  // Print
  _printAction =
      new QAction(QIcon(":/images/document-print.svg"), tr("&Print..."), this);
  _printAction->setShortcut(tr("Ctrl+p"));
  connect(_printAction, SIGNAL(triggered()), this, SLOT(print()));

  // Print to pdf
  _printToPdfAction = new QAction(tr("&Print to PDF..."), this);
  connect(_printToPdfAction, SIGNAL(triggered()), this, SLOT(printToPdf()));

  // Exit
  _exitAction = new QAction(tr("E&xit"), this);
  connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

  // Non Linear Curve Database
  _nlCurveDatabaseAction =
      new QAction(QIcon(":/images/system-file-manager.svg"),
                  tr("Add/Remove Nonlinear Curves"), this);
  connect(_nlCurveDatabaseAction, SIGNAL(triggered()), this,
          SLOT(showNonlinearDialog()));

  // Confining Stress Calculator
  _confiningStressDialogAction =
      new QAction(QIcon(":/images/accessories-calculator.svg"),
                  tr("Confining Stress Calculator"), this);
  connect(_confiningStressDialogAction, SIGNAL(triggered()), this,
          SLOT(showConfiningStressDialog()));

  // Help Action
  _helpAction =
      new QAction(QIcon(":/images/help-browser.svg"), tr("&User Manual"), this);
  _helpAction->setShortcut(tr("F1"));
  connect(_helpAction, SIGNAL(triggered()), this, SLOT(help()));

  // Check for updates
  // _updateAction = new QAction(tr("&Check for updates..."), this);
  // connect(_updateAction, SIGNAL(triggered()), this, SLOT(update()));

  // About action
  _aboutAction = new QAction(tr("&About"), this);
  _aboutAction->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createPages() {
  _tabWidget = new QTabWidget;
  connect(_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

  // Create the pages
  _pages << new GeneralPage;
  _tabWidget->addTab(_pages.last(), tr("General Settings"));

  _pages << new SoilTypePage;
  connect(_pages.last(), SIGNAL(linkActivated(QString)), _helpDialog,
          SLOT(gotoLink(QString)));
  _tabWidget->addTab(_pages.last(), tr("Soil Types"));

  _pages << new SoilProfilePage;
  _tabWidget->addTab(_pages.last(), tr("Soil Profile"));

  _pages << new MotionPage;
  _tabWidget->addTab(_pages.last(), tr("Motion(s)"));

  _pages << new OutputPage;
  _tabWidget->addTab(_pages.last(), tr("Output Specification"));

  auto *cp = new ComputePage;
  connect(cp, SIGNAL(saveRequested()), this, SLOT(save()));
  _pages << cp;
  _tabWidget->addTab(_pages.last(), tr("Compute"));

  _resultsPage = new ResultsPage;
  _pages << _resultsPage;
  _tabWidget->addTab(_pages.last(), tr("Results"));

  // Place the tab widget in a scroll area
  auto *scrollArea = new QScrollArea;
  scrollArea->setWidget(_tabWidget);
  scrollArea->setWidgetResizable(true);

  setCentralWidget(scrollArea);
}

void MainWindow::createMenus() {
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(_newAction);
  fileMenu->addAction(_openAction);
  fileMenu->addSeparator();
  fileMenu->addAction(_saveAction);
  fileMenu->addAction(_saveAsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(_exportAction);
  fileMenu->addSeparator();
  fileMenu->addAction(_printAction);
  fileMenu->addAction(_printToPdfAction);
  fileMenu->addSeparator();
  fileMenu->addAction(_exitAction);

  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(EditActions::instance()->cutAction());
  editMenu->addAction(EditActions::instance()->copyAction());
  editMenu->addAction(EditActions::instance()->pasteAction());
  editMenu->addAction(EditActions::instance()->clearAction());

  QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(_readOnlyAction);
  toolsMenu->addSeparator();
  toolsMenu->addAction(_nlCurveDatabaseAction);
  toolsMenu->addAction(_confiningStressDialogAction);

  QMenu *windowMenu = menuBar()->addMenu(tr("&Window"));
  windowMenu->addAction(_toolBar->toggleViewAction());

  QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(_helpAction);
  helpMenu->addSeparator();
  // FIXME: Update is currently disabled until hosting service can be figured
  // out.
  // helpMenu->addAction(_updateAction);
  helpMenu->addAction(_aboutAction);
}

void MainWindow::createToolbar() {
  _toolBar = new QToolBar(tr("Main ToolBar"), this);

  _toolBar->addAction(_readOnlyAction);

  _toolBar->addSeparator();
  _toolBar->addAction(_newAction);
  _toolBar->addAction(_openAction);
  _toolBar->addAction(_saveAction);
  _toolBar->addAction(_saveAsAction);

  _toolBar->addSeparator();
  _toolBar->addAction(_printAction);
  _toolBar->addAction(_exportAction);

  _toolBar->addSeparator();
  _toolBar->addAction(_nlCurveDatabaseAction);

  _toolBar->addSeparator();
  _toolBar->addAction(_helpAction);

  addToolBar(_toolBar);
}

void MainWindow::newModel() {
  if (!okToClose()) {
    return;
  }

  setModel(new SiteResponseModel);
}

void showExtensionError(QWidget *parent) {
  QMessageBox::critical(parent, "Invalid file extension",
                        "Invalid file extension."
                        "Possible extensions are \".strata\" for binary files,"
                        "and \".strata.json\" for JSON text files.");
}

void MainWindow::open(QString fileName) {
  if (!okToClose()) {
    return;
  }

  if (fileName.isEmpty()) {
    // Prompt for a fileName
    fileName = QFileDialog::getOpenFileName(
        this, tr("Open strata file..."),
        (_model->fileName().isEmpty()
             ? _settings
                   ->value("projectDirectory",
                           QStandardPaths::writableLocation(
                               QStandardPaths::DocumentsLocation))
                   .toString()
             : _model->fileName()),
        "Strata Files (*.strata *.json);;"
        "Strata Binary File (*.strata);;"
        "Strata JSON File (*.json);;");

    if (fileName.isEmpty()) {
      return;
    }
  } else {
    // Check that the provided filename exists
    if (!QFileInfo(fileName).exists()) {
      qWarning() << qPrintable(fileName) << "does not exist!";
      return;
    }
  }

  // Save the state
  _settings->setValue("projectDirectory", QFileInfo(fileName).filePath());

  auto *srm = new SiteResponseModel;
  if (fileName.endsWith(".strata")) {
    srm->loadBinary(fileName);
  } else if (fileName.endsWith(".json")) {
    srm->loadJson(fileName);
  } else {
    return showExtensionError(this);
  }

  setModel(srm);
}

auto MainWindow::save() -> bool {
  // If no file name is selected run saveAs
  if (_model->fileName().isEmpty())
    return saveAs();

  const QString &fileName = _model->fileName();

  // Save the model
  if (fileName.endsWith(".strata")) {
    _model->saveBinary();
  } else if (fileName.endsWith(".json")) {
    _model->saveJson();
  } else {
    showExtensionError(this);
    return false;
  }

  return true;
}

auto MainWindow::saveAs() -> bool {
  // Prompt for a fileName
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save file as..."),
      (_model->fileName().isEmpty()
           ? _settings
                 ->value("projectDirectory",
                         QStandardPaths::writableLocation(
                             QStandardPaths::DocumentsLocation))
                 .toString()
           : _model->fileName()),
      "Strata Files (*.strata *.json);;"
      "Strata Binary File (*.strata);;"
      "Strata JSON File (*.json);;");

  if (!fileName.isEmpty()) {
    // Make sure that the file name ends with .strata or .stratahr
    if (!fileName.endsWith(".strata", Qt::CaseInsensitive) &&
        !fileName.endsWith(".json", Qt::CaseInsensitive)) {
      fileName.append(".json");
    }

    // Save the state
    _settings->setValue("projectDirectory", QFileInfo(fileName).path());
    // Update the document title
    _model->setFileName(fileName);
    // Save the model if a file was selected
    return save();
  } else {
    return false;
  }
}

void MainWindow::exportData() { _resultsPage->exportData(); }

void MainWindow::print() {
  // Create a dialog to select the printer
  QPrintDialog printDialog(_printer);

  if (printDialog.exec()) {
    if (_tabWidget->currentIndex() == RESULTS_PAGE) {
      _resultsPage->print(_printer);
    } else {
      QTextDocument doc;
      doc.setHtml(_model->toHtml());
      // Print the report
      doc.print(_printer);
    }
  }
}

void MainWindow::printToPdf() {
  // Prompt for a fileName
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Select output file..."),
      _settings
          ->value("exportDirectory", QStandardPaths::writableLocation(
                                         QStandardPaths::DocumentsLocation))
          .toString(),
      "PDF File (*.pdf)");

  if (!fileName.isEmpty()) {
    // Save the state
    _settings->setValue("exportDirectory", QFileInfo(fileName).filePath());
    _printer->setOutputFileName(fileName);

    if (_tabWidget->currentIndex() == RESULTS_PAGE) {
      _resultsPage->print(_printer);
    } else {
      QTextDocument doc;
      doc.setHtml(_model->toHtml());
      // Print the report
      doc.print(_printer);
    }
  }
}

void MainWindow::showNonlinearDialog() {
  NonlinearPropertyCatalogDialog dialog(
      _model->siteProfile()->soilTypeCatalog()->nlCatalog());

  dialog.addAction(EditActions::instance()->cutAction());
  dialog.addAction(EditActions::instance()->copyAction());
  dialog.addAction(EditActions::instance()->pasteAction());

  dialog.exec();

  // Save the modified library
  _model->siteProfile()->soilTypeCatalog()->nlCatalog()->save();
}

void MainWindow::showConfiningStressDialog() {
  if (!_confiningStressDialog) {
    _confiningStressDialog = new ConfiningStressDialog(this);
  }
  _confiningStressDialog->show();
  _confiningStressDialog->raise();
  _confiningStressDialog->activateWindow();
}

void MainWindow::help() { _helpDialog->show(); }

/*
void MainWindow::update()
{
    UpdateDialog ud(this);

    ud.exec();
}
*/

void MainWindow::about() {
  QMessageBox::about(
      this, tr("About Strata"),
      tr("<p><b>Strata</b> was written by Albert Kottke working with Professor"
         " Ellen Rathje at The University of Texas at Austin.</p>"
         "<p>For comments and suggestions contact Albert at "
         "<a href=\"mailto:albert.kottke@gmail.com\">"
         "albert.kottke@gmail.com</a></p>"
         "<p>Version: %1 - %2</p>"
#ifdef ADVANCED_FEATURES
         "<p>Compiled with advanced options</p>"
#endif
         "<p>Website: <a href=\"https://github.com/arkottke/strata\">"
         "https://github.com/arkottke/strata</a></p>")
          .arg(QCoreApplication::applicationVersion())
          .arg(PROJECT_GITHASH));
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (okToClose()) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MainWindow::updateTabs() {
  const bool isBusy = _model->isRunning();

  // Enable/Disable all actions and menus
  for (QObject *object : children()) {
    if (auto *action = qobject_cast<QAction *>(object))
      action->setDisabled(isBusy);
  }

  for (QObject *object : menuBar()->children()) {
    if (auto *menu = qobject_cast<QMenu *>(object))
      menu->setDisabled(isBusy);
  }

  setCursor(isBusy ? Qt::WaitCursor : Qt::ArrowCursor);

  // Set all tabs to be enabled or disabled
  for (int i = 0; i < _tabWidget->count(); ++i)
    _tabWidget->setTabEnabled(i, !isBusy || i == COMPUTE_PAGE);
}

void MainWindow::setReadOnly(bool readOnly) {
  // Set all of the pages to be read only
  for (auto *page : _pages)
    page->setReadOnly(readOnly);

  // Change the icon and label of the button
  _readOnlyAction->setEnabled(readOnly);

  // readOnly means that the model has results and
  // therefore the Results tab needs to be enabled.
  _tabWidget->setTabEnabled(RESULTS_PAGE, readOnly);
  if (readOnly) {
    _tabWidget->setCurrentIndex(RESULTS_PAGE);
    _resultsPage->setModel(_model);
  }

  _exportAction->setEnabled(readOnly);
}

void MainWindow::tabChanged(int tab) {
  if (_settings->value("mainwindow/warnAboutReadOnly", true).toBool() &&
      _model && _model->hasResults() && tab != RESULTS_PAGE) {

    // Create a dialog box that reminds the user that they are in a
    // read-only environment.

    auto *layout = new QVBoxLayout;

    auto *label = new QLabel(
        tr("Strata input fields are locked in a read-only state. "
           "To edit the fields you must unlock the input by clicking "
           "on the lock button in the upper left portion of the window. "
           "Unlocking Strata will delete all results."));

    label->setWordWrap(true);
    layout->addWidget(label);

    auto *checkBox = new QCheckBox(tr("Stop reminding me."));
    layout->addWidget(checkBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout->addWidget(buttons);

    QDialog dialog(this);
    connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
    dialog.setLayout(layout);

    if (dialog.exec())
      _settings->setValue("mainwindow/warnAboutReadOnly",
                          !checkBox->isChecked());
  }
}

void MainWindow::setModel(SiteResponseModel *model) {
  if (_model)
    _model->deleteLater();

  _model = model;

  // Update all of the pages
  for (auto *page : _pages)
    page->setModel(_model);

  updateTabs();
  connect(_readOnlyAction, SIGNAL(triggered()), _model, SLOT(clearResults()));
  setReadOnly(_model->hasResults());
  connect(_model, SIGNAL(hasResultsChanged(bool)), this,
          SLOT(setReadOnly(bool)));

  setWindowModified(false);
  connect(_model, SIGNAL(modifiedChanged(bool)), this,
          SLOT(setWindowModified(bool)));

  updateWindowTitle(_model->fileName());
  connect(_model, SIGNAL(fileNameChanged(QString)), this,
          SLOT(updateWindowTitle(QString)));

  connect(_model, SIGNAL(started()), this, SLOT(updateTabs()));
  connect(_model, SIGNAL(finished()), this, SLOT(updateTabs()));

  // During this process clear() is called on the OutputCatalog which flags
  // the SiteResponseModel as being modified. We should reset this flag.
  // _model->setModified(false);
  // FIXME
}

void MainWindow::updateWindowTitle(const QString &fileName) {
  setWindowTitle(tr("%1[*] - %2")
                     .arg(fileName.isEmpty() ? "untitled.strata" : fileName)
                     .arg("Strata"));
}

auto MainWindow::okToClose() -> bool {
  // Prompt that the input has been modified
  if (_model && _model->modified()) {
    QMessageBox msgBox(this);
    msgBox.setText("The document has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard |
                              QMessageBox::Cancel);
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
