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

#ifndef COMPATIBLE_RVT_MOTION_DIALOG_H
#define COMPATIBLE_RVT_MOTION_DIALOG_H

#include <QDialog>

#include <qwt/qwt_plot_curve.h>

class MyTableView;
class CompatibleRvtMotion;

class CompatibleRvtMotionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CompatibleRvtMotionDialog(CompatibleRvtMotion *motion, bool readOnly, QWidget *parent = 0);

signals:

public slots:
    void calculate();
    void tryAccept();

protected slots:
    void openFrequencyDialog();

private:

    CompatibleRvtMotion *m_motion;

    QwtPlotCurve *m_fasCurve;
    QwtPlotCurve *m_saCurve;
    QwtPlotCurve *m_targetSaCurve;

    MyTableView *m_fasTableView;
    MyTableView *m_rsTableView;
};

#endif // COMPATIBLE_RVT_MOTION_DIALOG_H
