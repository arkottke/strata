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

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include "SiteResponseModel.h"

#include "GeneralPage.h"
#include "SoilTypePage.h"
#include "SoilProfilePage.h"
#include "MotionPage.h"
#include "OutputPage.h"
#include "ComputePage.h"
#include "ResultsPage.h"

#include "HelpDialog.h"
#include "ConfiningStressDialog.h"
#include "OutputExportDialog.h"
#include "ConfigurePlotDialog.h"

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

        public:
    MainWindow(QMainWindow *parent = 0);

protected:
    void closeEvent(QCloseEvent * event);

    enum Pages {
        COMPUTE_PAGE = 5,
        RESULTS_PAGE = 6 };

public slots:

protected slots:
    //! Reset the window the default values
    void reset();
    void open( const QString & fileName = "" );

    bool save();
    bool saveAs();

    void exportData();

    void print();
    void printToPdf();
    // void pageSetup();

    void showNonlinearDialog();
    void showConfiningStressDialog();

    void help();
    void update();
    void about();

    void setIsWorking(bool isWorking);
    void setReadOnly(bool readOnly = false);

    void tabChanged(int tab);

        private:
    //! Create the actions
    void createActions();

    //! Create the tab widget and pages
    void createPage();

    //! Create the menus
    void createMenus();

    //! Create the toolbar
    void createToolbar();

    //! Set the current file of the application
    void setCurrentFile(const QString & fileName);

    //! Allow the user to save changes before things happen
    /*!
         * \return true to continue
         */
    bool okToClose();

    //! Filename of the model
    QString m_fileName;

    //! Help dialog
    HelpDialog * m_helpDialog;

    ConfiningStressDialog * m_confiningStressDialog;

    QAction * m_readOnlyAction;

    QAction * m_resetAction;
    QAction * m_openAction;

    QAction * m_exportAction;

    QAction * m_printAction;
    QAction * m_printToPdfAction;

    QAction * m_saveAction;
    QAction * m_saveAsAction;
    QAction * m_exitAction;

    QAction * m_nlCurveDatabaseAction;
    QAction * m_confiningStressDialogAction;

    QAction * m_helpAction;
    QAction * m_updateAction;
    QAction * m_aboutAction;

    QToolBar * m_toolBar;

    QTabWidget * m_tabWidget;

    QPrinter * m_printer;

    QSettings * m_settings;
    
    GeneralPage * m_generalPage;
    SoilTypePage * m_soilTypePage;
    SoilProfilePage * m_soilProfilePage;
    MotionPage * m_motionPage;
    OutputPage * m_outputPage;
    ComputePage * m_computePage;
    ResultsPage * m_resultsPage;

    SiteResponseModel * m_model;

    //! Input values are in a read-only state
    bool m_readOnly;
};
#endif
