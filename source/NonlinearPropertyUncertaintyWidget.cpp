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

#include "NonlinearPropertyUncertaintyWidget.h"

#include "NonlinearPropertyUncertainty.h"

#include <QDebug>
#include <QLabel>

NonlinearPropertyUncertaintyWidget::NonlinearPropertyUncertaintyWidget(
    const QString &title, QGridLayout *layout, QObject *parent)
    : QObject(parent) {
  int row = layout->rowCount();
  int col = 0;
  layout->addWidget(new QLabel(title), row++, 0, 1, 2);

  // Ln. Stdev
  _lnStdevSpinBox = new QDoubleSpinBox;
  _lnStdevSpinBox->setDecimals(3);
  _lnStdevSpinBox->setSingleStep(0.01);
  _lnStdevSpinBox->setRange(0, 1.0);

  layout->addWidget(new QLabel(tr("Ln. Stdev:")), row, col++);
  layout->addWidget(_lnStdevSpinBox, row, col++);

  // Minimum
  _minSpinBox = new QDoubleSpinBox;
  _minSpinBox->setDecimals(2);
  _minSpinBox->setSingleStep(0.01);

  layout->addWidget(new QLabel(tr("Min:")), row, col++);
  layout->addWidget(_minSpinBox, row, col++);

  // Maximum
  _maxSpinBox = new QDoubleSpinBox;
  _maxSpinBox->setDecimals(2);
  _maxSpinBox->setSingleStep(0.01);

  layout->addWidget(new QLabel(tr("Max:")), row, col++);
  layout->addWidget(_maxSpinBox, row, col++);
}

void NonlinearPropertyUncertaintyWidget::setModel(
    NonlinearPropertyUncertainty *model) {
  _lnStdevSpinBox->setValue(model->lnStdev());
  connect(_lnStdevSpinBox, SIGNAL(valueChanged(double)), model,
          SLOT(setLnStdev(double)));

  _minSpinBox->setValue(model->min());
  connect(_minSpinBox, SIGNAL(valueChanged(double)), model,
          SLOT(setMin(double)));

  _maxSpinBox->setValue(model->max());
  connect(_maxSpinBox, SIGNAL(valueChanged(double)), model,
          SLOT(setMax(double)));
}

void NonlinearPropertyUncertaintyWidget::setDecimals(int prec) {
  _minSpinBox->setDecimals(prec);
  _maxSpinBox->setDecimals(prec);
}

void NonlinearPropertyUncertaintyWidget::setLnStdevRange(double min,
                                                         double max) {
  _lnStdevSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setMinRange(double min, double max) {
  _minSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setMaxRange(double min, double max) {
  _maxSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setSuffix(const QString &suffix) {
  _minSpinBox->setSuffix(suffix);
  _maxSpinBox->setSuffix(suffix);
}

void NonlinearPropertyUncertaintyWidget::setReadOnly(bool readOnly) {
  _lnStdevSpinBox->setReadOnly(readOnly);
  _minSpinBox->setReadOnly(readOnly);
  _maxSpinBox->setReadOnly(readOnly);
}

void NonlinearPropertyUncertaintyWidget::setUncertaintyModel(int model) {
  _lnStdevSpinBox->setEnabled((NonlinearPropertyRandomizer::Model)model ==
                              NonlinearPropertyRandomizer::SPID);
}
