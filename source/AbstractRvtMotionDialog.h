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

#ifndef ABSTRACT_RVT_MOTION_DIALOG_H
#define ABSTRACT_RVT_MOTION_DIALOG_H

#include "AbstractRvtMotion.h"
#include "MyTableView.h"

#include <QDialog>
#include <QFormLayout>
#include <QTabWidget>

#include <qwt_plot_curve.h>

class AbstractRvtMotionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AbstractRvtMotionDialog(
            AbstractRvtMotion *motion, bool readOnly, QWidget *parent = nullptr);

private slots:
    void calculate();
    void tryAccept();

    void openFrequencyDialog();

protected:
    void init(bool readOnly);
    virtual QFormLayout* createParametersLayout(bool readOnly); 
    virtual QTabWidget* createTabWidget(bool readOnly); 
    virtual QWidget* createRSPlotWidget();
    virtual QWidget* createFSPlotWidget();

    AbstractRvtMotion *_motion;

    QwtPlot* _rsPlot;
    QwtPlotCurve *_saCurve;

    MyTableView *_rsTableView;

    QwtPlot* _fsPlot;
    QwtPlotCurve *_faCurve;

    MyTableView *_fsTableView;
};

#endif // ABSTRACT_RVT_MOTION_DIALOG_H
