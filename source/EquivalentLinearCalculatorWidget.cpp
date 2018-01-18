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

#include "EquivalentLinearCalculatorWidget.h"

#include "EquivalentLinearCalculator.h"

#include <QFormLayout>

EquivalentLinearCalculatorWidget::EquivalentLinearCalculatorWidget(QWidget *parent) :
    QWidget(parent)
{
    QFormLayout* layout = new QFormLayout;

    // Error tolerance row
    _errorToleranceSpinBox = new QDoubleSpinBox;
    _errorToleranceSpinBox->setRange( 0.5, 10.0);
    _errorToleranceSpinBox->setDecimals(1);
    _errorToleranceSpinBox->setSingleStep(0.25);
    _errorToleranceSpinBox->setSuffix(" %");

    layout->addRow(tr("Error tolerance:"), _errorToleranceSpinBox);

    // Max Iterations row
    _maxIterationsSpinBox = new QSpinBox;
    _maxIterationsSpinBox->setMinimum(2);
    _maxIterationsSpinBox->setMaximum(30);

    layout->addRow(tr("Maximum number of iterations:"), _maxIterationsSpinBox);

    // Effective Strain Ratio
    _strainRatioSpinBox = new QDoubleSpinBox;
    _strainRatioSpinBox->setRange( 0.45, 0.80);
    _strainRatioSpinBox->setSingleStep(0.05);
    _strainRatioSpinBox->setDecimals(2);

    layout->addRow("Effective strain ratio:", _strainRatioSpinBox);

    setLayout(layout);
}

void EquivalentLinearCalculatorWidget::setCalculator(EquivalentLinearCalculator* elc)
{
    _strainRatioSpinBox->setValue(elc->strainRatio());
    connect(_strainRatioSpinBox, SIGNAL(valueChanged(double)),
            elc, SLOT(setStrainRatio(double)));

    _errorToleranceSpinBox->setValue(elc->errorTolerance());
    connect(_errorToleranceSpinBox, SIGNAL(valueChanged(double)),
            elc, SLOT(setErrorTolerance(double)));

    _maxIterationsSpinBox->setValue(elc->maxIterations());
    connect(_maxIterationsSpinBox, SIGNAL(valueChanged(int)),
             elc, SLOT(setMaxIterations(int)));
}

void EquivalentLinearCalculatorWidget::setReadOnly(bool readOnly)
{
    _strainRatioSpinBox->setReadOnly(readOnly);
    _errorToleranceSpinBox->setReadOnly(readOnly);
    _maxIterationsSpinBox->setReadOnly(readOnly);
}
