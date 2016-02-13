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
#include "BatchRunner.h"

#include <QtCore>
#include <QString>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QTextStream>
#include <QDebug>

#include <fstream>
#include <iostream>

//void myMessageOutput(QtMsgType type, const char *msg)
//{
//    switch (type) {
//        case QtDebugMsg:
//            fprintf( stdout, "Debug: %s\n", msg );
//            break;
//        case QtWarningMsg:
//            QMessageBox::warning(0, "Strata", msg);
//            break;
//        case QtCriticalMsg:
//            QMessageBox::critical(0, "Strata", msg);
//            break;
//        case QtFatalMsg:
//            QMessageBox::critical(0, "Strata", msg);
//            abort();
//        }
//}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

int main(int argc, char* argv[])
{
#ifndef DEBUG
    qInstallMessageHandler(myMessageOutput);
#endif

    bool batch = false;
    for (int i = 0; i < argc; i++) {
        QString a = QString::fromUtf8(argv[i]);
        if (a == "?" || a == "-?" || a == "--?" || a == "help" || a == "-help" || a == "--help")
        {
            fprintf(stdout, "strata.exe (-b, for batch use) [path_to_file]\n");
            exit(0);
        }
        if (a == "-b") {
            batch = true;
        }
    }

    if (batch) {
        // console hook
        QCoreApplication app(argc, argv);
        // console output hooks
        QTextStream qerr(stderr);

        const char * fn = argv[argc-1];
        QString fileName = QString::fromUtf8(fn);
        // check if input file exists
        std::ifstream input(fn, std::ios::binary);
        if (!input)
        {
            qerr << "file not found: " << fn << endl;
            app.exit(-1);
        }
        BatchRunner * b = new BatchRunner(fileName);
        b->run();
        return app.exec();
    } else {
        // Normal application mode
        QApplication app(argc, argv);

        // Set the window icon
        app.setWindowIcon(QIcon(":/images/application-icon.svg"));

        QCoreApplication::setOrganizationName("accipter");
        QCoreApplication::setApplicationName("Strata");

        MainWindow * mainWindow = new MainWindow;
        mainWindow->showMaximized();
        return app.exec();
    }
} 
