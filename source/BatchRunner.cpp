#include "BatchRunner.h"

#include "OutputCatalog.h"
#include "TextLog.h"

#include <QtDebug>
#include <QTextDocument>

BatchRunner::BatchRunner(QStringList fileNames):
    m_fileNames(fileNames), m_begin(0), m_end(100)
{
    startNext();
}

void BatchRunner::startNext()
{
    if (!m_fileNames.size())
        exit(0);

    const QString fileName = m_fileNames.takeFirst();
    qInfo().noquote() << "[BATCH] Opening:" << fileName;
    m_model = new SiteResponseModel;

    if (fileName.endsWith(".strata")) {
        m_model->loadBinary(fileName);
    } else {
        m_model->loadJson(fileName);
    }
    // Clean the run
    m_model->clearResults();

    // Need to call save after task is done, order not handled correctly....
    connect(m_model->outputCatalog()->log(), SIGNAL(textChanged(QString)),
            this, SLOT(updateLog(QString)));
    connect(m_model, SIGNAL(progressRangeChanged(int,int)),
            this, SLOT(rangeChanged(int,int)));
    connect(m_model, SIGNAL(progressChanged(int)),
            this, SLOT(updateEtc(int)));
    connect(m_model, SIGNAL(finished()), this, SLOT(finalize()));

    m_model->start();
}

void BatchRunner::updateLog(QString line)
{
    // Remove HTML formatting
    QTextDocument doc;
    doc.setHtml(line);

    // Add spaces to indent to [BATCH]
    qInfo().noquote() << "       " << doc.toPlainText().replace("\t", "    ");
}

void BatchRunner::updateEtc(int value)
{
    if (value == m_begin) {
        m_timer.restart();
    } else if (value == m_end) {
        return;
    }

    // Compute the average time per step of the progress bar
    double avgRate = double(m_timer.elapsed()) / double(value - m_begin);

    // The estimated time of completion is computed by multiplying the average rate by the number of remaining increments
    QTime eta = QTime::currentTime().addMSecs(int(avgRate * (m_end-value)));

    qInfo().noquote() << "[BATCH] ETC:" << eta.toString(Qt::LocalDate)
                      << "(" << value + 1 << "of" << m_end << ")";
}

void BatchRunner::rangeChanged(int begin, int end)
{
    m_begin = begin;
    m_end = end;
}

void BatchRunner::finalize()
{
    const QString fileName = m_model->fileName();

    qInfo().noquote() << "[BATCH] Saving results to:" << fileName;
    if (fileName.endsWith(".strata")) {
        m_model->saveBinary();
    } else {
        m_model->saveJson();
    }
    qInfo().noquote() << "[BATCH] Completed processing: %s" << fileName;

    m_model->deleteLater();
    startNext();
}
