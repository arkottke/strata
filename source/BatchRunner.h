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
    BatchRunner(QString filename);
    void run();

public slots:
    void updateLog(QString line);
    void updateEta(int value);
    void rangeChanged(int m_begin, int m_end);
    void finalize();

private:
    // model data
    QString fileName;
    // model runner
    SiteResponseModel * m_model;
    // model range for timing
    int m_begin;
    int m_end;
    // timer for updates
    QTime m_timer;
};

#endif // BATCHRUNNER_H
