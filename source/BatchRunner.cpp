#include "BatchRunner.h"

#include "OutputCatalog.h"
#include "TextLog.h"

#include <QtDebug>
#include <QTextDocument>

BatchRunner::BatchRunner(const QStringList &fileNames) :
    _fileNames(fileNames), _begin(0), _end(100)
{
    startNext();
}

void BatchRunner::startNext()
{
    if (!_fileNames.isEmpty()) {
        exit(0);
    }

    const QString fileName = _fileNames.takeFirst();
    qInfo().noquote() << "[BATCH] Opening:" << fileName;
    _model = new SiteResponseModel;

    if (fileName.endsWith(".strata")) {
        _model->loadBinary(fileName);
    } else {
        _model->loadJson(fileName);
    }
    // Clean the run
    _model->clearResults();

    // Need to call save after task is done, order not handled correctly....
    connect(_model->outputCatalog()->log(), SIGNAL(textChanged(QString)),
            this, SLOT(updateLog(QString)));
    connect(_model, SIGNAL(progressRangeChanged(int,int)),
            this, SLOT(rangeChanged(int,int)));
    connect(_model, SIGNAL(progressChanged(int)),
            this, SLOT(updateEtc(int)));
    connect(_model, SIGNAL(finished()), this, SLOT(finalize()));

    _model->start();
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
    if (value == _begin) {
        _timer.restart();
    } else if (value == _end) {
        return;
    }

    // Compute the average time per step of the progress bar
    double avgRate = double(_timer.elapsed()) / double(value - _begin);

    // The estimated time of completion is computed by multiplying the average rate by the number of remaining increments
    QTime eta = QTime::currentTime().addMSecs(int(avgRate * (_end-value)));

    qInfo().noquote() << "[BATCH] ETC:" << eta.toString(Qt::LocalDate)
                      << "(" << value + 1 << "of" << _end << ")";
}

void BatchRunner::rangeChanged(int begin, int end)
{
    _begin = begin;
    _end = end;
}

void BatchRunner::finalize()
{
    const QString fileName = _model->fileName();

    qInfo().noquote() << "[BATCH] Saving results to:" << fileName;
    if (fileName.endsWith(".strata")) {
        _model->saveBinary();
    } else {
        _model->saveJson();
    }
    qInfo().noquote() << "[BATCH] Completed processing:" << fileName;

    _model->deleteLater();
    startNext();
}
