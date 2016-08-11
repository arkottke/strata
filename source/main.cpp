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
#include <QCommandLineOption>
#include <QString>
#include <QMessageBox>
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QTextStream>
#include <QDebug>

#include <fstream>
#include <iostream>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s\n", localMsg.constData());
        fflush(stdout);
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

QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        if (qstrcmp(argv[i], "-b") == 0 || qstrcmp(argv[i], "-batch") == 0)
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}

int main(int argc, char* argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));

    QCoreApplication::setApplicationName("Strata");
    QCoreApplication::setApplicationVersion(VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription(
                "Strata - site response with RVT and simulated properties");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"b", "batch"},
            QCoreApplication::translate("main", "Batch mode without a GUI")},
     });
    parser.addPositionalArgument(
                "file",
                QCoreApplication::translate(
                    "main",
                    "Strata JSON or binary files to process. Multiple files"
                    "are only supported by batch mode (-b)."),
                "file1 [file2 file3...]");

    parser.process(*app.data());
    const QStringList args = parser.positionalArguments();

    if (qobject_cast<QApplication *>(app.data())) {
        // GUI Version
#ifndef DEBUG
        qInstallMessageHandler(myMessageOutput);
#endif
        qobject_cast<QApplication *>(app.data())->setWindowIcon(QIcon(":/images/application-icon.svg"));
        MainWindow * mainWindow = new MainWindow;
        if (args.size())
            mainWindow->open(args.at(0));
        mainWindow->showMaximized();
    } else {
        // start non-GUI version...
        qInstallMessageHandler(myMessageOutput);

        if (args.size() < 1) {
            qFatal("At least one file must be specified.");
            return 1;
        }
        BatchRunner *br = new BatchRunner(args);
        Q_UNUSED(br);
    }

    return app.data()->exec();
}
