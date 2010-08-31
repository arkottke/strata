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

#ifndef EQUIVALENT_LINEAR_CALCULATOR_WIDGET_H
#define EQUIVALENT_LINEAR_CALCULATOR_WIDGET_H

#include <QWidget>

#include <QDoubleSpinBox>
#include <QSpinBox>

class EquivalentLinearCalculator;

class EquivalentLinearCalculatorWidget : public QWidget
{
Q_OBJECT
public:
    explicit EquivalentLinearCalculatorWidget(QWidget *parent = 0);

public slots:
    void setCalculator(EquivalentLinearCalculator* elc);
    void setReadOnly(bool readOnly);

protected:
    QDoubleSpinBox* m_strainRatioSpinBox;
    QDoubleSpinBox* m_errorToleranceSpinBox;
    QSpinBox* m_maxIterationsSpinBox;
};

#endif // EQUIVALENT_LINEAR_CALCULATOR_WIDGET_H
