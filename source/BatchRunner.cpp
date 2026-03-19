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

#include "BatchRunner.h"

#include "OutputCatalog.h"
#include "TextLog.h"

#include <QLocale>
#include <QTextDocument>
#include <QtDebug>

BatchRunner::BatchRunner(const QStringList &fileNames)
    : _fileNames(fileNames), _begin(0), _end(100) {
  startNext();
}

void BatchRunner::startNext() {
  if (_fileNames.isEmpty()) {
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
  connect(_model->outputCatalog()->log(), &TextLog::textChanged, this,
          &BatchRunner::updateLog);
  connect(_model, &SiteResponseModel::progressRangeChanged, this,
          &BatchRunner::rangeChanged);
  connect(_model, &SiteResponseModel::progressChanged, this,
          &BatchRunner::updateEtc);
  connect(_model, &SiteResponseModel::finished, this, &BatchRunner::finalize);

  _model->start();
}

void BatchRunner::updateLog(QString line) {
  // Remove HTML formatting
  QTextDocument doc;
  doc.setHtml(line);

  // Add spaces to indent to [BATCH]
  qInfo().noquote() << "       " << doc.toPlainText().replace("\t", "    ");
}

void BatchRunner::updateEtc(int value) {
  if (value == _begin) {
    _timer.restart();
  } else if (value == _end) {
    return;
  }

  // Compute the average time per step of the progress bar
  double avgRate = double(_timer.elapsed()) / double(value - _begin);

  // The estimated time of completion is computed by multiplying the average
  // rate by the number of remaining increments
  QTime eta = QTime::currentTime().addMSecs(int(avgRate * (_end - value)));

  qInfo().noquote() << "[BATCH] ETC:" << QLocale::system().toString(eta) << "("
                    << value + 1 << "of" << _end << ")";
}

void BatchRunner::rangeChanged(int begin, int end) {
  _begin = begin;
  _end = end;
}

void BatchRunner::finalize() {
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
