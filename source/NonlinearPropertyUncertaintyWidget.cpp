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
// Copyright 2010-2016 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "NonlinearPropertyUncertaintyWidget.h"

#include "NonlinearPropertyUncertainty.h"

#include <QDebug>
#include <QLabel>

NonlinearPropertyUncertaintyWidget::NonlinearPropertyUncertaintyWidget(const QString& title, QGridLayout *layout, QObject *parent) :
    QObject(parent)
{
    int row = layout->rowCount();
    int col = 0;
    layout->addWidget(new QLabel(title), row++, 0, 1, 2);

    // Ln. Stdev
    m_lnStdevSpinBox = new QDoubleSpinBox;
    m_lnStdevSpinBox->setDecimals(3);
    m_lnStdevSpinBox->setSingleStep(0.01);
    m_lnStdevSpinBox->setRange(0, 1.0);

    layout->addWidget(new QLabel(tr("Ln. Stdev:")), row, col++);
    layout->addWidget(m_lnStdevSpinBox, row, col++);

    // Minimum
    m_minSpinBox = new QDoubleSpinBox;
    m_minSpinBox->setDecimals(2);
    m_minSpinBox->setSingleStep(0.01);

    layout->addWidget(new QLabel(tr("Min:")), row, col++);
    layout->addWidget(m_minSpinBox, row, col++);

    // Maximum
    m_maxSpinBox = new QDoubleSpinBox;
    m_maxSpinBox->setDecimals(2);
    m_maxSpinBox->setSingleStep(0.01);

    layout->addWidget(new QLabel(tr("Max:")), row, col++);
    layout->addWidget(m_maxSpinBox, row, col++);
}

void NonlinearPropertyUncertaintyWidget::setModel(NonlinearPropertyUncertainty* model)
{
    m_lnStdevSpinBox->setValue(model->lnStdev());
    connect(m_lnStdevSpinBox, SIGNAL(valueChanged(double)),
            model, SLOT(setLnStdev(double)));

    m_minSpinBox->setValue(model->min());
    connect(m_minSpinBox, SIGNAL(valueChanged(double)),
            model, SLOT(setMin(double)));

    m_maxSpinBox->setValue(model->max());
    connect(m_maxSpinBox, SIGNAL(valueChanged(double)),
            model, SLOT(setMax(double)));
}

void NonlinearPropertyUncertaintyWidget::setDecimals(int prec)
{
    m_minSpinBox->setDecimals(prec);
    m_maxSpinBox->setDecimals(prec);
}

void NonlinearPropertyUncertaintyWidget::setLnStdevRange(double min, double max)
{
    m_lnStdevSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setMinRange(double min, double max)
{
    m_minSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setMaxRange(double min, double max)
{
    m_maxSpinBox->setRange(min, max);
}

void NonlinearPropertyUncertaintyWidget::setSuffix(const QString& suffix)
{
    m_minSpinBox->setSuffix(suffix);
    m_maxSpinBox->setSuffix(suffix);
}

void NonlinearPropertyUncertaintyWidget::setReadOnly(bool readOnly)
{
    m_lnStdevSpinBox->setReadOnly(readOnly);
    m_minSpinBox->setReadOnly(readOnly);
    m_maxSpinBox->setReadOnly(readOnly);
}

void NonlinearPropertyUncertaintyWidget::setUncertaintyModel(int model)
{
    m_lnStdevSpinBox->setEnabled(
                (NonlinearPropertyRandomizer::Model)model == NonlinearPropertyRandomizer::SPID);
}
