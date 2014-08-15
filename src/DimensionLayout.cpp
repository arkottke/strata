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

#include "DimensionLayout.h"

#include "Dimension.h"

#include <QDebug>

DimensionLayout::DimensionLayout(QWidget *parent) :
    QFormLayout(parent)
{
    // Minimum
    m_minSpinBox = new QDoubleSpinBox;
    connect(m_minSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(updateMin(double)));

    addRow(tr("Minimum:"), m_minSpinBox);

    // Maximum
    m_maxSpinBox = new QDoubleSpinBox;
    m_maxSpinBox->setMaximum(300.);
    connect(m_maxSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(updateMax(double)));

    addRow(tr("Maximum:"), m_maxSpinBox);

    // Point count
    m_sizeSpinBox = new QSpinBox;
    m_sizeSpinBox->setRange(16, 16384);

    addRow(tr("Point count:"), m_sizeSpinBox);

    // Spacing
    m_spacingComboBox = new QComboBox;
    m_spacingComboBox->addItems(Dimension::spacingList());

    connect(m_spacingComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateSpacing(int)));

    addRow(tr("Spacing:"), m_spacingComboBox);
}

void DimensionLayout::setModel(Dimension *dimension)
{
    m_minSpinBox->setValue(dimension->min());
    connect(m_minSpinBox, SIGNAL(valueChanged(double)),
            dimension, SLOT(setMin(double)));

    m_maxSpinBox->setValue(dimension->max());
    connect(m_maxSpinBox, SIGNAL(valueChanged(double)),
            dimension, SLOT(setMax(double)));

    m_sizeSpinBox->setValue(dimension->size());
    connect(m_sizeSpinBox, SIGNAL(valueChanged(int)),
            dimension, SLOT(setSize(int)));

    m_spacingComboBox->setCurrentIndex(dimension->spacing());
    connect(m_spacingComboBox, SIGNAL(currentIndexChanged(int)),
            dimension, SLOT(setSpacing(int)));
}

void DimensionLayout::setSuffix(const QString &suffix)
{
    m_minSpinBox->setSuffix(suffix);
    m_maxSpinBox->setSuffix(suffix);
}

void DimensionLayout::setRange(double min, double max)
{
    m_minSpinBox->setMinimum(min);
    m_maxSpinBox->setMaximum(max);
}

void DimensionLayout::setSingleStep(double step)
{
    m_minSpinBox->setSingleStep(step);
    m_maxSpinBox->setSingleStep(step);
}

void DimensionLayout::setReadOnly(bool readOnly)
{
    m_minSpinBox->setReadOnly(readOnly);
    m_maxSpinBox->setReadOnly(readOnly);
    m_sizeSpinBox->setReadOnly(readOnly);
    m_spacingComboBox->setDisabled(readOnly);
}

void DimensionLayout::updateMin(double min)
{
    m_maxSpinBox->setMinimum(min);
}

void DimensionLayout::updateMax(double max)
{
    m_minSpinBox->setMaximum(max);
}

void DimensionLayout::updateSpacing(int spacing)
{
    if (Dimension::Log == (Dimension::Spacing)spacing) {
        m_minSpinBox->setMinimum(0.001);
    } else {
        m_minSpinBox->setMinimum(0.);
    }
}
