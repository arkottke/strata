#ifndef BATCHRUNNER_H
#define BATCHRUNNER_H

#include <QtCore>
#include <QTimer>
#include <QTextStream>

#include "SiteResponseModel.h"

class BatchRunner : public QObject
{
    Q_OBJECT

public:
    BatchRunner(QStringList fileNames);
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
    QTime _timer;
};

#endif // BATCHRUNNER_H
