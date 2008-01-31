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

#include <QString>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

void myMessageOutput(QtMsgType type, const char *msg)
{
	switch (type) {
		case QtDebugMsg:
			fprintf( stdout, "Debug: %s\n", msg );
			// if ( logBox == 0 )
		    //		logBox = new QTextEdit;
			// logBox->append( msg );
			break;
		case QtWarningMsg:
			QMessageBox::warning( 0, "Strata", msg );
			break;
		case QtCriticalMsg:
			QMessageBox::critical( 0, "Strata", msg );
			break;
		case QtFatalMsg:
			QMessageBox::critical( 0, "Strata", msg );
			abort();
	}
}

int main( int argc, char* argv[] )
{
	qInstallMsgHandler(myMessageOutput);
	QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Albert Kottke");
    QCoreApplication::setOrganizationDomain("accipter.org");
    QCoreApplication::setApplicationName("Strata");

    MainWindow * mainWindow = new MainWindow;
    mainWindow->showMaximized();
	
    return app.exec();
} 
