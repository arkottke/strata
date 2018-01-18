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

#ifndef RVT_MOTION_DIALOG_H
#define RVT_MOTION_DIALOG_H

#include "RvtMotion.h"
#include "MyTableView.h"

#include <QDialog>

#include <qwt_plot_curve.h>

class RvtMotionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RvtMotionDialog(RvtMotion *motion, bool readOnly, QWidget *parent = 0);

signals:

public slots:

private slots:
    void calculate();
    void tryAccept();

private:
    RvtMotion *_motion;

    QwtPlotCurve *_fasCurve;
    QwtPlotCurve *_saCurve;

    MyTableView *_rsTableView;
};

#endif // RVT_MOTION_DIALOG_H
