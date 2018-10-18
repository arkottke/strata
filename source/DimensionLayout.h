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

#ifndef DIMENSION_LAYOUT_H
#define DIMENSION_LAYOUT_H

#include <QFormLayout>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

class Dimension;

class DimensionLayout : public QFormLayout
{
    Q_OBJECT
public:
    explicit DimensionLayout(QWidget *parent = nullptr);

    void setModel(Dimension* dimension);
    void setSuffix(const QString &suffix);
    void setRange(double min, double max);
    void setSingleStep(double step);

public slots:
    void setReadOnly(bool readOnly);

protected slots:
    void updateMaxMin(double min);
    void updateMinMax(double max);
    void updateSpacing(int spacing);

protected:
    QDoubleSpinBox* _minSpinBox;
    QDoubleSpinBox* _maxSpinBox;
    QSpinBox* _sizeSpinBox;
    QComboBox* _spacingComboBox;
};

#endif // DIMENSION_LAYOUT_H
