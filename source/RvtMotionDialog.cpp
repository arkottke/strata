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

#include "RvtMotionDialog.h"


#include "EditActions.h"
#include "MyQwtCompatibility.h"
#include "ResponseSpectrum.h"
#include "OnlyIncreasingDelegate.h"
#include "TableGroupBox.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>

RvtMotionDialog::RvtMotionDialog(RvtMotion *motion, bool readOnly, QWidget *parent) :
    AbstractRvtMotionDialog(motion, readOnly, parent)
{
    init();
}

auto RvtMotionDialog::createParametersLayout() -> QFormLayout*
{
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

    return layout;
}

auto RvtMotionDialog::createTabWidget() -> QTabWidget*
{
    auto tabWidget = AbstractRvtMotionDialog::createTabWidget();
    tabWidget->setCurrentIndex(3);

    return tabWidget;
}
