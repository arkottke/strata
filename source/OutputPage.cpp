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

#include "OutputPage.h"

#include "Dimension.h"
#include "DimensionLayout.h"
#include "MotionLibrary.h"
#include "OutputTableFrame.h"
#include "OutputCatalog.h"
#include "ProfilesOutputCatalog.h"
#include "RatiosOutputCatalog.h"
#include "SiteResponseModel.h"
#include "SoilTypesOutputCatalog.h"
#include "SpectraOutputCatalog.h"
#include "TextLog.h"
#include "TimeSeriesOutputCatalog.h"

#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>

OutputPage::OutputPage(QWidget * parent, Qt::WindowFlags f)
    : AbstractPage(parent,f)
{
    m_profilesTableView = new QTableView;
    m_timeSeriesTableFrame = new OutputTableFrame;
    m_spectraTableFrame = new OutputTableFrame;
    m_ratiosTableFrame = new OutputTableFrame;
    m_soilTypesTableView = new QTableView;

    QGridLayout* layout = new QGridLayout;    
    layout->setRowStretch(3, 1);

    // Tab widget
    // Left Column
    m_tabWidget = new QTabWidget;

    m_tabWidget->addTab(m_profilesTableView, tr("Profiles"));
    m_tabWidget->addTab(m_timeSeriesTableFrame, tr("Time Series"));
    m_tabWidget->addTab(m_spectraTableFrame, tr("Response and Fourier Spectra"));
    m_tabWidget->addTab(m_ratiosTableFrame, tr("Ratios"));
    m_tabWidget->addTab(m_soilTypesTableView, tr("Soil Types"));

    layout->addWidget(m_tabWidget, 0, 0, 4, 1);
    layout->addWidget(createRespSpecGroupBox(), 0, 1);

    layout->addWidget(createFreqGroupBox(), 1, 1);
    layout->addWidget(createLogGroupBox(), 2, 1);

    // Set general layout
    setLayout(layout);
}

void OutputPage::setModel(SiteResponseModel *model)
{    
    OutputCatalog* oc = model->outputCatalog();

    m_profilesTableView->setModel(oc->profilesCatalog());
    m_profilesTableView->resizeColumnsToContents();
    m_profilesTableView->resizeRowsToContents();

    m_timeSeriesTableFrame->setModel(oc->timeSeriesCatalog());
    m_spectraTableFrame->setModel(oc->spectraCatalog());
    m_ratiosTableFrame->setModel(oc->ratiosCatalog());
    m_soilTypesTableView->setModel(oc->soilTypesCatalog());

    m_respSpecGroupBox->setVisible(oc->periodIsNeeded());
    connect(oc, SIGNAL(periodIsNeededChanged(bool)),
            m_respSpecGroupBox, SLOT(setVisible(bool)));

    m_dampingSpinBox->setValue(oc->damping());
    connect( m_dampingSpinBox, SIGNAL(valueChanged(double)),
             oc, SLOT(setDamping(double)));

    m_periodLayout->setModel(oc->period());

    m_freqGroupBox->setVisible(oc->frequencyIsNeeded());
    connect(oc, SIGNAL(frequencyIsNeededChanged(bool)),
            m_freqGroupBox, SLOT(setVisible(bool)));

    m_frequencyLayout->setModel(oc->frequency());

    m_logLevelComboBox->setCurrentIndex(oc->log()->level());
    connect(m_logLevelComboBox, SIGNAL(currentIndexChanged(int)),
            oc->log(), SLOT(setLevel(int)));

    setApproach(model->motionLibrary()->approach());
    connect(model->motionLibrary(), SIGNAL(approachChanged(int)),
            this, SLOT(setApproach(int)));
}

void OutputPage::setReadOnly(bool readOnly)
{
    m_timeSeriesTableFrame->setReadOnly(readOnly);
    m_spectraTableFrame->setReadOnly(readOnly);
    m_ratiosTableFrame->setReadOnly(readOnly);

    m_dampingSpinBox->setReadOnly(readOnly);
    m_periodLayout->setReadOnly(readOnly);

    m_frequencyLayout->setReadOnly(readOnly);

    m_logLevelComboBox->setDisabled(readOnly);
}

void OutputPage::setApproach(int approach)
{
    const int tsIndex = 1;
    const bool enabled = (MotionLibrary::Approach)approach == MotionLibrary::TimeSeries;

    m_tabWidget->setTabEnabled(tsIndex, enabled);

    if (!enabled && m_tabWidget->currentIndex() == tsIndex)
        m_tabWidget->setCurrentIndex(0);
}

QGroupBox* OutputPage::createRespSpecGroupBox()
{
    // Damping
    m_dampingSpinBox = new QDoubleSpinBox;
    m_dampingSpinBox->setSuffix(" %");
    m_dampingSpinBox->setRange(1, 50);
    m_dampingSpinBox->setSingleStep(1);
    m_dampingSpinBox->setDecimals(1);

    m_periodLayout = new DimensionLayout(this);
    m_periodLayout->setRange(0.001, 100);
    m_periodLayout->setSuffix(" s");

    m_periodLayout->insertRow(0, "Damping:", m_dampingSpinBox);

    m_respSpecGroupBox = new QGroupBox(tr("Response Spectrum Properties"), this);
    m_respSpecGroupBox->setLayout(m_periodLayout);

    return m_respSpecGroupBox;
}

QGroupBox* OutputPage::createFreqGroupBox()
{
    m_frequencyLayout = new DimensionLayout(this);
    m_frequencyLayout->setRange(0.001, 1000);
    m_frequencyLayout->setSuffix(" Hz");

    m_freqGroupBox = new QGroupBox(tr("Frequency Properties"), this);
    m_freqGroupBox->setLayout(m_frequencyLayout);

    return m_freqGroupBox;
}

QGroupBox* OutputPage::createLogGroupBox()
{
    QFormLayout* layout = new QFormLayout;

    // Logging combo box
    m_logLevelComboBox = new QComboBox;
    m_logLevelComboBox->addItems(TextLog::levelList());

    layout->addRow(tr("Logging level:"), m_logLevelComboBox);

    m_logGroupBox = new QGroupBox(tr("Logging Properties"));
    m_logGroupBox->setLayout(layout);

    return m_logGroupBox;
}
