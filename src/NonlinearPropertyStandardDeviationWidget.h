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

#ifndef NONLINEAR_PROPERTY_STANDARD_DEVIATION_WIDGET_H
#define NONLINEAR_PROPERTY_STANDARD_DEVIATION_WIDGET_H

#include <QObject>

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLineEdit>

class AbstractNonlinearPropertyStandardDeviation;

class NonlinearPropertyStandardDeviationWidget : public QObject
{
    Q_OBJECT
public:
    explicit NonlinearPropertyStandardDeviationWidget(const QString& title, QGridLayout *layout, QObject *parent);

    void setModel(AbstractNonlinearPropertyStandardDeviation* model);

    void setDecimals(int prec);
    void setMinRange(double min, double max);
    void setMaxRange(double min, double max);
    void setSuffix(const QString& suffix);

public slots:
    void setCustomEnabled(bool enabled);
    void setReadOnly(bool readOnly);

protected:
    QLineEdit* m_functionLineEdit;
    QDoubleSpinBox* m_minSpinBox;
    QDoubleSpinBox* m_maxSpinBox;
};

#endif // NONLINEAR_PROPERTY_STANDARD_DEVIATION_WIDGET_H
