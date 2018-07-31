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

#include "FrequencyDependentCalculatorWidget.h"

#include "FrequencyDependentCalculator.h"

#include <QFormLayout>

FrequencyDependentCalculatorWidget::FrequencyDependentCalculatorWidget(QWidget *parent)
    : QWidget(parent)
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

    // Use smooth Strain FAS shape
    _useSmoothStrainCheckBox = new QCheckBox(tr("Use Kausel & Assimaki spectral shape"));
    layout->addRow(_useSmoothStrainCheckBox);

    setLayout(layout);
}

void FrequencyDependentCalculatorWidget::setCalculator(FrequencyDependentCalculator* fdc)
{
    _errorToleranceSpinBox->setValue(fdc->errorTolerance());
    connect(_errorToleranceSpinBox, SIGNAL(valueChanged(double)),
            fdc, SLOT(setErrorTolerance(double)));

    _maxIterationsSpinBox->setValue(fdc->maxIterations());
    connect(_maxIterationsSpinBox, SIGNAL(valueChanged(int)),
            fdc, SLOT(setMaxIterations(int)));

    _useSmoothStrainCheckBox->setChecked(fdc->useSmoothSpectrum());
    connect(_useSmoothStrainCheckBox, SIGNAL(toggled(bool)),
            fdc, SLOT(setUseSmoothSpectrum(bool)));
}

void FrequencyDependentCalculatorWidget::setReadOnly(bool readOnly)
{
    _errorToleranceSpinBox->setReadOnly(readOnly);
    _maxIterationsSpinBox->setReadOnly(readOnly);
    _useSmoothStrainCheckBox->setDisabled(readOnly);
}
