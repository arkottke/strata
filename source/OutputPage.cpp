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
    _profilesTableView = new QTableView;
    _timeSeriesTableFrame = new OutputTableFrame;
    _spectraTableFrame = new OutputTableFrame;
    _ratiosTableFrame = new OutputTableFrame;
    _soilTypesTableView = new QTableView;

    auto *layout = new QGridLayout;
    layout->setRowStretch(3, 1);

    // Tab widget
    // Left Column
    _tabWidget = new QTabWidget;

    _tabWidget->addTab(_profilesTableView, tr("Profiles"));
    _tabWidget->addTab(_timeSeriesTableFrame, tr("Time Series"));
    _tabWidget->addTab(_spectraTableFrame, tr("Response and Fourier Spectra"));
    _tabWidget->addTab(_ratiosTableFrame, tr("Ratios"));
    _tabWidget->addTab(_soilTypesTableView, tr("Soil Types"));

    layout->addWidget(_tabWidget, 0, 0, 4, 1);
    layout->addWidget(createRespSpecGroupBox(), 0, 1);

    layout->addWidget(createFreqGroupBox(), 1, 1);
    layout->addWidget(createLogGroupBox(), 2, 1);

    // Set general layout
    setLayout(layout);
}

void OutputPage::setModel(SiteResponseModel *model)
{    
    OutputCatalog* oc = model->outputCatalog();

    _profilesTableView->setModel(oc->profilesCatalog());
    _profilesTableView->resizeColumnsToContents();
    _profilesTableView->resizeRowsToContents();

    _timeSeriesTableFrame->setModel(oc->timeSeriesCatalog());
    _spectraTableFrame->setModel(oc->spectraCatalog());
    _ratiosTableFrame->setModel(oc->ratiosCatalog());
    _soilTypesTableView->setModel(oc->soilTypesCatalog());

    _respSpecGroupBox->setVisible(oc->periodIsNeeded());
    connect(oc, SIGNAL(periodIsNeededChanged(bool)),
            _respSpecGroupBox, SLOT(setVisible(bool)));

    _dampingSpinBox->setValue(oc->damping());
    connect( _dampingSpinBox, SIGNAL(valueChanged(double)),
             oc, SLOT(setDamping(double)));

    _periodLayout->setModel(oc->period());

    _freqGroupBox->setVisible(oc->frequencyIsNeeded());
    connect(oc, SIGNAL(frequencyIsNeededChanged(bool)),
            _freqGroupBox, SLOT(setVisible(bool)));

    _frequencyLayout->setModel(oc->frequency());

    _logLevelComboBox->setCurrentIndex(oc->log()->level());
    connect(_logLevelComboBox, SIGNAL(currentIndexChanged(int)),
            oc->log(), SLOT(setLevel(int)));

    setApproach(model->motionLibrary()->approach());
    connect(model->motionLibrary(), SIGNAL(approachChanged(int)),
            this, SLOT(setApproach(int)));
}

void OutputPage::setReadOnly(bool readOnly)
{
    _timeSeriesTableFrame->setReadOnly(readOnly);
    _spectraTableFrame->setReadOnly(readOnly);
    _ratiosTableFrame->setReadOnly(readOnly);

    _dampingSpinBox->setReadOnly(readOnly);
    _periodLayout->setReadOnly(readOnly);

    _frequencyLayout->setReadOnly(readOnly);

    _logLevelComboBox->setDisabled(readOnly);
}

void OutputPage::setApproach(int approach)
{
    const int tsIndex = 1;
    const bool enabled = (MotionLibrary::Approach)approach == MotionLibrary::TimeSeries;

    _tabWidget->setTabEnabled(tsIndex, enabled);

    if (!enabled && _tabWidget->currentIndex() == tsIndex)
        _tabWidget->setCurrentIndex(0);
}

auto OutputPage::createRespSpecGroupBox() -> QGroupBox*
{
    // Damping
    _dampingSpinBox = new QDoubleSpinBox;
    _dampingSpinBox->setSuffix(" %");
    _dampingSpinBox->setRange(1, 50);
    _dampingSpinBox->setSingleStep(1);
    _dampingSpinBox->setDecimals(1);

    _periodLayout = new DimensionLayout(this);
    _periodLayout->setRange(0.001, 100);
    _periodLayout->setSuffix(" s");

    _periodLayout->insertRow(0, "Damping:", _dampingSpinBox);

    _respSpecGroupBox = new QGroupBox(tr("Response Spectrum Properties"), this);
    _respSpecGroupBox->setLayout(_periodLayout);

    return _respSpecGroupBox;
}

auto OutputPage::createFreqGroupBox() -> QGroupBox*
{
    _frequencyLayout = new DimensionLayout(this);
    _frequencyLayout->setRange(0.001, 1000);
    _frequencyLayout->setSuffix(" Hz");

    _freqGroupBox = new QGroupBox(tr("Frequency Properties"), this);
    _freqGroupBox->setLayout(_frequencyLayout);

    return _freqGroupBox;
}

auto OutputPage::createLogGroupBox() -> QGroupBox*
{
    auto *layout = new QFormLayout;

    // Logging combo box
    _logLevelComboBox = new QComboBox;
    _logLevelComboBox->addItems(TextLog::levelList());

    layout->addRow(tr("Logging level:"), _logLevelComboBox);

    _logGroupBox = new QGroupBox(tr("Logging Properties"));
    _logGroupBox->setLayout(layout);

    return _logGroupBox;
}
