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

#include "InputWidget.h"
#include "OutputWidget.h"
#include "HelpDialog.h"

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

        enum Window{
            InputWindow, //!< Input Window
            OutputWindow //!< Output Window
        };

    protected:
        void closeEvent(QCloseEvent * event);
       
    public slots:
        void setModified(bool modified = true);

	protected slots:
        void reset();
        void open( const QString & fileName = "" );
        
        bool save();
        bool saveAs();

        void exportData();

        void print();
        void printToPdf();
        // void pageSetup();

        void copy();
        void paste();

        void showNonLinearDialog();

        void switchView();

        void help();
        void about();

        void updateMenus(int widgetIndex);

	private:
        //! Create the actions
        void createActions();

        //! Create the tab widget and pages
		void createPage();
        
        //! Create the menus
        void createMenus();

        //! Set the current file of the application
        void setCurrentFile(const QString & fileName);

        //! Allow the user to save changes before things happen
        /*!
         * \return true to continue
         */
        bool okToClose();
        
        //! Filename of the model
        QString m_fileName;

        //! Whether the model has been modified since the last save
        bool m_modified;

        //! Help dialog
        HelpDialog * m_helpDialog;
        
        QAction * m_resetAction;
        QAction * m_openAction;

        QAction * m_exportAction;

        QAction * m_printAction;
        QAction * m_printToPdfAction;

        QAction * m_saveAction;
        QAction * m_saveAsAction;
        QAction * m_exitAction;
        
        QAction * m_copyAction;
        QAction * m_pasteAction;

        QAction * m_nlCurveDatabaseAction;

        QAction * m_switchViewAction;
        
        QAction * m_helpAction;
        QAction * m_aboutAction;

        QStackedWidget * m_stackedWidget;
        InputWidget * m_inputWidget;
        OutputWidget * m_outputWidget;

        QPrinter * m_printer;

        QSettings * m_settings;
};
#endif
