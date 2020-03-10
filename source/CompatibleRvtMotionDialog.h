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

#ifndef COMPATIBLE_RVT_MOTION_DIALOG_H
#define COMPATIBLE_RVT_MOTION_DIALOG_H

#include "AbstractRvtMotionDialog.h"

class MyTableView;
class CompatibleRvtMotion;

class CompatibleRvtMotionDialog : public AbstractRvtMotionDialog
{
Q_OBJECT
public:
    explicit CompatibleRvtMotionDialog(CompatibleRvtMotion *motion, bool readOnly,
                                       QWidget *parent = nullptr);

protected slots:
    void calculate();

protected:
    virtual auto createParametersLayout() -> QFormLayout*;
    virtual auto createTabWidget() -> QTabWidget*;
    virtual void addRespSpecCurves();

    MyTableView *_targetSaTableView;
    QwtPlotCurve *_targetSaCurve;
};

#endif // COMPATIBLE_RVT_MOTION_DIALOG_H
