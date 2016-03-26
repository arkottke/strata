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

#include "NonlinearPropertyStandardDeviationWidget.h"

#include "AbstractNonlinearPropertyStandardDeviation.h"

#include <QDebug>
#include <QLabel>

NonlinearPropertyStandardDeviationWidget::NonlinearPropertyStandardDeviationWidget(const QString& title, QGridLayout *layout, QObject *parent) :
    QObject(parent)
{
    int row = layout->rowCount();

    layout->addWidget(new QLabel(title), row++, 0, 1, 2);

    // Standard deviation
    QLabel* label = new QLabel(tr("Function:"));
    label->setIndent(30);
    m_functionLineEdit = new QLineEdit;

    layout->addWidget(label, row, 0);
    layout->addWidget(m_functionLineEdit, row, 1, 1, 3);

    // Minimum
    m_minSpinBox = new QDoubleSpinBox;
    m_minSpinBox->setDecimals(2);
    m_minSpinBox->setSingleStep(0.01);

    layout->addWidget(new QLabel(tr("Min:")), row, 4);
    layout->addWidget(m_minSpinBox, row, 5);

    // Maximum
    m_maxSpinBox = new QDoubleSpinBox;
    m_maxSpinBox->setDecimals(2);
    m_maxSpinBox->setSingleStep(0.01);

    layout->addWidget(new QLabel(tr("Max:")), row, 6);
    layout->addWidget(m_maxSpinBox, row, 7);
}

void NonlinearPropertyStandardDeviationWidget::setModel(AbstractNonlinearPropertyStandardDeviation* model)
{
    m_functionLineEdit->setText(model->function());
    connect(m_functionLineEdit, SIGNAL(textChanged(QString)),
            model, SLOT(setFunction(QString)));
    connect(model, SIGNAL(functionChanged(QString)),
            m_functionLineEdit, SLOT(setText(QString)));

    m_minSpinBox->setValue(model->min());
    connect(m_minSpinBox, SIGNAL(valueChanged(double)),
            model, SLOT(setMin(double)));

    m_maxSpinBox->setValue(model->max());
    connect(m_maxSpinBox, SIGNAL(valueChanged(double)),
            model, SLOT(setMax(double)));
}

void NonlinearPropertyStandardDeviationWidget::setDecimals(int prec)
{
    m_minSpinBox->setDecimals(prec);
    m_maxSpinBox->setDecimals(prec);
}

void NonlinearPropertyStandardDeviationWidget::setMinRange(double min, double max)
{
    m_minSpinBox->setRange(min, max);
}

void NonlinearPropertyStandardDeviationWidget::setMaxRange(double min, double max)
{
    m_maxSpinBox->setRange(min, max);
}

void NonlinearPropertyStandardDeviationWidget::setSuffix(const QString& suffix)
{
    m_minSpinBox->setSuffix(suffix);
    m_maxSpinBox->setSuffix(suffix);
}

void NonlinearPropertyStandardDeviationWidget::setCustomEnabled(bool enabled)
{
    m_functionLineEdit->setReadOnly(!enabled);
}

void NonlinearPropertyStandardDeviationWidget::setReadOnly(bool readOnly)
{
    m_functionLineEdit->setReadOnly(readOnly);
    m_minSpinBox->setReadOnly(readOnly);
    m_maxSpinBox->setReadOnly(readOnly);
}
