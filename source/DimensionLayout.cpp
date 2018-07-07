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

#include "DimensionLayout.h"

#include "Dimension.h"

#include <QDebug>

DimensionLayout::DimensionLayout(QWidget *parent) :
    QFormLayout(parent)
{
    // Minimum
    _minSpinBox = new QDoubleSpinBox;
    _minSpinBox->setDecimals(3);
    _minSpinBox->setSingleStep(0.01);
    connect(_minSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(updateMaxMin(double)));

    addRow(tr("Minimum:"), _minSpinBox);

    // Maximum
    _maxSpinBox = new QDoubleSpinBox;
    _maxSpinBox->setMaximum(300.);
    _maxSpinBox->setDecimals(3);
    _maxSpinBox->setSingleStep(1);
    connect(_maxSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(updateMinMax(double)));

    addRow(tr("Maximum:"), _maxSpinBox);

    // Point count
    _sizeSpinBox = new QSpinBox;
    _sizeSpinBox->setRange(16, 16384);

    addRow(tr("Point count:"), _sizeSpinBox);

    // Spacing
    _spacingComboBox = new QComboBox;
    _spacingComboBox->addItems(Dimension::spacingList());

    connect(_spacingComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateSpacing(int)));

    addRow(tr("Spacing:"), _spacingComboBox);
}

void DimensionLayout::setModel(Dimension *dimension)
{
    _minSpinBox->setValue(dimension->min());
    connect(_minSpinBox, SIGNAL(valueChanged(double)),
            dimension, SLOT(setMin(double)));

    _maxSpinBox->setValue(dimension->max());
    connect(_maxSpinBox, SIGNAL(valueChanged(double)),
            dimension, SLOT(setMax(double)));

    _sizeSpinBox->setValue(dimension->size());
    connect(_sizeSpinBox, SIGNAL(valueChanged(int)),
            dimension, SLOT(setSize(int)));

    _spacingComboBox->setCurrentIndex(dimension->spacing());
    connect(_spacingComboBox, SIGNAL(currentIndexChanged(int)),
            dimension, SLOT(setSpacing(int)));
}

void DimensionLayout::setSuffix(const QString &suffix)
{
    _minSpinBox->setSuffix(suffix);
    _maxSpinBox->setSuffix(suffix);
}

void DimensionLayout::setRange(double min, double max)
{
    // Need to block the signals to prevent the values from getting changed,
    // which causes the min-max and max-min to be updated.
    _minSpinBox->blockSignals(true);
    _minSpinBox->setMinimum(min);
    _minSpinBox->blockSignals(false);

    _maxSpinBox->blockSignals(true);
    _maxSpinBox->setMaximum(max);
    _maxSpinBox->blockSignals(false);
}

void DimensionLayout::setSingleStep(double step)
{
    _minSpinBox->setSingleStep(step);
    _maxSpinBox->setSingleStep(step);
}

void DimensionLayout::setReadOnly(bool readOnly)
{
    _minSpinBox->setReadOnly(readOnly);
    _maxSpinBox->setReadOnly(readOnly);
    _sizeSpinBox->setReadOnly(readOnly);
    _spacingComboBox->setDisabled(readOnly);
}

void DimensionLayout::updateMaxMin(double min)
{
    _maxSpinBox->setMinimum(min);
}

void DimensionLayout::updateMinMax(double max)
{
    _minSpinBox->setMaximum(max);
}

void DimensionLayout::updateSpacing(int spacing)
{
    if (Dimension::Log == (Dimension::Spacing)spacing) {
        _minSpinBox->setMinimum(0.001);
    } else {
        _minSpinBox->setMinimum(0.);
    }
}
