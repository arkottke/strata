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

#ifndef NONLINEAR_PROPERTY_UNCERTAINTY_WIDGET_H
#define NONLINEAR_PROPERTY_UNCERTAINTY_WIDGET_H

#include <QObject>

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLineEdit>

class NonlinearPropertyUncertainty;

class NonlinearPropertyUncertaintyWidget : public QObject {
  Q_OBJECT
public:
  explicit NonlinearPropertyUncertaintyWidget(const QString &title,
                                              QGridLayout *layout,
                                              QObject *parent);

  void setDecimals(int prec);
  void setLnStdevRange(double min, double max);
  void setMinRange(double min, double max);
  void setMaxRange(double min, double max);
  void setSuffix(const QString &suffix);
  void setModel(NonlinearPropertyUncertainty *model);

public slots:
  void setReadOnly(bool readOnly);
  void setUncertaintyModel(int model);

protected:
  QDoubleSpinBox *_lnStdevSpinBox;
  QDoubleSpinBox *_minSpinBox;
  QDoubleSpinBox *_maxSpinBox;
};

#endif // NONLINEAR_PROPERTY_UNCERTAINTY_WIDGET_H
