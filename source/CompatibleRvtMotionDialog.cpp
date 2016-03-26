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

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_picker.h>
#include <qwt/qwt_picker_machine.h>
#include <qwt/qwt_scale_engine.h>
#include <qwt/qwt_symbol.h>
#include <qwt/qwt_text.h>

CompatibleRvtMotionDialog::CompatibleRvtMotionDialog(CompatibleRvtMotion *motion, bool readOnly, QWidget *parent) :
    QDialog(parent), m_motion(motion)
{
    int row = 0;
    // Input group box
    QGridLayout *layout = new QGridLayout;
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 2);
    layout->setRowStretch(1, 1);

    // Name
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setText(m_motion->name());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            m_motion, SLOT(setName(QString)));

    layout->addWidget(new QLabel(tr("Name:")), row, 0);
    layout->addWidget(lineEdit, row++, 1);

    // Description
    lineEdit = new QLineEdit;
    lineEdit->setText(m_motion->description());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            m_motion, SLOT(setDescription(QString)));

    layout->addWidget(new QLabel(tr("Description:")), row, 0);
    layout->addWidget(lineEdit, row++, 1);

    // Type
    QComboBox *comboBox = new QComboBox;
    comboBox->addItems(AbstractMotion::typeList());
    comboBox->setCurrentIndex(m_motion->type());
    comboBox->setDisabled(readOnly);
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            m_motion, SLOT(setType(int)));

    layout->addWidget(new QLabel(tr("Type:")), row, 0);
    layout->addWidget(comboBox, row++, 1);

    // Duration
    QDoubleSpinBox *spinBox = new QDoubleSpinBox;
    spinBox->setRange(0.10, 100.00);
    spinBox->setDecimals(2);
    spinBox->setSingleStep(0.10);
    spinBox->setSuffix(" s");
    spinBox->setValue(m_motion->duration());
    spinBox->setReadOnly(readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setDuration(double)));

    layout->addWidget(new QLabel(tr("Duration:")), row, 0);
    layout->addWidget(spinBox, row++, 1);

    // Damping
    spinBox = new QDoubleSpinBox;
    spinBox->setRange(1, 20);
    spinBox->setDecimals(1);
    spinBox->setSingleStep(1);
    spinBox->setSuffix(" %");
    spinBox->setValue(m_motion->targetRespSpec()->damping());
    spinBox->setReadOnly(readOnly);

    connect(spinBox, SIGNAL(valueChanged(double)),
            m_motion->targetRespSpec(), SLOT(setDamping(double)));

    layout->addWidget(new QLabel(tr("Damping of Target:")), row, 0);
    layout->addWidget(spinBox, row++, 1);

    // Target response specturm
    TableGroupBox *tableGroupBox = new TableGroupBox(tr("Target Response Spectrum"), this);
    tableGroupBox->setModel(m_motion->targetRespSpec());
    tableGroupBox->setReadOnly(readOnly);
    tableGroupBox->table()->setItemDelegateForColumn(0, new OnlyIncreasingDelegate);

    layout->addWidget(tableGroupBox, row++, 0, 1, 2);

    // Limit
    QCheckBox *checkBox = new QCheckBox(tr("Limit shape of FAS"));
    checkBox->setChecked(m_motion->limitFas());
    connect(checkBox, SIGNAL(toggled(bool)), m_motion, SLOT(setLimitFas(bool)));
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

    m_saCurve = new QwtPlotCurve;
    m_saCurve->setPen(QPen(Qt::blue));
    m_saCurve->setSamples(m_motion->respSpec()->period(),
                       m_motion->respSpec()->sa());
    m_saCurve->attach(plot);

    m_targetSaCurve = new QwtPlotCurve;
    m_targetSaCurve->setStyle(QwtPlotCurve::NoCurve);
    m_targetSaCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(),
                                         QPen(Qt::red), QSize(5,5)));
    m_targetSaCurve->setSamples(m_motion->targetRespSpec()->period(),
                             m_motion->targetRespSpec()->sa());
    m_targetSaCurve->attach(plot);

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

    m_fasCurve = new QwtPlotCurve;
    m_fasCurve->setPen(QPen(Qt::blue));
    m_fasCurve->setSamples(m_motion->freq(),m_motion->fourierAcc());
    m_fasCurve->attach(plot);

    scrollArea = new QScrollArea;
    scrollArea->setWidget(plot);

    tabWidget->addTab(scrollArea, tr("FAS Plot"));

    // Response spectrum table
    m_rsTableView = new MyTableView;
    m_rsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_rsTableView->setModel(m_motion->respSpec());
    tabWidget->addTab(m_rsTableView, tr("RS Data"));

    // Fourier amplitude spectrum table
    m_fasTableView = new MyTableView;
    m_fasTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fasTableView->setModel(m_motion);
    tabWidget->addTab(m_fasTableView, tr("FAS Data"));

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
    layout->setModel(m_motion->freqDimension());
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
    m_motion->calculate();

    m_fasTableView->resizeRowsToContents();
    m_rsTableView->resizeRowsToContents();

    m_fasCurve->setSamples(m_motion->freq(),m_motion->fourierAcc());
    m_saCurve->setSamples(m_motion->respSpec()->period(),
                          m_motion->respSpec()->sa());
    m_targetSaCurve->setSamples(m_motion->targetRespSpec()->period(),
                                m_motion->targetRespSpec()->sa());
}

void CompatibleRvtMotionDialog::tryAccept()
{
    m_motion->calculate();
    accept();
}

