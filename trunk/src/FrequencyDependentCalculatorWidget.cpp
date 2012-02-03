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
// Copyright 2010 Albert Kottke
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
    m_errorToleranceSpinBox = new QDoubleSpinBox;
    m_errorToleranceSpinBox->setRange( 0.5, 10.0);
    m_errorToleranceSpinBox->setDecimals(1);
    m_errorToleranceSpinBox->setSingleStep(0.25);
    m_errorToleranceSpinBox->setSuffix(" %");

    layout->addRow(tr("Error tolerance:"), m_errorToleranceSpinBox);

    // Max Iterations row
    m_maxIterationsSpinBox = new QSpinBox;
    m_maxIterationsSpinBox->setMinimum(2);
    m_maxIterationsSpinBox->setMaximum(30);

    layout->addRow(tr("Maximum number of iterations:"), m_maxIterationsSpinBox);

    setLayout(layout);
}

void FrequencyDependentCalculatorWidget::setCalculator(FrequencyDependentCalculator* fdc)
{
    m_errorToleranceSpinBox->setValue(fdc->errorTolerance());
    connect(m_errorToleranceSpinBox, SIGNAL(valueChanged(double)),
            fdc, SLOT(setErrorTolerance(double)));

    m_maxIterationsSpinBox->setValue(fdc->maxIterations());
    connect(m_maxIterationsSpinBox, SIGNAL(valueChanged(int)),
            fdc, SLOT(setMaxIterations(int)));
}

void FrequencyDependentCalculatorWidget::setReadOnly(bool readOnly)
{
    m_errorToleranceSpinBox->setReadOnly(readOnly);
    m_maxIterationsSpinBox->setReadOnly(readOnly);
}
