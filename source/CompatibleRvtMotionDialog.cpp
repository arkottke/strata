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
#include "DimensionLayout.h"
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
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTabWidget>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

CompatibleRvtMotionDialog::CompatibleRvtMotionDialog(CompatibleRvtMotion *motion, bool readOnly, QWidget *parent) :
    QDialog(parent), _motion(motion)
{
    int row = 0;
    // Input group box
    QGridLayout *layout = new QGridLayout;
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 2);
    layout->setRowStretch(1, 1);

    // Name
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setText(_motion->name());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            _motion, SLOT(setName(QString)));

    layout->addWidget(new QLabel(tr("Name:")), row, 0);
    layout->addWidget(lineEdit, row++, 1);

    // Description
    lineEdit = new QLineEdit;
    lineEdit->setText(_motion->description());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            _motion, SLOT(setDescription(QString)));

    layout->addWidget(new QLabel(tr("Description:")), row, 0);
    layout->addWidget(lineEdit, row++, 1);

    // Type
    QComboBox *comboBox = new QComboBox;
    comboBox->addItems(AbstractMotion::typeList());
    comboBox->setCurrentIndex(_motion->type());
    comboBox->setDisabled(readOnly);
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            _motion, SLOT(setType(int)));

    layout->addWidget(new QLabel(tr("Type:")), row, 0);
    layout->addWidget(comboBox, row++, 1);

    // Duration
    QDoubleSpinBox *spinBox = new QDoubleSpinBox;
    spinBox->setRange(0.10, 100.00);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.10);
    spinBox->setSuffix(" s");
    spinBox->setValue(_motion->duration());
    spinBox->setReadOnly(readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            _motion, SLOT(setDuration(double)));

    layout->addWidget(new QLabel(tr("Duration:")), row, 0);
    layout->addWidget(spinBox, row++, 1);

    // Damping
    spinBox = new QDoubleSpinBox;
    spinBox->setRange(1, 20);
    spinBox->setDecimals(1);
    spinBox->setSingleStep(1);
    spinBox->setSuffix(" %");
    spinBox->setValue(_motion->targetRespSpec()->damping());
    spinBox->setReadOnly(readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            _motion->targetRespSpec(), SLOT(setDamping(double)));

    layout->addWidget(new QLabel(tr("Damping of Target:")), row, 0);
    layout->addWidget(spinBox, row++, 1);

    // Target response specturm
    TableGroupBox *tableGroupBox = new TableGroupBox(tr("Target Response Spectrum"), this);
    tableGroupBox->setModel(_motion->targetRespSpec());
    tableGroupBox->setReadOnly(readOnly);
    tableGroupBox->table()->setItemDelegateForColumn(0, new OnlyIncreasingDelegate);

    layout->addWidget(tableGroupBox, row++, 0, 1, 2);

    // Limit
    QCheckBox *checkBox = new QCheckBox(tr("Limit shape of FAS"));
    checkBox->setChecked(_motion->limitFas());
    connect(checkBox, SIGNAL(toggled(bool)), _motion, SLOT(setLimitFas(bool)));
    layout->addWidget(checkBox, row++, 0, 1, 2);


    // Create a tabs for the plots and table of data
    QTabWidget *tabWidget = new QTabWidget;


    // Response spectrum plot
    QwtPlot *plot = new QwtPlot;
    plot->setAutoReplot(true);
    QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());


    plot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());

    QFont font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text = QwtText(tr("Period (s)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Spectral Accel. (g)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    _saCurve = new QwtPlotCurve;
    _saCurve->setPen(QPen(Qt::blue));
    _saCurve->setSamples(_motion->respSpec()->period(),
                       _motion->respSpec()->sa());
    _saCurve->attach(plot);

    _targetSaCurve = new QwtPlotCurve;
    _targetSaCurve->setStyle(QwtPlotCurve::NoCurve);
    _targetSaCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(),
                                         QPen(Qt::red), QSize(5,5)));
    _targetSaCurve->setSamples(_motion->targetRespSpec()->period(),
                             _motion->targetRespSpec()->sa());
    _targetSaCurve->attach(plot);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(plot);

    tabWidget->addTab(scrollArea, tr("RS Plot"));

    // Fourier amplitude spectrum plot
    plot = new QwtPlot;
    plot->setAutoReplot(true);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPlotPicker::CrossRubberBand,
                               QwtPlotPicker::ActiveOnly, plot->canvas());

    plot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());
    font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    text = QwtText(tr("Frequency (Hz)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Fourier Amplitude (g-s)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    _fasCurve = new QwtPlotCurve;
    _fasCurve->setPen(QPen(Qt::blue));
    _fasCurve->setSamples(_motion->freq(),_motion->fourierAcc());
    _fasCurve->attach(plot);

    scrollArea = new QScrollArea;
    scrollArea->setWidget(plot);

    tabWidget->addTab(scrollArea, tr("FAS Plot"));

    // Response spectrum table
    _rsTableView = new MyTableView;
    _rsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _rsTableView->setModel(_motion->respSpec());
    tabWidget->addTab(_rsTableView, tr("RS Data"));

    // Fourier amplitude spectrum table
    _fasTableView = new MyTableView;
    _fasTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _fasTableView->setModel(_motion);
    tabWidget->addTab(_fasTableView, tr("FAS Data"));

    layout->addWidget(tabWidget, 0, 2, row, 1);

    // Buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
            Qt::Horizontal);


    QPushButton* pushButton = new QPushButton(tr("Frequency Parameters..."));
    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(openFrequencyDialog()));
    buttonBox->addButton(pushButton, QDialogButtonBox::ActionRole);

    // FIXME connect accepted() to tryAccept() and then check if the motion is valid
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(calculate()));

    layout->addWidget(buttonBox, row, 0, 1, 3);

    setLayout(layout);

    // Add copy and paste actions
    addAction(EditActions::instance()->copyAction());
    addAction(EditActions::instance()->pasteAction());
}

void CompatibleRvtMotionDialog::openFrequencyDialog()
{
    QDialog dialog(this);

    DimensionLayout* layout = new DimensionLayout;
    layout->setRange(0.001, 1000);
    layout->setModel(_motion->freqDimension());
    layout->setSuffix(" Hz");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok, Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()),
            &dialog, SLOT(accept()));

    layout->addRow(buttonBox);

    dialog.setLayout(layout);
    dialog.exec();
}

void CompatibleRvtMotionDialog::calculate()
{
    _motion->calculate();

    _fasTableView->resizeRowsToContents();
    _rsTableView->resizeRowsToContents();

    _fasCurve->setSamples(_motion->freq(),_motion->fourierAcc());
    _saCurve->setSamples(_motion->respSpec()->period(),
                          _motion->respSpec()->sa());
    _targetSaCurve->setSamples(_motion->targetRespSpec()->period(),
                                _motion->targetRespSpec()->sa());
}

void CompatibleRvtMotionDialog::tryAccept()
{
    _motion->calculate();
    accept();
}

