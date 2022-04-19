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

#include "MethodGroupBox.h"

#include "AbstractCalculator.h"
#include "EquivalentLinearCalculator.h"
#include "EquivalentLinearCalculatorWidget.h"
#include "FrequencyDependentCalculator.h"
#include "FrequencyDependentCalculatorWidget.h"
#include "LinearElasticCalculator.h"

#include <QDebug>

MethodGroupBox::MethodGroupBox(QWidget *parent) : QGroupBox(parent) {
  _stackedLayout = new QStackedLayout;

  _elcWidget = new EquivalentLinearCalculatorWidget;
  _stackedLayout->addWidget(_elcWidget);

  _fdcWidget = new FrequencyDependentCalculatorWidget;
  _stackedLayout->addWidget(_fdcWidget);

  setTitle("Calculation Parameters");
  setLayout(_stackedLayout);
}

void MethodGroupBox::setCalculator(AbstractCalculator *ac) {
  const QString className = ac->metaObject()->className();

  setDisabled(className == "LinearElasticCalculator");

  if (className == "EquivalentLinearCalculator") {
    _elcWidget->setCalculator(qobject_cast<EquivalentLinearCalculator *>(ac));
    _stackedLayout->setCurrentIndex(0);
  } else if (className == "FrequencyDependentCalculator") {
    _fdcWidget->setCalculator(qobject_cast<FrequencyDependentCalculator *>(ac));
    _stackedLayout->setCurrentIndex(1);
  }
}

void MethodGroupBox::setReadOnly(bool readOnly) {
  _elcWidget->setReadOnly(readOnly);
  _fdcWidget->setReadOnly(readOnly);
}
