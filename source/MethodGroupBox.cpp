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

#include "MethodGroupBox.h"

#include "AbstractCalculator.h"
#include "EquivalentLinearCalculator.h"
#include "EquivalentLinearCalculatorWidget.h"
#include "FrequencyDependentCalculator.h"
#include "FrequencyDependentCalculatorWidget.h"
#include "LinearElasticCalculator.h"

#include <QDebug>

MethodGroupBox::MethodGroupBox(QWidget* parent)
    : QGroupBox(parent)
{    
    m_stackedLayout = new QStackedLayout;

    m_elcWidget = new EquivalentLinearCalculatorWidget;
    m_stackedLayout->addWidget(m_elcWidget);

    m_fdcWidget = new FrequencyDependentCalculatorWidget;
    m_stackedLayout->addWidget(m_fdcWidget);

    setTitle("Calculation Parameters");
    setLayout(m_stackedLayout);    
}

void MethodGroupBox::setCalculator(AbstractCalculator* ac)
{
    const QString className = ac->metaObject()->className();

    setDisabled(className == "LinearElasticCalculator");

    if (className == "EquivalentLinearCalculator") {
        m_elcWidget->setCalculator(qobject_cast<EquivalentLinearCalculator*>(ac));
        m_stackedLayout->setCurrentIndex(0);
    } else if (className == "FrequencyDependentCalculator") {
        m_fdcWidget->setCalculator(qobject_cast<FrequencyDependentCalculator*>(ac));
        m_stackedLayout->setCurrentIndex(1);
    }
}

void MethodGroupBox::setReadOnly(bool readOnly)
{
    m_elcWidget->setReadOnly(readOnly);
    m_fdcWidget->setReadOnly(readOnly);
}
