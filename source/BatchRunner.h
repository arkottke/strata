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

#ifndef BATCHRUNNER_H
#define BATCHRUNNER_H

#include <QtCore>
#include <QElapsedTimer>
#include <QTextStream>

#include "SiteResponseModel.h"

class BatchRunner : public QObject
{
    Q_OBJECT

public:
    explicit BatchRunner(const QStringList &fileNames);
    void startNext();

public slots:
    void updateLog(QString line);
    void updateEtc(int value);
    void rangeChanged(int _begin, int _end);
    void finalize();

private:
    // All files to process
    QStringList _fileNames;

    // Current site response model
    SiteResponseModel * _model;

    // model range for timing
    int _begin;
    int _end;

    // timer for updates
    QElapsedTimer _timer;
};

#endif // BATCHRUNNER_H
