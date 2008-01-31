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

#include "FourierSpectrumModel.h"
#include "MotionPage.h"
#include "RecordedMotionModel.h"
#include "ResponseSpectrumModel.h"
#include "StringListDelegate.h"
#include "Dimension.h"
#include "MyProgressDialog.h"

#include <QTableView>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressDialog>
#include <QDebug>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>

MotionPage::MotionPage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f)
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

void MotionPage::setModel( SiteResponseModel * model )
{
    m_model = model;
}

void MotionPage::setMethod(int type)
{
    m_stackedWidget->setCurrentIndex(type);
}

void MotionPage::createInputLayer()
{
    QHBoxLayout * layout = new QHBoxLayout;

    layout->addWidget(new QLabel(tr("Specify the location to input the motion(s):")));

    m_depthComboBox = new DepthComboBox;
    connect( m_depthComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(hasChanged()));
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
    connect( m_recordedMotionTableBox, SIGNAL(dataChanged()), this, SIGNAL(hasChanged()));

    connect( m_model, SIGNAL(recordedMotionsChanged()), m_recordedMotionTableBox->table()->model(), SLOT(resetModel()));

    m_stackedWidget->addWidget(m_recordedMotionTableBox);
}

void MotionPage::createRvtPage()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 1, 1);
    layout->setRowStretch( 2, 1);

    // Properties group box
    QGridLayout * propsLayout = new QGridLayout;
    propsLayout->setColumnStretch( 2, 1);
    // First column is used to indent the strain and soil factors
    propsLayout->setColumnMinimumWidth(1,20);

    // Type of motion
    m_typeComboBox = new QComboBox;
    connect( m_typeComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(hasChanged()));
    m_typeComboBox->addItems(Motion::typeList());

    propsLayout->addWidget( new QLabel(tr("Type")), 0, 0, 1, 2);
    propsLayout->addWidget( m_typeComboBox, 0, 2, 1, 2);

    // Source combo
    m_sourceComboBox = new QComboBox;
    m_sourceComboBox->addItems( RvtMotion::sourceList() );

    connect( m_sourceComboBox, SIGNAL( currentIndexChanged(int)), this, SLOT(setSource(int)));

    propsLayout->addWidget(new QLabel(tr("Source:")), 1, 0, 1, 2);
    propsLayout->addWidget(m_sourceComboBox, 1, 2, 1, 2);

    // Insert a spacing row
    propsLayout->setRowMinimumHeight(2, 15);

    // Duration of motion
    m_durationSpinBox = new QDoubleSpinBox;
    m_durationSpinBox->setRange( 1, 40);
    m_durationSpinBox->setDecimals(2);
    m_durationSpinBox->setSingleStep(0.1);
    m_durationSpinBox->setSuffix(" s");

    connect( m_durationSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));
    connect( m_durationSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion(), SLOT(setModified()));
    connect( m_durationSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion()->targetRespSpec(), SLOT(setModified()));

    propsLayout->addWidget(new QLabel(tr("Duration:")), 3, 0, 1, 2);
    propsLayout->addWidget(m_durationSpinBox, 3, 2, 1, 2);

    // Strain duration factor
    m_strainFactorSpinBox = new QDoubleSpinBox;
    m_strainFactorSpinBox->setRange( 0.1, 10);
    m_strainFactorSpinBox->setDecimals(2);
    m_strainFactorSpinBox->setSingleStep(0.1);

    connect( m_strainFactorSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

    propsLayout->addWidget(new QLabel(tr("Strain factor:")), 4, 1);
    propsLayout->addWidget(m_strainFactorSpinBox, 4, 2, 1, 2);

    // Soil duration factor
    m_soilFactorSpinBox = new QDoubleSpinBox;
    m_soilFactorSpinBox->setRange( 0.1, 10);
    m_soilFactorSpinBox->setDecimals(2);
    m_soilFactorSpinBox->setSingleStep(0.1);

    connect( m_soilFactorSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

    propsLayout->addWidget(new QLabel(tr("Soil factor:")), 5, 1);
    propsLayout->addWidget(m_soilFactorSpinBox, 5, 2, 1, 2);

    // Insert a spacing row
    propsLayout->setRowMinimumHeight(6, 15);

    // Damping of response spectrum
    m_dampingSpinBox = new QDoubleSpinBox;
    m_dampingSpinBox->setRange( 1, 15);
    m_dampingSpinBox->setDecimals(2);
    m_dampingSpinBox->setSingleStep(1);
    m_dampingSpinBox->setSuffix(" %");
    m_dampingSpinBox->setEnabled(false);

    connect( m_dampingSpinBox, SIGNAL(valueChanged(QString)), this, SIGNAL(hasChanged()));
    connect( m_dampingSpinBox, SIGNAL(valueChanged(double)),
            m_model->rvtMotion()->targetRespSpec(), SLOT(setModified()));

    propsLayout->addWidget(new QLabel(tr("Damping:")), 7, 0, 1, 2);
    propsLayout->addWidget(m_dampingSpinBox, 7, 2, 1, 2);
    
    // Insert a spacing row
    propsLayout->setRowMinimumHeight(8, 15);

    // Add preview push button
    propsLayout->addWidget( new QLabel(tr("Preview")), 9, 0, 1, 2);

    QPushButton * dataPushButton = new QPushButton(tr("Data"));
    connect( dataPushButton, SIGNAL(clicked()), this, SLOT(previewData()));

    propsLayout->addWidget( dataPushButton, 9, 2);

    QPushButton * plotPushButton = new QPushButton(tr("Plot"));
    connect( plotPushButton, SIGNAL(clicked()), this, SLOT(previewPlot()));

    propsLayout->addWidget( plotPushButton, 9, 3);

    // Properties group box
    QGroupBox * propsGroupBox = new QGroupBox(tr("Properties"));
    propsGroupBox->setLayout(propsLayout);

    layout->addWidget( propsGroupBox, 0, 0);

    // Response Spectrum table
    m_respSpecTableBox = new TableGroupBox( 
            new ResponseSpectrumModel(m_model->rvtMotion()->targetRespSpec()), tr("Acceleration Response Spectrum"));
    connect( m_respSpecTableBox->table(), SIGNAL(dataPasted()), m_model->rvtMotion(), SLOT(setModified()));
    connect( m_respSpecTableBox, SIGNAL(dataChanged()), this, SIGNAL(hasChanged()));
    // Pasting data does not trigger data changed, so it has a separated signal
    connect( m_respSpecTableBox->table(), SIGNAL(dataPasted()), this, SIGNAL(hasChanged()));

    // Fourier Amplitude Spectrum table
    m_fourierSpecTableBox = new TableGroupBox( 
            new FourierSpectrumModel(m_model->rvtMotion(), true), tr("Fourier Amplitude Spectrum"));
    connect( m_fourierSpecTableBox, SIGNAL(dataChanged()), this, SIGNAL(hasChanged()));
    // Pasting data does not trigger data changed, so it has a separated signal
    connect( m_fourierSpecTableBox->table(), SIGNAL(dataPasted()), this, SIGNAL(hasChanged()));

    // Combine the spectra in a tabbed widget
    m_spectraStackedWidget = new QStackedWidget;
    m_spectraStackedWidget->addWidget( m_fourierSpecTableBox );
    m_spectraStackedWidget->addWidget( m_respSpecTableBox );

    layout->addWidget( m_spectraStackedWidget, 0, 1, 3, 1 );

    // Create the widget
    QWidget * widget = new QWidget;
    widget->setLayout(layout); 

    m_stackedWidget->addWidget(widget);
}

bool MotionPage::computeRvtResponse()
{
    if ( m_sourceComboBox->currentIndex() == RvtMotion::DefinedFourierSpectrum ) {
            if ( !m_model->rvtMotion()->modified() )
                return true;

            save();
            // Compute the period of the response spectrum
            double minPeriod;
            double maxPeriod;
            if ( m_model->rvtMotion()->freq().first() < m_model->rvtMotion()->freq().last() ) {
                maxPeriod = 1 / m_model->rvtMotion()->freq().first();
                minPeriod = 1 / m_model->rvtMotion()->freq().last();
            } else {
                minPeriod = 1 / m_model->rvtMotion()->freq().first();
                maxPeriod = 1 / m_model->rvtMotion()->freq().last();
            }
            // Set some bounds on the period
            if (minPeriod < 0.01 )
                minPeriod = 0.01;
            if ( 5.0 < maxPeriod )
                maxPeriod = 5.0;

            m_model->rvtMotion()->respSpec()->sa() = 
                m_model->rvtMotion()->computeSa( Motion::Rock, Dimension::logSpace( minPeriod, maxPeriod, 60 ), 5.0 );

            // Reset the modification flag
            m_model->rvtMotion()->setModified(false);
    } else {
        if ( !m_model->rvtMotion()->targetRespSpec()->modified() )
            return true;
        save();
        
        MyProgressDialog * dialog = new MyProgressDialog(tr("Computing Fourier amplitude spectrum..."));

        dialog->setModal(true);
        dialog->show();

        // Compute the inversion
        m_model->rvtMotion()->invert( dialog->okToContinue(), dialog->progressBar() );

        delete dialog;
    }
    //FIXME
    return true;
}

void MotionPage::previewData()
{
    if (computeRvtResponse() && m_dataDialog.isNull()) {
        QHBoxLayout * layout = new QHBoxLayout;

        // Response Spectrum table
        TableGroupBox * respSpecTableBox = new TableGroupBox( 
                new ResponseSpectrumModel(m_model->rvtMotion()->respSpec()), tr("Acceleration Response Spectrum"));
        respSpecTableBox->setEditable(false);
        layout->addWidget(respSpecTableBox);

        // Fourier Amplitude Spectrum table
        TableGroupBox * fourierSpecTableBox = new TableGroupBox( 
                new FourierSpectrumModel(m_model->rvtMotion(), false), tr("Fourier Amplitude Spectrum"));
        fourierSpecTableBox->setEditable(false);
        layout->addWidget(fourierSpecTableBox);

        m_dataDialog = new QDialog;
        m_dataDialog->setModal(true);
        m_dataDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        m_dataDialog->setLayout(layout);

        m_dataDialog->show();
    } 
}

void MotionPage::previewPlot()
{
    if (computeRvtResponse() && m_plotDialog.isNull()) {
        // FAS Plot
        QwtPlot * fasPlot = new QwtPlot;
        fasPlot->setAxisTitle( QwtPlot::xBottom, tr("Frequency (Hz)"));
        fasPlot->setAxisTitle( QwtPlot::yLeft, tr("Fourier Amplitude (g-s)"));
        fasPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
        fasPlot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLog10ScaleEngine);

        QwtPlotCurve * fasCurve = new QwtPlotCurve;
        fasCurve->setPen(QPen(Qt::red));
        fasCurve->setData( 
                m_model->rvtMotion()->freq().data(), 
                m_model->rvtMotion()->fas().data(), 
                m_model->rvtMotion()->freq().size() );
        fasCurve->attach(fasPlot);

        QHBoxLayout * fasLayout = new QHBoxLayout;
        fasLayout->addWidget(fasPlot);

        QGroupBox * fasGroupBox = new QGroupBox(tr("Fourier Amplitude Spectrum")); 
        fasGroupBox->setLayout(fasLayout);

        // Sa Plot
        QwtPlot * saPlot = new QwtPlot;
        saPlot->setAxisTitle( QwtPlot::xBottom, tr("Period (s)"));
        saPlot->setAxisTitle( QwtPlot::yLeft, tr("Spectral Accel. (g)"));
        saPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
        //saPlot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLog10ScaleEngine);

        // User defined points
        QwtPlotCurve * saPoints = new QwtPlotCurve;
        saPoints->setStyle(QwtPlotCurve::NoCurve);
        saPoints->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(Qt::darkRed), QSize(5,5)));
        saPoints->setData(
                m_model->rvtMotion()->targetRespSpec()->period().data(),
                m_model->rvtMotion()->targetRespSpec()->sa().data(),
                m_model->rvtMotion()->targetRespSpec()->period().size());
        saPoints->attach(saPlot);

        // Interpolated curve   
        QwtPlotCurve * saCurve = new QwtPlotCurve;
        saCurve->setPen(QPen(Qt::red));
        saCurve->setData( 
                m_model->rvtMotion()->respSpec()->period().data(), 
                m_model->rvtMotion()->respSpec()->sa().data(), 
                m_model->rvtMotion()->respSpec()->period().size() );
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
        m_plotDialog = new QDialog;
        m_plotDialog->setModal(true);
        m_plotDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        m_plotDialog->setLayout(layout);
        m_plotDialog->resize(1000,400);

        // Show the dialog 
        m_plotDialog->show();
    }
}

void MotionPage::load()
{
    // Location
    m_depthComboBox->setDepth(m_model->siteProfile().inputDepth());

    m_typeComboBox->setCurrentIndex(m_model->rvtMotion()->type());
    m_sourceComboBox->setCurrentIndex(m_model->rvtMotion()->source());

    m_durationSpinBox->setValue(m_model->rvtMotion()->duration());
    m_strainFactorSpinBox->setValue(m_model->rvtMotion()->strainFactor());
    m_soilFactorSpinBox->setValue(m_model->rvtMotion()->soilFactor());

    m_dampingSpinBox->setValue(m_model->rvtMotion()->targetRespSpec()->damping());
}

void MotionPage::save()
{
    m_model->siteProfile().setInputDepth( m_depthComboBox->depth());

    m_model->rvtMotion()->setType((Motion::Type)m_typeComboBox->currentIndex());
    m_model->rvtMotion()->setSource((RvtMotion::Source)m_sourceComboBox->currentIndex());

    m_model->rvtMotion()->setDuration(m_durationSpinBox->value());
    m_model->rvtMotion()->setStrainFactor(m_strainFactorSpinBox->value());
    m_model->rvtMotion()->setSoilFactor(m_soilFactorSpinBox->value());

    m_model->rvtMotion()->targetRespSpec()->setDamping(m_dampingSpinBox->value());
}

void MotionPage::setSource(int index)
{
    m_spectraStackedWidget->setCurrentIndex(index);

    // If response spectrum enabled, allow for damping to be edited
    bool enabled = (index ==1) ? true : false;

    m_dampingSpinBox->setEnabled(enabled);
}
