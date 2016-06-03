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
    void rangeChanged(int m_begin, int m_end);
    void finalize();

private:
    // All files to process
    QStringList m_fileNames;

    // Current site response model
    SiteResponseModel * m_model;

    // model range for timing
    int m_begin;
    int m_end;

    // timer for updates
    QTime m_timer;
};

#endif // BATCHRUNNER_H
