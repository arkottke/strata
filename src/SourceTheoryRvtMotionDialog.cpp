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

#include "SourceTheoryRvtMotionDialog.h"

#include "CrustalAmplification.h"
#include "CrustalModel.h"
#include "Dimension.h"
#include "DimensionLayout.h"
#include "EditActions.h"
#include "ResponseSpectrum.h"
#include "TableGroupBox.h"

#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

SourceTheoryRvtMotionDialog::SourceTheoryRvtMotionDialog(SourceTheoryRvtMotion *motion, bool readOnly, QWidget *parent) :
        QDialog(parent), m_motion(motion)
{
    QGridLayout *layout = new QGridLayout;

    layout->addWidget(createSourceTheoryForm(readOnly), 0, 0);

    // Create a tabs for the plots and table of data
    QTabWidget *tabWidget = new QTabWidget;

    // Response spectrum plot
    QwtPlot *plot = new QwtPlot;
    plot->setAutoReplot(true);

    QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());


    plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
    QFont font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text = QwtText(tr("Period (s)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Spectral Accel. (g)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    m_saCurve = new QwtPlotCurve;
    m_saCurve->setPen(QPen(Qt::blue));
    m_saCurve->setSamples(m_motion->respSpec()->period(), m_motion->respSpec()->sa());
    m_saCurve->attach(plot);

    tabWidget->addTab(plot, tr("RS Plot"));

    // Fourier amplitude spectrum plot
    plot = new QwtPlot;
    plot->setAutoReplot(true);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
    font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    text = QwtText(tr("Frequency (Hz)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Fourier Amplitude (g-s)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    m_fasCurve = new QwtPlotCurve;
    m_fasCurve->setPen(QPen(Qt::blue));
    m_fasCurve->setSamples(m_motion->freq(), m_motion->fourierAcc());
    m_fasCurve->attach(plot);

    tabWidget->addTab(plot, tr("FAS Plot"));

    // Response spectrum table
    MyTableView *tableView = new MyTableView;
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setModel(m_motion->respSpec());
    tabWidget->addTab(tableView, tr("RS Data"));

    // Fourier amplitude spectrum table
    tableView = new MyTableView;
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setModel(m_motion);
    tabWidget->addTab(tableView, tr("FAS Data"));

    layout->addWidget(tabWidget, 0, 1);

    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
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

    layout->addWidget(buttonBox, 1, 0, 1, 2);

    setLayout(layout);

    // Add copy and paste actions
    addAction(EditActions::instance()->copyAction());
    addAction(EditActions::instance()->pasteAction());
}

QTabWidget* SourceTheoryRvtMotionDialog::createSourceTheoryForm(bool readOnly)
{
    QTabWidget *tabWidget = new QTabWidget;
    QFormLayout *layout = new QFormLayout;

    // Name
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setText(m_motion->nameTemplate());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            m_motion, SLOT(setName(QString)));    
    layout->addRow(tr("Name:"), lineEdit);

    // Description
    lineEdit = new QLineEdit;
    lineEdit->setText(m_motion->description());
    lineEdit->setReadOnly(readOnly);

    connect(lineEdit, SIGNAL(textChanged(QString)),
            m_motion, SLOT(setDescription(QString)));
    layout->addRow(tr("Description:"), lineEdit);


    QLabel *label = new QLabel("Brune single-corner frequency point source model. Default coefficients from Campbell (2003).");
    label->setWordWrap(true);
    layout->addRow(label);

    // Moment magnitude
    QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(4, 9);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.1);
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->momentMag());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setMomentMag(double)));

    layout->addRow(tr("Moment Magnitude (<b>M</b>):"), doubleSpinBox);

    // Distance
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 500);
    doubleSpinBox->setDecimals(1);
    doubleSpinBox->setSingleStep(1);
    doubleSpinBox->setSuffix(" km");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->distance());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setDistance(double)));

    layout->addRow(tr("Epicentral distance:"), doubleSpinBox);

    // Depth
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 20);
    doubleSpinBox->setDecimals(1);
    doubleSpinBox->setSingleStep(1);
    doubleSpinBox->setSuffix(" km");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->depth());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setDepth(double)));

    layout->addRow(tr("Depth:"), doubleSpinBox);

    // Location
    QComboBox *modelComboBox = new QComboBox;
    modelComboBox->addItems(SourceTheoryRvtMotion::sourceList());


    connect(modelComboBox, SIGNAL(currentIndexChanged(int)),
            m_motion, SLOT(setModel(int)));

    layout->addRow(tr("Parameter Region:"), modelComboBox);

    // Stress drop
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(5, 500);
    doubleSpinBox->setDecimals(0);
    doubleSpinBox->setSingleStep(5);
    doubleSpinBox->setSuffix(" bars");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->stressDrop());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setStressDrop(double)));
    connect(m_motion, SIGNAL(stressDropChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Stress drop (%1%2)")).arg(QChar(0x0394)).arg(QChar(0x03c3)),
                   doubleSpinBox);

    // Geometric attenuation
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 1);
    doubleSpinBox->setDecimals(4);
    doubleSpinBox->setSingleStep(0.01);
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->geoAtten());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setGeoAtten(double)));
    connect(m_motion, SIGNAL(geoAttenChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Geometric atten. coeff.:"), doubleSpinBox);

    // Path duration coefficient
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 0.20);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.01);
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->pathDurCoeff());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setPathDurCoeff(double)));
    connect(m_motion, SIGNAL(pathDurCoeffChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Path duration coefficient:"), doubleSpinBox);

    // Path attenuation
    layout->addRow(new QLabel(tr("Path attenuation, Q(f) = <b>a</b> f <sup><b>b</b></sup>")));

    // Use a label with 20 points of indent


    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(50, 10000);
    doubleSpinBox->setDecimals(0);
    doubleSpinBox->setSingleStep(10);
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->pathAttenCoeff());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setPathAttenCoeff(double)));
    connect(m_motion, SIGNAL(pathAttenCoeffChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    label = new QLabel(tr("Coefficient (a):"));
    const int indent = 20;
    label->setIndent(indent);
    layout->addRow(label, doubleSpinBox);

    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0.0, 1.0);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.01);
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->pathAttenPower());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setPathAttenPower(double)));
    connect(m_motion, SIGNAL(pathAttenPowerChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    label = new QLabel(tr("Power (b):"));
    label->setIndent(indent);
    layout->addRow(label, doubleSpinBox);

    // Shear velocity
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(2.0, 5.0);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.1);
    doubleSpinBox->setSuffix(" km/sec");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->shearVelocity());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setShearVelocity(double)));
    connect(m_motion, SIGNAL(shearVelocityChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Shear velocity (v<sub>s</sub>):"), doubleSpinBox);

    // Density
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(2.4, 3.5);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.1);
    doubleSpinBox->setSuffix(" g/cc");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->density());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setDensity(double)));
    connect(m_motion, SIGNAL(densityChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Density (%1)")).arg(QChar(0x03c1)), doubleSpinBox);

    // Site Attenuation
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0.001, 0.10);
    doubleSpinBox->setDecimals(4);
    doubleSpinBox->setSingleStep(0.001);
    doubleSpinBox->setSuffix(" sec");
    doubleSpinBox->setReadOnly(readOnly);

    doubleSpinBox->setValue(m_motion->siteAtten());
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setSiteAtten(double)));
    connect(m_motion, SIGNAL(siteAttenChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Site attenuation (%1<sub>0</sub>)")).arg(QChar(0x03ba)), doubleSpinBox);

    // Duration
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 60);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSuffix(" sec");
    doubleSpinBox->setReadOnly(true);
    doubleSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    doubleSpinBox->setValue(m_motion->duration());
    doubleSpinBox->setReadOnly(readOnly);


    connect(m_motion, SIGNAL(durationChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));

    layout->addRow(tr("Duration:"), doubleSpinBox);

    QFrame *frame = new QFrame;
    frame->setLayout(layout);
    tabWidget->addTab(frame, tr("Point Source Parameters"));

    // Crustal model
    QGridLayout *gridLayout = new QGridLayout;
    int row = 0;

    QComboBox *comboBox = new QComboBox;
    comboBox->addItems(CrustalAmplification::sourceList());
    comboBox->setCurrentIndex(m_motion->crustalAmp()->model());
    comboBox->setDisabled(readOnly);

    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            m_motion->crustalAmp(), SLOT(setModel(int)));
    connect(m_motion->crustalAmp(), SIGNAL(modelChanged(int)),
            comboBox, SLOT(setCurrentIndex(int)));
    connect(m_motion, SIGNAL(isCustomizeable(bool)),
            comboBox, SLOT(setEnabled(bool)));

    gridLayout->addWidget(new QLabel(tr("Crustal Model:")), row, 0);
    gridLayout->addWidget(comboBox, row++, 1);

    TableGroupBox *tableGroupBox = new TableGroupBox(tr("Amplification"));
    tableGroupBox->setModel(m_motion->crustalAmp());
    tableGroupBox->setReadOnly(readOnly);

    connect(m_motion->crustalAmp(), SIGNAL(readOnlyChanged(bool)),
            tableGroupBox, SLOT(setReadOnly(bool)));

    gridLayout->addWidget(tableGroupBox, row, 0, 1, 2);

    tableGroupBox = new TableGroupBox(tr("Crustal Model"));
    tableGroupBox->setModel(m_motion->crustalAmp()->crustalModel());
    tableGroupBox->setReadOnly(readOnly);

    connect(m_motion->crustalAmp(), SIGNAL(needsCrustalModelChanged(bool)),
            tableGroupBox, SLOT(setVisible(bool)));

    gridLayout->addWidget(tableGroupBox, row, 2);

    frame = new QFrame;
    frame->setLayout(gridLayout);

    tabWidget->addTab(frame, tr("Crustal Amplification"));

    // Need to set source after all of the connections have been established
    modelComboBox->setCurrentIndex(m_motion->model());

    return tabWidget;
}

void SourceTheoryRvtMotionDialog::openFrequencyDialog()
{
    QDialog dialog;

    DimensionLayout* layout = new DimensionLayout;    
    layout->setModel(m_motion->freqDimension());
    layout->setRange(0.001, 200);
    layout->setSuffix(" Hz");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()),
            &dialog, SLOT(accept()));

    layout->addRow(buttonBox);

    dialog.setLayout(layout);
    dialog.exec();
}

void SourceTheoryRvtMotionDialog::calculate()
{
    m_motion->calculate();

    m_fasCurve->setSamples(m_motion->freq(), m_motion->fourierAcc());
    m_saCurve->setSamples(m_motion->respSpec()->period(), m_motion->respSpec()->sa());
}

void SourceTheoryRvtMotionDialog::tryAccept()
{
    m_motion->calculate();
    accept();
}
