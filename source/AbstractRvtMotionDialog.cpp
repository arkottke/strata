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

#include "DimensionLayout.h"
#include "EditActions.h"
#include "MyQwtCompatibility.h"
#include "OnlyIncreasingDelegate.h"
#include "ResponseSpectrum.h"
#include "TableGroupBox.h"
#include "CompatibleRvtMotion.h"
#include "SourceTheoryRvtMotion.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTabWidget>

#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_text.h>

AbstractRvtMotionDialog::AbstractRvtMotionDialog(
        AbstractRvtMotion *motion, bool readOnly, QWidget *parent) :
    QDialog(parent), _readOnly(readOnly), _motion(motion)
{}

void AbstractRvtMotionDialog::init()
{
    // Buttons
    auto buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
            Qt::Horizontal);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(calculate()));

    if (!qobject_cast<RvtMotion*>(_motion)) {
        auto pushButton = new QPushButton(tr("Frequency Parameters..."));
        connect(pushButton, SIGNAL(clicked()), this, SLOT(openFrequencyDialog()));
        buttonBox->addButton(pushButton, QDialogButtonBox::ActionRole);
    }

    auto layout = new QGridLayout;
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(1, 1);

    layout->addLayout(createParametersLayout(), 0, 0);
    layout->addWidget(createTabWidget(), 0, 1);
    layout->addWidget(buttonBox, 1, 0, 1, 2);

    setLayout(layout);

    // Add copy and paste actions
    addAction(EditActions::instance()->copyAction());
    addAction(EditActions::instance()->pasteAction());
}

QFormLayout* AbstractRvtMotionDialog::createParametersLayout()
{
    auto layout = new QFormLayout;
    
    // Name
    auto lineEdit = new QLineEdit;
    lineEdit->setText(_motion->nameTemplate());
    lineEdit->setReadOnly(_readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            _motion, SLOT(setName(QString)));
    
    layout->addRow(tr("Name:"), lineEdit);

    // Description
    lineEdit = new QLineEdit;
    lineEdit->setText(_motion->description());
    lineEdit->setReadOnly(_readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            _motion, SLOT(setDescription(QString)));

    layout->addRow(tr("Description:"), lineEdit);

    // Region
    auto comboBox = new QComboBox;
    comboBox->addItems(AbstractRvtMotion::regionList());
    comboBox->setCurrentIndex(
                static_cast<int>(_motion->region()));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            _motion, SLOT(setRegion(int)));

    layout->addRow(tr("Region:"), comboBox);

    // Magnitude
    auto spinBox = new QDoubleSpinBox;
    spinBox->setRange(4, 9);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.1);
    spinBox->setReadOnly(_readOnly);

    spinBox->setValue(_motion->magnitude());
    connect(spinBox, SIGNAL(valueChanged(double)),
            _motion, SLOT(setMagnitude(double)));

    layout->addRow(tr("Magnitude (<b>M</b>):"), spinBox);

    // Distance
    spinBox = new QDoubleSpinBox;
    spinBox->setRange(0, 2000);
    spinBox->setDecimals(1);
    spinBox->setSingleStep(1);
    spinBox->setSuffix(" km");
    spinBox->setReadOnly(_readOnly);

    spinBox->setValue(_motion->distance());
    connect(spinBox, SIGNAL(valueChanged(double)),
            _motion, SLOT(setDistance(double)));

    layout->addRow(tr("Epicentral distance:"), spinBox);

    // Type
    comboBox = new QComboBox;
    comboBox->addItems(AbstractMotion::typeList());
    comboBox->setCurrentIndex(_motion->type());
    // Only permit setting type for RvtMotions
    comboBox->setDisabled(_readOnly || !qobject_cast<RvtMotion*>(_motion));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            _motion, SLOT(setType(int)));

    layout->addRow(tr("Motion type:"), comboBox);

    return layout;
}

QTabWidget* AbstractRvtMotionDialog::createTabWidget()
{
    _dataTabWidget = new QTabWidget;
    _dataTabWidget->addTab(createRSPlotWidget(), tr("RS Plot"));
    
    // Response spectrum table
    _rsTableView = new MyTableView;
    _rsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _rsTableView->setModel(_motion->respSpec());
    _rsTableView->setReadOnly(true);
    _dataTabWidget->addTab(_rsTableView, tr("RS Data"));

    _dataTabWidget->addTab(createFSPlotWidget(), tr("FAS Plot"));

    // Fourier amplitude spectrum table
    _fsTableView = new MyTableView;

    if (auto rm = qobject_cast<RvtMotion*>(_motion)) {
        _fsTableView->setModel(rm);
    } else {
        _fsTableView->setModel(_motion);
        _fsTableView->setReadOnly(true);
    }
    _dataTabWidget->addTab(_fsTableView, tr("FAS Data"));

    return _dataTabWidget;
}

QWidget* AbstractRvtMotionDialog::createRSPlotWidget()
{
    // Response spectrum plot
    _rsPlot = new QwtPlot;
    _rsPlot->setAutoReplot(true);
    auto picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, _rsPlot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    _rsPlot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());

    QFont font = QApplication::font();
    _rsPlot->setAxisFont(QwtPlot::xBottom, font);

    _rsPlot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    _rsPlot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text = QwtText(tr("Period (s)"));
    text.setFont(font);
    _rsPlot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Spectral Accel. (g)"));
    _rsPlot->setAxisTitle(QwtPlot::yLeft, text);

    _saCurve = new QwtPlotCurve(tr("Calculated"));
    _saCurve->setPen(QPen(Qt::blue, 2));
    _saCurve->setSamples(_motion->respSpec()->period(),
                       _motion->respSpec()->sa());
    _saCurve->attach(_rsPlot);

    addRespSpecCurves();

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(_rsPlot);

    return scrollArea;
}

void AbstractRvtMotionDialog::addRespSpecCurves()
{
    // Do nothing.
}

QWidget* AbstractRvtMotionDialog::createFSPlotWidget()
{
    // Fourier amplitude spectrum plot
    _fsPlot = new QwtPlot;
    _fsPlot->setAutoReplot(true);
    auto picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPlotPicker::CrossRubberBand,
                               QwtPlotPicker::ActiveOnly, _fsPlot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    _fsPlot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());
    QFont font = QApplication::font();
    _fsPlot->setAxisFont(QwtPlot::xBottom, font);

    _fsPlot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    _fsPlot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text(tr("Frequency (Hz)"));
    text.setFont(font);
    _fsPlot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Fourier Amplitude (g-s)"));
    _fsPlot->setAxisTitle(QwtPlot::yLeft, text);

    _faCurve = new QwtPlotCurve;
    _faCurve->setPen(QPen(Qt::blue, 2));
    _faCurve->setSamples(_motion->freq(),_motion->fourierAcc());
    _faCurve->attach(_fsPlot);

    auto scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(_fsPlot);

    return scrollArea;
}

void AbstractRvtMotionDialog::openFrequencyDialog()
{
    Dimension* freq;

    if (auto m = qobject_cast<CompatibleRvtMotion*>(_motion)) {
        freq = m->freqDimension(); 
    } else if (auto m = qobject_cast<SourceTheoryRvtMotion*>(_motion)) {
        freq = m->freqDimension(); 
    } else {
        return; 
    }

    QDialog dialog(this);
    auto layout = new DimensionLayout;
    layout->setRange(0.001, 1000);
    layout->setModel(freq);
    layout->setSuffix(" Hz");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok, Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()),
            &dialog, SLOT(accept()));

    layout->addRow(buttonBox);

    dialog.setLayout(layout);
    dialog.exec();
}

void AbstractRvtMotionDialog::calculate()
{
    _motion->calculate();

    _fsTableView->resizeRowsToContents();
    _rsTableView->resizeRowsToContents();

    _faCurve->setSamples(_motion->freq(),_motion->fourierAcc());
    _saCurve->setSamples(_motion->respSpec()->period(), _motion->respSpec()->sa());

    _dataTabWidget->setCurrentIndex(0);
    // FIXME 
    // _targetSaCurve->setSamples(_motion->targetRespSpec()->period(),
    //                           _motion->targetRespSpec()->sa());
}

void AbstractRvtMotionDialog::tryAccept()
{
    _motion->calculate();
    accept();
}
