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

#include "CompatibleRvtMotionDialog.h"

#include "CompatibleRvtMotion.h"
#include "EditActions.h"
#include "MyQwtCompatibility.h"
#include "MyTableView.h"
#include "OnlyIncreasingDelegate.h"
#include "ResponseSpectrum.h"
#include "TableGroupBox.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

#include <qwt_picker_machine.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

CompatibleRvtMotionDialog::CompatibleRvtMotionDialog(
        CompatibleRvtMotion *motion, 
        bool readOnly, 
        QWidget *parent) :
    AbstractRvtMotionDialog(motion, readOnly, parent)
{
    init();
}

QFormLayout* CompatibleRvtMotionDialog::createParametersLayout()
{
    auto crm = qobject_cast<CompatibleRvtMotion*>(_motion);

    auto layout = AbstractRvtMotionDialog::createParametersLayout();

    // Duration
    auto spinBox = new QDoubleSpinBox;
    spinBox->setRange(0.10, 100.00);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.10);
    spinBox->setSuffix(" s");
    spinBox->setValue(_motion->duration());
    spinBox->setReadOnly(_readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            _motion, SLOT(setDuration(double)));

    layout->addRow(tr("Duration:"), spinBox);

    // Damping
    spinBox = new QDoubleSpinBox;
    spinBox->setRange(1, 20);
    spinBox->setDecimals(1);
    spinBox->setSingleStep(1);
    spinBox->setSuffix(" %");
    spinBox->setValue(crm->targetRespSpec()->damping());
    spinBox->setReadOnly(_readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            crm->targetRespSpec(), SLOT(setDamping(double)));

    layout->addRow(tr("Damping of target:"), spinBox);

    // Limit
    auto checkBox = new QCheckBox(tr("Limit shape of FAS"));
    checkBox->setChecked(crm->limitFas());
    connect(checkBox, SIGNAL(toggled(bool)), _motion, SLOT(setLimitFas(bool)));

    layout->addRow(checkBox);

    return layout;
}

void CompatibleRvtMotionDialog::calculate()
{
    AbstractRvtMotionDialog::calculate();

    auto crm = qobject_cast<CompatibleRvtMotion*>(_motion);
    _targetSaCurve->setSamples(
            crm->targetRespSpec()->period(),
            crm->targetRespSpec()->sa());

    _dataTabWidget->setCurrentIndex(1);
}

QTabWidget* CompatibleRvtMotionDialog::createTabWidget()
{
    auto tabWidget = AbstractRvtMotionDialog::createTabWidget();
    auto crm = qobject_cast<CompatibleRvtMotion*>(_motion);

    // Target response specturm
    auto tableGroupBox = new TableGroupBox(tr("Target Response Spectrum"), this);
    tableGroupBox->setModel(crm->targetRespSpec());
    tableGroupBox->setReadOnly(_readOnly);
    tableGroupBox->table()->setItemDelegateForColumn(0, new OnlyIncreasingDelegate);
    
    auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(tableGroupBox);

    tabWidget->insertTab(0, scrollArea, "Target RS");
    tabWidget->setCurrentIndex(0);

    return tabWidget;
}

void CompatibleRvtMotionDialog::addRespSpecCurves()
{
    auto crm = qobject_cast<CompatibleRvtMotion*>(_motion);

    _targetSaCurve = new QwtPlotCurve(tr("Target"));
    _targetSaCurve->setPen(Qt::red, 1., Qt::DashLine);
    _targetSaCurve->setSamples(crm->targetRespSpec()->period(), crm->targetRespSpec()->sa());
    _targetSaCurve->attach(_rsPlot);

    auto *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    _rsPlot->insertLegend(legend, QwtPlot::BottomLegend);
}
