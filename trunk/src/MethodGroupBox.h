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

#ifndef METHOD_GROUP_BOX_H
#define METHOD_GROUP_BOX_H

#include <QGroupBox>

#include <QStackedLayout>

class AbstractCalculator;
class EquivalentLinearCalculatorWidget;
class FrequencyDependentCalculatorWidget;

class MethodGroupBox : public QGroupBox
{
Q_OBJECT
public:
    explicit MethodGroupBox(QWidget* parent = 0);

signals:

public slots:
    void setCalculator(AbstractCalculator* ac);
    void setReadOnly(bool readOnly);

protected:
    QStackedLayout* m_stackedLayout;
    EquivalentLinearCalculatorWidget* m_elcWidget;
    FrequencyDependentCalculatorWidget* m_fdcWidget;
};

#endif // METHOD_GROUP_BOX_H
