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

#ifndef CONFINING_STRESS_DIALOG_H_
#define CONFINING_STRESS_DIALOG_H_

#include <QDialog>
#include <QDoubleSpinBox>

//! A dialog that aids in calculation of the confining stress.

class ConfiningStressDialog : public QDialog {
  Q_OBJECT

public:
  ConfiningStressDialog(QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

protected slots:
  void updateLabels();

protected:
  QDoubleSpinBox *_waterDepthSpinBox;
};
#endif
