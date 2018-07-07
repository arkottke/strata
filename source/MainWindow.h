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

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_


#include <QMainWindow>
#include <QCloseEvent>
#include <QTabWidget>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QString>
#include <QStackedWidget>
#include <QPrinter>
#include <QSettings>

class AbstractPage;
class ConfiningStressDialog;
class ConfigurePlotDialog;
class HelpDialog;
class OutputExportDialog;
class ResultsPage;
class SiteResponseModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent * event);

    enum Pages {
        COMPUTE_PAGE = 5,
        RESULTS_PAGE = 6};

public slots:
    void open(QString fileName = "");

protected slots:
    //! Reset the window the default values
    void newModel();

    bool save();
    bool saveAs();

    void exportData();

    void print();
    void printToPdf();
    // void pageSetup();

    void showNonlinearDialog();
    void showConfiningStressDialog();

    //! Update the title of the window to reflect the fileName of the document
    void updateWindowTitle(const QString & fileName);

    void help();
    // FIXME void update();
    void about();

    //! Update the tabs to reflect the state of the widget (working and having results)
    void updateTabs();
    void setReadOnly(bool readOnly);

    void tabChanged(int tab);

private:
    //! Create the actions
    void createActions();

    //! Create the tab widget and pages
    void createPages();

    //! Create the menus
    void createMenus();

    //! Create the toolbar
    void createToolbar();

    //! Set the model
    void setModel(SiteResponseModel* model);


    //! Allow the user to save changes before things happen
    /*!
     * \return true to continue
     */
    bool okToClose();

    //! Help dialog
    HelpDialog * _helpDialog;

    ConfiningStressDialog * _confiningStressDialog;

    QAction * _readOnlyAction;

    QAction * _newAction;
    QAction * _openAction;

    QAction * _exportAction;

    QAction * _printAction;
    QAction * _printToPdfAction;

    QAction * _saveAction;
    QAction * _saveAsAction;
    QAction * _exitAction;

    QAction * _nlCurveDatabaseAction;
    QAction * _confiningStressDialogAction;

    QAction * _helpAction;
    QAction * _updateAction;
    QAction * _aboutAction;

    QToolBar * _toolBar;

    QTabWidget * _tabWidget;

    QPrinter * _printer;

    QSettings * _settings;
    
    ResultsPage* _resultsPage;
    QList<AbstractPage*> _pages;

    SiteResponseModel * _model;
};
#endif
