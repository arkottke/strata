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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "CrustalAmpModel.h"
#include "CrustalVelModel.h"
#include "FourierSpectrumModel.h"
#include "MotionPage.h"
#include "RecordedMotionModel.h"
#include "ColumnTableModel.h"
#include "StringListDelegate.h"
#include "Dimension.h"
#include "EditActions.h"

#include <QTableView>
#include <QDesktopServices>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressDialog>
#include <QSettings>
#include <QDebug>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>

MotionPage::MotionPage(SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent,f), m_model(model)
{
    m_stackedWidget = new QStackedWidget;
    // Rvt Motion Page
    createInputLayer();
    createRecordedPage(); 
    createRvtPage();

    // Set the layout of the widget
    QVBoxLayout * layout = new QVBoxLayout;

    layout->addWidget(m_inputLocationGroupBox);
    layout->addWidget(m_stackedWidget);

    setLayout(layout);
    // Load the values
    load();
}

void MotionPage::setMethod(int type)
{
    m_stackedWidget->setCurrentIndex(type);
}

void MotionPage::setReadOnly(bool b)
{
	m_depthComboBox->setDisabled(b);

	m_recordedMotionTableBox->setReadOnly(b);
        
	m_typeComboBox->setDisabled(b);
	m_durationSpinBox->setReadOnly(b);
	m_limitFasCheckBox->setDisabled(b);
	m_sourceComboBox->setDisabled(b);
	m_dampingSpinBox->setReadOnly(b);

	m_fourierSpecTableBox->setReadOnly(b);
	m_respSpecTableBox->setReadOnly(b);

	m_momentMagSpinBox->setReadOnly(b);
	m_distanceSpinBox->setReadOnly(b);
	m_depthSpinBox->setReadOnly(b);
	m_locationComboBox->setDisabled(b);
	m_stressDropSpinBox->setReadOnly(b);
	m_geoAttenSpinBox->setReadOnly(b);
	m_pathDurSpinBox->setReadOnly(b);
	m_pathAttenCoeffSpinBox->setReadOnly(b);
	m_pathAttenPowerSpinBox->setReadOnly(b);
	m_shearVelSpinBox->setReadOnly(b);
	m_densitySpinBox->setReadOnly(b);
	m_siteAttenSpinBox->setReadOnly(b);
	m_specificCrustalAmpCheckBox->setDisabled(b);
	m_computeCrustalAmpButton->setDisabled(b);
	m_siteAmpTableGroupBox->setReadOnly(b);
	m_crustalTableGroupBox->setReadOnly(b);
}

void MotionPage::createInputLayer()
{
    QHBoxLayout * layout = new QHBoxLayout;

    layout->addWidget(new QLabel(tr("Specify the location to input the motion(s):")));

    m_depthComboBox = new DepthComboBox;
    connect(m_depthComboBox, SIGNAL(depthChanged(double)), 
            m_model->siteProfile(), SLOT(setInputDepth(double)));
    layout->addWidget(m_depthComboBox);

    layout->addStretch(1);

    m_inputLocationGroupBox = new QGroupBox(tr("Motion Input Location"));
    m_inputLocationGroupBox->setLayout(layout);
}

void MotionPage::createRecordedPage()
{
    // Recorded Motions Page
    m_recordedMotionTableBox = new TableGroupBox(
            new RecordedMotionModel(m_model->recordedMotions()), 
            tr("Recorded Motion(s)"));
    m_recordedMotionTableBox->table()->setItemDelegateForColumn(2, new StringListDelegate);

    connect(m_recordedMotionTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));

    QPushButton * pushButton = new QPushButton(QIcon(":/images/document-import.svg"), tr("Load Suite"));
    m_recordedMotionTableBox->addButton(pushButton);
    connect(pushButton, SIGNAL(clicked()), SLOT(loadSuite()));

    connect(m_model, SIGNAL(recordedMotionsChanged()), m_recordedMotionTableBox->table()->model(), SLOT(resetModel()));

    m_stackedWidget->addWidget(m_recordedMotionTableBox);
}

void MotionPage::createRvtPage()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(2, 1);

    // Properties group box
    QGridLayout * propsLayout = new QGridLayout;
    propsLayout->setColumnStretch(1, 1);

    int row = 0;
    // Type of motion
    m_typeComboBox = new QComboBox;
    m_typeComboBox->addItems(Motion::typeList());
    connect(m_typeComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->rvtMotion(), SLOT(setType(int)));

    propsLayout->addWidget(new QLabel(tr("Type:")), row, 0);
    propsLayout->addWidget(m_typeComboBox, row++, 1, 1, 2);

    // Source combo
    m_sourceComboBox = new QComboBox;
    m_sourceComboBox->addItems(RvtMotion::sourceList());

    connect(m_sourceComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->rvtMotion(), SLOT(setSource(int)));
    connect(m_sourceComboBox, SIGNAL(currentIndexChanged(int)), SLOT(setSource(int)));

    propsLayout->addWidget(new QLabel(tr("Source:")), row, 0);
    propsLayout->addWidget(m_sourceComboBox, row++, 1, 1, 2);

    // Duration of motion
    m_durationSpinBox = new QDoubleSpinBox;
    m_durationSpinBox->setRange(1, 40);
    m_durationSpinBox->setDecimals(2);
    m_durationSpinBox->setSingleStep(0.1);
    m_durationSpinBox->setSuffix(" sec");

    connect(m_durationSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion(), SLOT(setDuration(double)));
    connect(m_durationSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion()->targetRespSpec(), SLOT(setModified()));

    propsLayout->addWidget(new QLabel(tr("Duration:")), row, 0);
    propsLayout->addWidget(m_durationSpinBox, row++, 1, 1, 2);

    // Maximum engineering frequency spinbox
    m_maxEngFreqSpinBox = new QDoubleSpinBox;
    m_maxEngFreqSpinBox->setRange(10, 100);
    m_maxEngFreqSpinBox->setDecimals(1);
    m_maxEngFreqSpinBox->setSingleStep(1);
    m_maxEngFreqSpinBox->setSuffix(" Hz");

    connect(m_maxEngFreqSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion(), SLOT(setMaxEngFreq(double)));

    propsLayout->addWidget(new QLabel(tr("Max. Engineering Freq.:")), row, 0);
    propsLayout->addWidget(m_maxEngFreqSpinBox, row++, 1, 1, 2);

    // Insert a spacing row
    propsLayout->setRowMinimumHeight(row++, 15);

    // Damping of response spectrum
    m_dampingSpinBox = new QDoubleSpinBox;
    m_dampingSpinBox->setRange(1, 15);
    m_dampingSpinBox->setDecimals(2);
    m_dampingSpinBox->setSingleStep(1);
    m_dampingSpinBox->setSuffix(" %");

    connect(m_dampingSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion()->targetRespSpec(), SLOT(setDamping(double)));

    propsLayout->addWidget(new QLabel(tr("Damping:")), row, 0);
    propsLayout->addWidget(m_dampingSpinBox, row++, 1, 1, 2);

    // Insert a spacing row
    propsLayout->setRowMinimumHeight(row++, 15);

    // Apply limitation to FAS shape
    m_limitFasCheckBox = new QCheckBox(tr("Limit FAS shape (recommended)"));
    m_limitFasCheckBox->setEnabled(false);

    connect(m_limitFasCheckBox, SIGNAL(toggled(bool)), 
            m_model->rvtMotion(), SLOT(setLimitFas(bool)));
    propsLayout->addWidget(m_limitFasCheckBox, row++, 0, 1, 3);

    // Insert a spacing row
    propsLayout->setRowMinimumHeight(row++, 15);

    // Add preview push button
    propsLayout->addWidget(new QLabel(tr("Preview:")), row, 0);

    QPushButton * dataPushButton = new QPushButton(tr("Data"));
    connect(dataPushButton, SIGNAL(clicked()), this, SLOT(previewData()));

    propsLayout->addWidget(dataPushButton, row, 1);

    QPushButton * plotPushButton = new QPushButton(tr("Plot"));
    connect(plotPushButton, SIGNAL(clicked()), this, SLOT(previewPlot()));

    propsLayout->addWidget(plotPushButton, row, 2);

    // Properties group box
    m_rvtPropsGroupBox = new QGroupBox(tr("Properties"));
    m_rvtPropsGroupBox->setLayout(propsLayout);

    layout->addWidget(m_rvtPropsGroupBox, 0, 0);

    // Response Spectrum table
    m_respSpecTableBox = new TableGroupBox(
            m_model->rvtMotion()->targetRespSpec(), tr("Acceleration Response Spectrum"));
    connect(m_respSpecTableBox->table(), SIGNAL(dataPasted()), m_model->rvtMotion(), SLOT(setModified()));
    connect(m_model->rvtMotion()->targetRespSpec(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_model, SLOT(setModified()));

    // Fourier Amplitude Spectrum table
    m_fourierSpecTableBox = new TableGroupBox(
            new FourierSpectrumModel(m_model->rvtMotion(), false), tr("Fourier Amplitude Spectrum"));
    connect(m_fourierSpecTableBox, SIGNAL(dataChanged()), m_model->rvtMotion(), SLOT(setModified()));

    // Input for point source model
    createPointSourceGroupBox();

    // Combine the spectra in a tabbed widget
    m_rvtSourceStackedWidget = new QStackedWidget;
    m_rvtSourceStackedWidget->addWidget(m_fourierSpecTableBox);
    m_rvtSourceStackedWidget->addWidget(m_respSpecTableBox);
    m_rvtSourceStackedWidget->addWidget(m_pointSourceGroupBox);

    layout->addWidget(m_rvtSourceStackedWidget, 0, 1, 3, 1);

    // Create the widget
    QWidget * widget = new QWidget;
    widget->setLayout(layout); 

    m_stackedWidget->addWidget(widget);
}

void MotionPage::createPointSourceGroupBox()
{
    QGridLayout * layout = new QGridLayout;

    int row = 0;

    layout->addWidget(new QLabel("Brune single-corner frequency point source model. Default coefficients from Campbell (2003)."), row++, 0, 1, 3);
    // Moment magnitude
    m_momentMagSpinBox = new QDoubleSpinBox;
    m_momentMagSpinBox->setRange(4, 9);
    m_momentMagSpinBox->setDecimals(2);
    m_momentMagSpinBox->setSingleStep(0.1);
    connect(m_momentMagSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setMomentMag(double)));

    layout->addWidget(new QLabel(tr("Moment Magnitude (<b>M</b>):")), row, 0);
    layout->addWidget(m_momentMagSpinBox, row++, 1);

    // Distance
    m_distanceSpinBox = new QDoubleSpinBox;
    m_distanceSpinBox->setRange(0, 1000);
    m_distanceSpinBox->setDecimals(0);
    m_distanceSpinBox->setSingleStep(1);
    m_distanceSpinBox->setSuffix(" km");
    connect(m_distanceSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setDistance(double)));

    layout->addWidget(new QLabel(tr("Epicentral distance:")), row, 0);
    layout->addWidget(m_distanceSpinBox, row++, 1);
    
    // Depth
    m_depthSpinBox = new QDoubleSpinBox;
    m_depthSpinBox->setRange(3, 20);
    m_depthSpinBox->setDecimals(0);
    m_depthSpinBox->setSingleStep(1);
    m_depthSpinBox->setSuffix(" km");
    connect(m_depthSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setDepth(double)));

    layout->addWidget(new QLabel(tr("Depth:")), row, 0);
    layout->addWidget(m_depthSpinBox, row++, 1);

    // Location
    m_locationComboBox = new QComboBox;
    m_locationComboBox->addItems(PointSourceModel::locationList());

    connect(m_locationComboBox, SIGNAL(currentIndexChanged(int)), SLOT(setLocation(int)));

    layout->addWidget(new QLabel(tr("General crustal region:")), row, 0);
    layout->addWidget(m_locationComboBox, row++, 1);

    // Stress drop
    m_stressDropSpinBox = new QDoubleSpinBox;
    m_stressDropSpinBox->setRange(5, 500);
    m_stressDropSpinBox->setDecimals(0);
    m_stressDropSpinBox->setSingleStep(5);
    m_stressDropSpinBox->setSuffix(" bars");
    connect(m_stressDropSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setStressDrop(double)));

    layout->addWidget(new QLabel(QString(tr("Stress drop (%1%2)").arg(QChar(0x0394)).arg((QChar(0x03c3))))), row, 0);
    layout->addWidget(m_stressDropSpinBox, row++, 1);

    // Geometric attenuation
    m_geoAttenSpinBox = new QDoubleSpinBox;
    m_geoAttenSpinBox->setRange(0, 1);
    m_geoAttenSpinBox->setDecimals(4);
    m_geoAttenSpinBox->setSingleStep(0.01);
    connect(m_geoAttenSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setGeoAtten(double)));
    connect(m_model->rvtMotion()->pointSourceModel(), SIGNAL(geoAttenChanged(double)), m_geoAttenSpinBox, SLOT(setValue(double)));

    layout->addWidget(new QLabel(tr("Geometric attenuation coeff.:")), row, 0);
    layout->addWidget(m_geoAttenSpinBox, row++, 1);

    // Path duration coefficient
    m_pathDurSpinBox = new QDoubleSpinBox;
    m_pathDurSpinBox->setRange(0, 0.20);
    m_pathDurSpinBox->setDecimals(2);
    m_pathDurSpinBox->setSingleStep(0.01);
    connect(m_pathDurSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setPathDurCoeff(double)));
    connect(m_model->rvtMotion()->pointSourceModel(), SIGNAL(pathDurCoeffChanged(double)), m_pathDurSpinBox, SLOT(setValue(double)));

    layout->addWidget(new QLabel(tr("Path duration coefficient:")), row, 0);
    layout->addWidget(m_pathDurSpinBox, row++, 1);


    // Path attenuation 
    layout->addWidget(new QLabel(tr("Path attenuation, Q(f) = <b>a</b> f <sup><b>b</b></sup>")), row++, 0);

    m_pathAttenCoeffSpinBox = new QDoubleSpinBox;
    m_pathAttenCoeffSpinBox->setRange(50,1000);
    m_pathAttenCoeffSpinBox->setDecimals(0);
    m_pathAttenCoeffSpinBox->setSingleStep(10);
    connect(m_pathAttenCoeffSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setPathAttenCoeff(double)));

    QLabel * label = new QLabel(tr("Coefficient (a):"));
    label->setIndent(20);
    layout->addWidget(label, row, 0);
    layout->addWidget(m_pathAttenCoeffSpinBox, row++, 1);

    m_pathAttenPowerSpinBox = new QDoubleSpinBox;
    m_pathAttenPowerSpinBox->setRange(0.2,0.6);
    m_pathAttenPowerSpinBox->setDecimals(2);
    m_pathAttenPowerSpinBox->setSingleStep(0.01);
    connect(m_pathAttenPowerSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setPathAttenPower(double)));

    label = new QLabel(tr("Power (b):"));
    label->setIndent(20);
    layout->addWidget(label, row, 0);
    layout->addWidget(m_pathAttenPowerSpinBox, row++, 1);

    // Shear velocity
    m_shearVelSpinBox = new QDoubleSpinBox;
    m_shearVelSpinBox->setRange(2.0, 5.0);
    m_shearVelSpinBox->setDecimals(2);
    m_shearVelSpinBox->setSingleStep(0.1);
    m_shearVelSpinBox->setSuffix(" km/sec");
    connect(m_shearVelSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setShearVelocity(double)));

    layout->addWidget(new QLabel(tr("Shear velocity (v<sub>s</sub>):")), row, 0);
    layout->addWidget(m_shearVelSpinBox, row++, 1);

    // Density
    m_densitySpinBox = new QDoubleSpinBox;
    m_densitySpinBox->setRange(2.4,3.0);
    m_densitySpinBox->setDecimals(2);
    m_densitySpinBox->setSingleStep(0.1);
    m_densitySpinBox->setSuffix(" g/cc");
    connect(m_densitySpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setDensity(double)));

    layout->addWidget(new QLabel(QString(tr("Density (%1)")).arg(QChar(0x03c1))), row, 0);
    layout->addWidget(m_densitySpinBox, row++, 1);

    // Site Attenuation
    m_siteAttenSpinBox = new QDoubleSpinBox;
    m_siteAttenSpinBox->setRange(0.001, 0.06);
    m_siteAttenSpinBox->setDecimals(4);
    m_siteAttenSpinBox->setSingleStep(0.001);
    m_siteAttenSpinBox->setSuffix(" sec");
    connect(m_siteAttenSpinBox, SIGNAL(valueChanged(double)), m_model->rvtMotion()->pointSourceModel(), SLOT(setSiteAttenuation(double)));

    layout->addWidget(new QLabel(QString(tr("Site attenuation (%1<sub>0</sub>)")).arg(QChar(0x03ba))), row, 0);
    layout->addWidget(m_siteAttenSpinBox, row++, 1);

    // Site specific Crustal Amplification
    m_specificCrustalAmpCheckBox = new QCheckBox(tr("Site specific crustal amplification"));
    connect(m_specificCrustalAmpCheckBox, SIGNAL(toggled(bool)), SLOT(setEnableCrustalTab(bool)));
    connect(m_specificCrustalAmpCheckBox, SIGNAL(toggled(bool)), m_model->rvtMotion()->pointSourceModel(), SLOT(setSiteSpecificCrustalAmp(bool)));

    m_computeCrustalAmpButton = new QPushButton(tr("Preview"));
    connect(m_specificCrustalAmpCheckBox, SIGNAL(clicked(bool)), m_computeCrustalAmpButton, SLOT(setEnabled(bool)));
    connect(m_computeCrustalAmpButton, SIGNAL(clicked()), SLOT(previewCrustalAmp()));
    
    layout->addWidget(m_specificCrustalAmpCheckBox , row, 0);
    layout->addWidget(m_computeCrustalAmpButton, row++, 1);


    // Site Tabbed Widget
    m_siteTabWidget = new QTabWidget;
    layout->addWidget(m_siteTabWidget, 1, 2, row, 1);

    // Site amplification
    m_siteAmpTableGroupBox = new TableGroupBox(new CrustalAmpModel(m_model->rvtMotion()->pointSourceModel()), tr("Crustal Amplification Model"));
    connect(m_siteAmpTableGroupBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));

    m_siteTabWidget->addTab(m_siteAmpTableGroupBox, tr("Crustal Amplification"));
    
    // Crustal Model amplification
    m_crustalTableGroupBox = new TableGroupBox(new CrustalVelModel(m_model->rvtMotion()->pointSourceModel()), tr("Crustal Velocity Model"));
    connect(m_crustalTableGroupBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));

    m_siteTabWidget->addTab(m_crustalTableGroupBox, tr("Crustal Model"));

    // Add some stretch!
    layout->setRowStretch(row, 1);
    layout->setColumnStretch(2, 1);
    // Create the group box
    m_pointSourceGroupBox = new QGroupBox(tr("Point Source Model"));
    m_pointSourceGroupBox->setLayout(layout);
}

bool MotionPage::computeRvtResponse()
{
    if (m_sourceComboBox->currentIndex() == RvtMotion::DefinedFourierSpectrum) {
        if (!m_model->rvtMotion()->modified())
            return true;

        ResponseSpectrum * rs = m_model->rvtMotion()->respSpec();

        rs->setDamping(m_dampingSpinBox->value());
        rs->setPeriod(Dimension::logSpace(0.01, 10., 60));
        rs->setSa(m_model->rvtMotion()->computeSa(rs->period(), rs->damping()));

        // Reset the modification flag
        m_model->rvtMotion()->setModified(false);
    } else if (m_sourceComboBox->currentIndex() == RvtMotion::DefinedResponseSpectrum) {
        if (!m_model->rvtMotion()->modified() && !m_model->rvtMotion()->targetRespSpec()->modified())
            return true;

        QProgressDialog progress("Computing FAS...", "Abort", 0, 10, this);
        progress.setWindowModality(Qt::WindowModal);

        connect(m_model->rvtMotion(), SIGNAL(progressRangeChanged(int,int)), &progress, SLOT(setRange(int, int)));
        connect(m_model->rvtMotion(), SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));
        connect(&progress, SIGNAL(canceled()), m_model->rvtMotion(), SLOT(stop()));

        // Compute the inversion
        //m_model->rvtMotion()->invert(dialog->okToContinue(), dialog->progressBar());
        m_model->rvtMotion()->invert();
    } else if (m_sourceComboBox->currentIndex() == RvtMotion::CalculatedFourierSpectrum) {
        if (!m_model->rvtMotion()->pointSourceModel()->modified())
            return true;

        m_model->rvtMotion()->calcPointSource();

        ResponseSpectrum * rs = m_model->rvtMotion()->respSpec();
        rs->setDamping(m_dampingSpinBox->value());
        rs->setPeriod(Dimension::logSpace(0.01, 10, 60));
        rs->setSa(m_model->rvtMotion()->computeSa(rs->period(), rs->damping()));
    }
    return true;
}

void MotionPage::previewData()
{
    if (computeRvtResponse() && m_dataDialog.isNull()) {
        QHBoxLayout * layout = new QHBoxLayout;

        // Response Spectrum table
        TableGroupBox * respSpecTableBox = new TableGroupBox(
                m_model->rvtMotion()->respSpec(), tr("Acceleration Response Spectrum"));
        respSpecTableBox->setReadOnly(true);
        respSpecTableBox->table()->resizeRowsToContents();
        layout->addWidget(respSpecTableBox);

        // Fourier Amplitude Spectrum table
        TableGroupBox * fourierSpecTableBox = new TableGroupBox(
                new FourierSpectrumModel(m_model->rvtMotion(), true), tr("Fourier Amplitude Spectrum"));
        fourierSpecTableBox->setReadOnly(true);
        fourierSpecTableBox->table()->resizeRowsToContents();
        layout->addWidget(fourierSpecTableBox);

        m_dataDialog = new QDialog(this,Qt::Window);
        m_dataDialog->setModal(false);
        m_dataDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        m_dataDialog->setLayout(layout);

        // Copy action
        m_dataDialog->addAction(EditActions::instance()->copyAction());

        // Show the dialog
        m_dataDialog->show();
    } 
    m_dataDialog->raise();
    m_dataDialog->activateWindow();
}

void MotionPage::previewPlot()
{
    if (computeRvtResponse() && m_plotDialog.isNull()) {
        // FAS Plot
        QwtPlot * fasPlot = new QwtPlot;
        fasPlot->setAxisTitle(QwtPlot::xBottom, tr("Frequency (Hz)"));
        fasPlot->setAxisTitle(QwtPlot::yLeft, tr("Fourier Amplitude (g-s)"));
        fasPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
        fasPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);

        QwtPlotCurve * fasCurve = new QwtPlotCurve;
        fasCurve->setPen(QPen(Qt::red));
        fasCurve->setData(
                m_model->rvtMotion()->freq().data(), 
                m_model->rvtMotion()->fas().data(), 
                m_model->rvtMotion()->freq().size());
        fasCurve->attach(fasPlot);

        QHBoxLayout * fasLayout = new QHBoxLayout;
        fasLayout->addWidget(fasPlot);

        QGroupBox * fasGroupBox = new QGroupBox(tr("Fourier Amplitude Spectrum")); 
        fasGroupBox->setLayout(fasLayout);

        // Sa Plot
        QwtPlot * saPlot = new QwtPlot;
        saPlot->setAxisTitle(QwtPlot::xBottom, tr("Period (s)"));
        saPlot->setAxisTitle(QwtPlot::yLeft, tr("Spectral Accel. (g)"));
        saPlot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
        //saPlot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);

        // User defined points
        if (m_sourceComboBox->currentIndex() == RvtMotion::DefinedResponseSpectrum) {
            QwtPlotCurve * saPoints = new QwtPlotCurve;
            saPoints->setStyle(QwtPlotCurve::NoCurve);
            saPoints->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(Qt::darkRed), QSize(5,5)));
            saPoints->setData(
                    m_model->rvtMotion()->targetRespSpec()->period().data(),
                    m_model->rvtMotion()->targetRespSpec()->sa().data(),
                    m_model->rvtMotion()->targetRespSpec()->period().size());
            saPoints->attach(saPlot);
        }

        // Interpolated curve   
        QwtPlotCurve * saCurve = new QwtPlotCurve;
        saCurve->setPen(QPen(Qt::red));
        saCurve->setData(
                m_model->rvtMotion()->respSpec()->period().data(), 
                m_model->rvtMotion()->respSpec()->sa().data(), 
                m_model->rvtMotion()->respSpec()->period().size());
        saCurve->attach(saPlot);

        QHBoxLayout * saLayout = new QHBoxLayout;
        saLayout->addWidget(saPlot);

        QGroupBox * saGroupBox = new QGroupBox(tr("Acceleration Response Spectrum"));
        saGroupBox->setLayout(saLayout);

        // Create the layout
        QHBoxLayout * layout = new QHBoxLayout;
        layout->addWidget(fasGroupBox);
        layout->addWidget(saGroupBox);

        // Create the dialog
        m_plotDialog = new QDialog(this);
        m_plotDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        m_plotDialog->setLayout(layout);
        m_plotDialog->resize(1000,400);

        // Show the dialog 
        m_plotDialog->exec();
    }
}

void MotionPage::previewCrustalAmp()
{
    m_model->rvtMotion()->pointSourceModel()->calcCrustalAmp();
    m_siteTabWidget->setCurrentIndex(0);
}

void MotionPage::loadSuite()
{
    QSettings settings;
    // Prompt for the file to load
    QString fileName =  QFileDialog::getOpenFileName(this,
                             tr("Select suite file.. - strata"),
                             settings.value("suiteDirectory",
                                            QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString(),
                             "Strata Suite File (*.csv)");


    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        QDir dir = fileInfo.dir();

        // Save the path
        settings.setValue("suiteDirectory", fileInfo.filePath());

        // Load the file
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() <<  "Unable to open file:" << fileName;
            return;
        }

        // Clear the previous motions
        while (m_model->recordedMotions().size())
            delete m_model->recordedMotions().takeFirst();

        // Load each of the filename/scale pairs in a map
        QMap<QString, double> map;

        QTextStream fin(&file);
        QString line = fin.readLine();;
        while (!line.isNull()) {
            if (line.startsWith("#"))
                continue;

            QStringList parts = line.split(",");
            if (parts.size() != 2) {
                qCritical() <<  "Invalid file format:" << fileName;
                return;
            }

            QString _fileName = parts.first();

            bool ok = false;
            double scale = parts.last().toDouble(&ok);
            if (!ok) {
                qCritical() << "Error converting" << parts.last() << "to a number!";
            }
            map.insertMulti(dir.absoluteFilePath(_fileName), scale);
            line = fin.readLine();
        } 

        QProgressDialog progressDialog(tr("Loading motion suite..."), tr("Abort"), 1, map.size(), this);
        progressDialog.setWindowModality(Qt::WindowModal);

        QMapIterator<QString, double> i(map);
        while (i.hasNext() && !progressDialog.wasCanceled()) {
            i.next();

            // Update the progress dialog
            progressDialog.setValue(m_model->recordedMotions().size());
            // Load the motion
            bool successful = true;
            RecordedMotion * m = new RecordedMotion(i.key(), i.value(), &successful);

            if (successful) {
                m_model->recordedMotions() << m;
            }
        } 
        progressDialog.setValue(m_model->recordedMotions().size());

        // Reset the table model
        qobject_cast<RecordedMotionModel*>(m_recordedMotionTableBox->model())->resetModel();
        // Set that the data has changed
        m_model->setModified(); 
    }
}
    
void MotionPage::setEnableCrustalTab(bool enabled)
{
    m_siteTabWidget->setTabEnabled(1, enabled);

    if (enabled) {
        m_siteTabWidget->setCurrentIndex(1);
    } else {
        m_siteTabWidget->setCurrentIndex(0);
    }
}

void MotionPage::load()
{
    m_depthComboBox->setDepth(m_model->siteProfile()->inputDepth());

    m_typeComboBox->setCurrentIndex(m_model->rvtMotion()->type());
    setSource(m_model->rvtMotion()->source());

    m_durationSpinBox->setValue(m_model->rvtMotion()->duration());
    m_limitFasCheckBox->setChecked(m_model->rvtMotion()->limitFas());

    m_maxEngFreqSpinBox->setValue(m_model->rvtMotion()->maxEngFreq());

    m_dampingSpinBox->setValue(m_model->rvtMotion()->targetRespSpec()->damping());
}

void MotionPage::setSource(int index)
{
    m_sourceComboBox->setCurrentIndex(index);
    m_rvtSourceStackedWidget->setCurrentIndex(index);

    if (index == RvtMotion::CalculatedFourierSpectrum) {
        m_durationSpinBox->setEnabled(false);
        connect(m_model->rvtMotion()->pointSourceModel(), SIGNAL(durationChanged(double)),
               m_durationSpinBox, SLOT(setValue(double)));

        // Load values for the point source model
        PointSourceModel * model = m_model->rvtMotion()->pointSourceModel();

        m_momentMagSpinBox->setValue(model->momentMag());
        m_distanceSpinBox->setValue(model->distance());
        m_depthSpinBox->setValue(model->depth());
        m_locationComboBox->setCurrentIndex((int)model->location());
    
        loadPointSourceParams();
    } else {
        m_durationSpinBox->setEnabled(true);
        disconnect(m_model->rvtMotion()->pointSourceModel(), 0 , m_durationSpinBox, 0);
    }
    
    if (index == RvtMotion::DefinedResponseSpectrum) {
        m_limitFasCheckBox->setEnabled(true);
    } else {
        m_limitFasCheckBox->setEnabled(false);
    }
}

void MotionPage::loadPointSourceParams()
{
    PointSourceModel * model = m_model->rvtMotion()->pointSourceModel();

    m_stressDropSpinBox->setValue(model->stressDrop());
    m_geoAttenSpinBox->setValue(model->geoAtten());
    m_pathDurSpinBox->setValue(model->pathDurCoeff());
    m_pathAttenCoeffSpinBox->setValue(model->pathAttenCoeff());
    m_pathAttenPowerSpinBox->setValue(model->pathAttenPower());
    m_shearVelSpinBox->setValue(model->shearVelocity());
    m_densitySpinBox->setValue(model->density());
    m_siteAttenSpinBox->setValue(model->siteAtten());
    m_durationSpinBox->setValue(model->duration());
}


void MotionPage::setLocation(int location)
{
    m_model->rvtMotion()->pointSourceModel()->setLocation((PointSourceModel::Location)location);
    bool enabled = false;
    PointSourceModel * model = m_model->rvtMotion()->pointSourceModel();

    switch (model->location())
    {
        case PointSourceModel::Custom:
            enabled = true;
            break;
        case PointSourceModel::WUS:
        case PointSourceModel::CEUS:
            // Load values
            loadPointSourceParams();
            m_specificCrustalAmpCheckBox->setChecked(false);

            enabled = false;
            break;
    }

    // Enable/disable the boxes
    m_stressDropSpinBox->setEnabled(enabled);
    m_geoAttenSpinBox->setEnabled(enabled);
    m_pathDurSpinBox->setEnabled(enabled);
    m_pathAttenCoeffSpinBox->setEnabled(enabled);
    m_pathAttenPowerSpinBox->setEnabled(enabled);
    m_shearVelSpinBox->setEnabled(enabled);
    m_densitySpinBox->setEnabled(enabled);
    m_siteAttenSpinBox->setEnabled(enabled);
    m_specificCrustalAmpCheckBox->setEnabled(enabled);
    m_siteAmpTableGroupBox->setReadOnly(enabled);

    if (model->location() == PointSourceModel::Custom
            && m_specificCrustalAmpCheckBox->isChecked()) {
        setEnableCrustalTab(true);
        m_computeCrustalAmpButton->setEnabled(true);
    } else {
        setEnableCrustalTab(false);
        m_computeCrustalAmpButton->setEnabled(false);
    }
}
