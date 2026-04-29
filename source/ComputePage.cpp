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

#include "ComputePage.h"

#include "OutputCatalog.h"
#include "SiteResponseModel.h"
#include "TextLog.h"

#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLocale>
#include <QTime>

ComputePage::ComputePage(QWidget *parent, Qt::WindowFlags f)
    : AbstractPage(parent, f), _model(nullptr) {
  auto *layout = new QGridLayout;

  // Progress bar
  _progressBar = new QProgressBar;
  connect(_progressBar, &QProgressBar::valueChanged, this,
          &ComputePage::updateEta);
  layout->addWidget(_progressBar, 0, 0);

  // Completion time
  _etaLineEdit = new QLineEdit;
  _etaLineEdit->setReadOnly(true);
  _etaLineEdit->setMinimumWidth(150);
  _etaLineEdit->setAlignment(Qt::AlignLeft);

  layout->addWidget(new QLabel(tr("ETC:")), 0, 1);
  layout->addWidget(_etaLineEdit, 0, 2);

  // Buttons
  _cancelButton =
      new QPushButton(QIcon(":/images/process-stop.svg"), tr("Cancel"));
  _cancelButton->setEnabled(false);

  layout->addWidget(_cancelButton, 0, 3);

  //_computeButton = new QPushButton(QIcon(":/images/system-run.svg"),
  // tr("Compute"));
  _computeButton = new QPushButton(tr("Compute"));
  _computeButton->setDefault(true);
  connect(_computeButton, &QPushButton::clicked, this, &ComputePage::compute);

  layout->addWidget(_computeButton, 0, 4);

  // Text area
  _logView = new QTextEdit;
  _logView->setReadOnly(true);
  _logView->setTabStopDistance(20);

  layout->addWidget(_logView, 1, 0, 1, 5);

  // Set stretch row and column
  layout->setColumnStretch(0, 1);
  layout->setRowStretch(0, 1);

  setLayout(layout);
}

void ComputePage::setModel(SiteResponseModel *model) {
  _model = model;

  connect(model, &SiteResponseModel::progressRangeChanged, _progressBar,
          &QProgressBar::setRange);

  connect(model, &SiteResponseModel::progressChanged, _progressBar,
          &QProgressBar::setValue);

  connect(_cancelButton, &QPushButton::clicked, model,
          &SiteResponseModel::stop);

  connect(this, &ComputePage::startCalculation, model,
          [model]() { model->start(); });
  connect(model, &SiteResponseModel::finished, this, &ComputePage::reset);

  _logView->clear();
  for (const QString &line : model->outputCatalog()->log()->text())
    _logView->append(line);

  connect(model->outputCatalog()->log(), &TextLog::textChanged, _logView,
          &QTextEdit::append);
  connect(model->outputCatalog()->log(), &TextLog::textCleared, _logView,
          &QTextEdit::clear);
}

void ComputePage::setReadOnly(bool b) {
  _computeButton->setDisabled(b || _model->hasResults());
  // Only enabled by the compute button
  _cancelButton->setDisabled(true);

  _cancelButton->setCursor(Qt::ArrowCursor);
}

void ComputePage::compute() {
  emit saveRequested();

  _logView->clear();
  // Disable the compute button
  _computeButton->setEnabled(false);
  _cancelButton->setEnabled(true);

  _cancelButton->setCursor(Qt::BusyCursor);

  _model->start();
}

void ComputePage::reset() { setReadOnly(false); }

void ComputePage::updateEta(int value) {
  if (value == _progressBar->minimum()) {
    _timer.restart();
  } else {
    // Compute the average time per step of the progress bar
    double avgRate =
        double(_timer.elapsed()) / double(value - _progressBar->minimum());

    // The estimated time of completion is computed by multiplying the average
    // rate by the number of remaining increments
    QTime eta = QTime::currentTime().addMSecs(
        int(avgRate * (_progressBar->maximum() - value)));

    _etaLineEdit->setText(QLocale::system().toString(eta));
    _etaLineEdit->setCursorPosition(0);
  }
}
