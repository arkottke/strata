#include "BatchRunner.h"

#include "OutputCatalog.h"
#include "TextLog.h"

BatchRunner::BatchRunner(QString filename)
{
    m_begin = 0;
    m_end = 100;
    fileName = filename;
    // console output hooks
    QTextStream(stdout) << "preparing batch for file " << fileName << endl;
    m_model = new SiteResponseModel;
    if (fileName.endsWith(".strata"))
    {
        m_model->load(fileName);
    }
    else
    {
        m_model->loadReadable(fileName);
    }
    // Clean the run
    m_model->clearResults();
    // Need to call save after task is done, order not handled correctly....
    QObject::connect(m_model->outputCatalog()->log(), SIGNAL(textChanged(QString)), this, SLOT(updateLog(QString)));
    QObject::connect(m_model, SIGNAL(progressRangeChanged(int,int)), this, SLOT(rangeChanged(int,int)));
    QObject::connect(m_model, SIGNAL(progressChanged(int)), this, SLOT(updateEta(int)));
    QObject::connect(m_model, SIGNAL(finished()), this, SLOT(finalize()));
}

void BatchRunner::run()
{
    QTextStream(stdout) << "starting batch run" << endl;
    m_model->start();
}

void BatchRunner::updateLog(QString line)
{
    QTextStream(stdout) << line << endl;
}

void BatchRunner::updateEta(int value)
{
    if (value == m_begin)
    {
        m_timer.restart();
    }
    // Compute the average time per step of the progress bar
    double avgRate = double(m_timer.elapsed()) / double(value - m_begin);

    // The estimated time of completion is computed by multiplying the average rate by the number of remaining increments
    QTime eta = QTime::currentTime().addMSecs(int(avgRate * (m_end-value)));

    QTextStream(stdout) << "eta: " << eta.toString(Qt::LocalDate) << " ([" << m_begin << ":" << m_end << "] " << value << " )" <<  endl;
}

void BatchRunner::rangeChanged(int begin, int end)
{
    m_begin = begin;
    m_end = end;
}

void BatchRunner::finalize()
{
    QTextStream(stdout) << "finished batch run, saving results" << endl;
    if (fileName.endsWith(".strata"))
    {
        m_model->save();
    }
    else
    {
        m_model->saveReadable();
    }
    QTextStream(stdout) << "done." << endl;
    exit(0);
}
