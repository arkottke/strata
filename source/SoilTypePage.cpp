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

#include "SoilTypePage.h"

#include "CustomNonlinearProperty.h"
#include "DampingFactory.h"
#include "ModulusFactory.h"
#include "MyTableView.h"
#include "NonlinearProperty.h"
#include "NonlinearPropertyCatalog.h"
#include "NonlinearPropertyDelegate.h"
#include "NonlinearPropertyRandomizer.h"
#include "NonlinearPropertyUncertaintyWidget.h"
#include "OnlyIncreasingDelegate.h"
#include "RockLayer.h"
#include "SiteResponseModel.h"
#include "SoilProfile.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "TableGroupBox.h"
#include "Units.h"

#include <QDebug>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <limits>


SoilTypePage::SoilTypePage(QWidget * parent, Qt::WindowFlags f )
    : AbstractPage(parent, f)
{
    // Set the layout
    QGridLayout * layout = new QGridLayout;
    
    layout->addWidget(createLayersGroupBox(), 0, 0, 2, 2);
    layout->addWidget(createBedrockGroupBox(), 2, 0);
    layout->addWidget(createWaterTableDepthGroupBox(), 2, 1);
    layout->addWidget(createVariationGroupBox(), 3, 0, 1, 2);
    layout->addWidget(createSoilPropsGroupBox(), 0, 2);
    layout->addWidget(createNlPropTableBox(), 1, 2, 3, 1);

    layout->setColumnStretch(0, 1);
    layout->setRowStretch(1, 1);

    setLayout(layout);
    // Connections
    connect( Units::instance(), SIGNAL(systemChanged(int)),
             this, SLOT(updateUnits()));
}

void SoilTypePage::setModel(SiteResponseModel *model)
{
    updateUnits();

    _soilTypeCatalog = model->siteProfile()->soilTypeCatalog();
    _soilTypeTableBox->setModel(_soilTypeCatalog);
    _modulusDelegate->setModel(
            _soilTypeCatalog->nlCatalog()->modulusFactory());
    _dampingDelegate->setModel(
            _soilTypeCatalog->nlCatalog()->dampingFactory());

    connect(_soilTypeTableBox, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(selectIndex(QModelIndex, QModelIndex)));

    _soilPropsGroupBox->setEnabled(false);
    _nlPropTableBox->setEnabled(false);

    updateNonlinearPropertiesRequired(model->nonlinearPropertiesRequired());
    connect(model, SIGNAL(nonlinearPropertiesRequiredChanged(bool)),
            this, SLOT(updateNonlinearPropertiesRequired(bool)));

    updateDampingRequired(model->dampingRequired());
    connect(model, SIGNAL(dampingRequiredChanged(bool)),
            this, SLOT(updateDampingRequired(bool)));

    updateVariedColumn(model->siteProfile()->nonlinearPropertyRandomizer()->enabled());
    connect(model->siteProfile()->nonlinearPropertyRandomizer(), SIGNAL(enabledChanged(bool)),
            this, SLOT(updateVariedColumn(bool)));

    RockLayer* rl = model->siteProfile()->bedrock();

    _bedrockUntWtSpinBox->setValue(rl->untWt());
    connect(_bedrockUntWtSpinBox, SIGNAL(valueChanged(double)),
            rl, SLOT(setUntWt(double)));

    _bedrockDampingSpinBox->setValue(rl->avgDamping());
    connect(_bedrockDampingSpinBox, SIGNAL(valueChanged(double)),
            rl, SLOT(setAvgDamping(double)));

    _waterTableDepthSpinBox->setValue(
            model->siteProfile()->waterTableDepth());
    connect(_waterTableDepthSpinBox, SIGNAL(valueChanged(double)),
                model->siteProfile(), SLOT(setWaterTableDepth(double)));

    NonlinearPropertyRandomizer* npr
            = model->siteProfile()->nonlinearPropertyRandomizer();

    _varyBedrockDampingCheckBox->setChecked(npr->bedrockIsEnabled());
    connect(_varyBedrockDampingCheckBox, SIGNAL(toggled(bool)),
            npr, SLOT(setBedrockIsEnabled(bool)));

    _varyBedrockDampingCheckBox->setVisible(npr->enabled());
    connect(npr, SIGNAL(enabledChanged(bool)),
            _varyBedrockDampingCheckBox, SLOT(setVisible(bool)));

    _nprModelComboBox->setCurrentIndex(npr->model());
    connect(_nprModelComboBox, SIGNAL(currentIndexChanged(int)),
            npr, SLOT(setModel(int)));

    _modulusUncertWidget->setUncertaintyModel(npr->model());
    _modulusUncertWidget->setModel(npr->modulusUncert());
    connect(npr, SIGNAL(modelChanged(int)),
            _modulusUncertWidget, SLOT(setUncertaintyModel(int)));

    _dampingUncertWidget->setUncertaintyModel(npr->model());
    _dampingUncertWidget->setModel(npr->dampingUncert());
    connect(npr, SIGNAL(modelChanged(int)),
            _dampingUncertWidget, SLOT(setUncertaintyModel(int)));

    _correlSpinBox->setValue(npr->correl());
    connect(_correlSpinBox, SIGNAL(valueChanged(double)),
            npr, SLOT(setCorrel(double)));

    _randomizerGroupBox->setVisible(npr->enabled());
    connect(npr, SIGNAL(enabledChanged(bool)),
            _randomizerGroupBox, SLOT(setVisible(bool)));
}

void SoilTypePage::setReadOnly(bool readOnly)
{
    _readOnly = readOnly;

    _soilTypeTableBox->setReadOnly(readOnly);
    _nlPropTableBox->setReadOnly(readOnly);

    _bedrockUntWtSpinBox->setReadOnly(readOnly);
    _bedrockDampingSpinBox->setReadOnly(readOnly);
    _varyBedrockDampingCheckBox->setDisabled(readOnly);

    _waterTableDepthSpinBox->setReadOnly(readOnly);

    _nprModelComboBox->setDisabled(readOnly);
    _modulusUncertWidget->setReadOnly(readOnly);
    _dampingUncertWidget->setReadOnly(readOnly);
    _correlSpinBox->setReadOnly(readOnly);

    _stressSpinBox->setReadOnly(readOnly);
    _piSpinBox->setReadOnly(readOnly);
    _ocrSpinBox->setReadOnly(readOnly);
    _freqSpinBox->setReadOnly(readOnly);
    _nCyclesSpinBox->setReadOnly(readOnly);
}

QGroupBox* SoilTypePage::createWaterTableDepthGroupBox()
{
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
    _waterTableDepthSpinBox = new QDoubleSpinBox;
    _waterTableDepthSpinBox->setRange(0, std::numeric_limits<double>::max());
    _waterTableDepthSpinBox->setDecimals(2);

    layout->addWidget(new QLabel(tr("Depth:")));
    layout->addWidget(_waterTableDepthSpinBox);

    // Create the group box
    QGroupBox* groupBox = new QGroupBox(tr("Water Table Depth"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox* SoilTypePage::createLayersGroupBox()
{
    _soilTypeTableBox = new TableGroupBox(tr("Soil Types"), this);
    _modulusDelegate = new NonlinearPropertyDelegate;
    _dampingDelegate = new NonlinearPropertyDelegate;

    _soilTypeTableBox->setItemDelegateForColumn(3, _modulusDelegate);
    _soilTypeTableBox->setItemDelegateForColumn(4, _dampingDelegate);

    return _soilTypeTableBox;
}

QGroupBox* SoilTypePage::createBedrockGroupBox()
{  
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
    _bedrockUntWtSpinBox = new QDoubleSpinBox;
    _bedrockUntWtSpinBox->setRange(10,200);
    _bedrockUntWtSpinBox->setDecimals(2);

    layout->addWidget(new QLabel(tr("Unit weight:")));
    layout->addWidget(_bedrockUntWtSpinBox);

    // Damping
    _bedrockDampingSpinBox = new QDoubleSpinBox;
    _bedrockDampingSpinBox->setSuffix(" %");
    _bedrockDampingSpinBox->setDecimals(2);
    _bedrockDampingSpinBox->setRange(0.1,5);
    _bedrockDampingSpinBox->setSingleStep(0.1);
    
    layout->addWidget(new QLabel(tr("Damping:")));
    layout->addWidget(_bedrockDampingSpinBox);
    
    // Damping variation in the bedrock
    _varyBedrockDampingCheckBox = new QCheckBox(tr("Vary the damping of the bedrock"));
    layout->addWidget(_varyBedrockDampingCheckBox);

    // Add a stretch to the end
    layout->addStretch(1);

    // Create the group box
    QGroupBox* groupBox = new QGroupBox(tr("Bedrock Layer"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox* SoilTypePage::createVariationGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(6, 1);

    // Model for the standard deviation
    _nprModelComboBox = new QComboBox;
    _nprModelComboBox->addItems(NonlinearPropertyRandomizer::modelList());

    // Link for help on standard deviation models
    QLabel* label = new QLabel(tr("Standard deviation model:"));
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(_nprModelComboBox, 0, 2);

    // Modulus reduction parameters
    _modulusUncertWidget = new NonlinearPropertyUncertaintyWidget(
            tr("Normalized shear modulus (G/G_max):"), layout, this);
    _modulusUncertWidget->setDecimals(3);
    _modulusUncertWidget->setMaxRange(1, 2);
    _modulusUncertWidget->setMinRange(0.001, 0.40);

    _dampingUncertWidget = new NonlinearPropertyUncertaintyWidget(
            tr("Damping:"), layout, this);
    _dampingUncertWidget->setSuffix(" %");
    _dampingUncertWidget->setMaxRange(10., 30.);
    _dampingUncertWidget->setMinRange(0.01, 2);
    
    // Correlation
    _correlSpinBox = new QDoubleSpinBox;
    _correlSpinBox->setRange(-1, 1);
    _correlSpinBox->setDecimals(2);
    _correlSpinBox->setSingleStep(0.1);

    const int row = layout->rowCount();
    layout->addWidget(new QLabel(tr("G/G_max, Damping Correlation Coefficent (%1):").arg(QChar(0x03C1))),
                      row, 0, 1, 2);
    layout->addWidget(_correlSpinBox, row, 2);
    
    // Group box
    _randomizerGroupBox = new QGroupBox(tr("Nonlinear Curve Variation Parameters"));
    _randomizerGroupBox->setLayout(layout);

    return _randomizerGroupBox;
}

QGroupBox* SoilTypePage::createSoilPropsGroupBox()
{
    QFormLayout* layout = new QFormLayout;
    
    // Stress line
    _stressSpinBox = new QDoubleSpinBox;
    _stressSpinBox->setDecimals(2);
    _stressSpinBox->setRange( 0.1, 200);
    _stressSpinBox->setSingleStep(1);
    _stressSpinBox->setSuffix(" atm");

    layout->addRow(tr("Mean effective stress:"), _stressSpinBox);

    // Plasticity line
    _piSpinBox = new QDoubleSpinBox;
    _piSpinBox->setDecimals(0);
    _piSpinBox->setRange( 0, 200);
    _piSpinBox->setSingleStep(1);
    
    layout->addRow(tr("Plasticity Index:"), _piSpinBox);

    // OCR line
    _ocrSpinBox = new QDoubleSpinBox;
    _ocrSpinBox->setDecimals(2);
    _ocrSpinBox->setRange( 1, 20);
    _ocrSpinBox->setSingleStep(1);

    layout->addRow(tr("Over-consolidation ratio:"), _ocrSpinBox);

    // Frequency line
    _freqSpinBox = new QDoubleSpinBox;
    _freqSpinBox->setDecimals(1);
    _freqSpinBox->setRange( 0.1, 100);
    _freqSpinBox->setSingleStep(1);
    _freqSpinBox->setSuffix(" Hz");

    layout->addRow(tr("Excitation frequency:"), _freqSpinBox);
    
    // Cycles line
    _nCyclesSpinBox = new QSpinBox;
    _nCyclesSpinBox->setRange( 1, 100);

    layout->addRow(tr("Number of cycles:"), _nCyclesSpinBox);

    // Create the group box
    _soilPropsGroupBox = new QGroupBox(tr("Darendeli and Stokoe Model Parameters"));
    _soilPropsGroupBox->setLayout(layout);


    connect(this, SIGNAL(soilPropertiesNeeded(bool)),
            _soilPropsGroupBox, SLOT(setEnabled(bool)));

    return _soilPropsGroupBox;
}

QGroupBox* SoilTypePage::createNlPropTableBox()
{
    _nlPropTableBox = new TableGroupBox(tr("Nonlinear Property"));
    _nlPropTableBox->table()->setItemDelegateForColumn(0, new OnlyIncreasingDelegate);


    return _nlPropTableBox;
}

void SoilTypePage::updateUnits()
{
    _bedrockUntWtSpinBox->setSuffix(" " + Units::instance()->untWt());
    _waterTableDepthSpinBox->setSuffix(" " + Units::instance()->length());

    // Need to invalidate the size cache since setSuffix doesn't
    _bedrockUntWtSpinBox->setRange(
            _bedrockUntWtSpinBox->minimum(),
            _bedrockUntWtSpinBox->maximum());
}

void SoilTypePage::selectIndex(const QModelIndex &current, const QModelIndex &previous)
{
    if (!_soilTypeCatalog)
        return;

    if (previous.isValid()) {
        // Stop listening to the previous soil type
        disconnect(_soilTypeCatalog->soilType(previous.row()),
                   SIGNAL(modulusModelChanged(NonlinearProperty*)), 0, 0);
        disconnect(_soilTypeCatalog->soilType(previous.row()),
                   SIGNAL(dampingModelChanged(NonlinearProperty*)), 0, 0);
    }

    if (current.isValid()) {
        SoilType* const st = _soilTypeCatalog->soilType(current.row());
        NonlinearProperty* np = 0;

        if (current.column() == SoilTypeCatalog::ModulusModelColumn) {
            np = st->modulusModel();

            connect(st, SIGNAL(modulusModelChanged(NonlinearProperty*)),
                    this, SLOT(setCurrentNonlinearProperty(NonlinearProperty*)));
        } else if (current.column() == SoilTypeCatalog::DampingModelColumn) {
            np = st->dampingModel();

            connect(st, SIGNAL(dampingModelChanged(NonlinearProperty*)),
                    this, SLOT(setCurrentNonlinearProperty(NonlinearProperty*)));
        }

        _nlPropTableBox->setEnabled(np);
        if (np)
            setCurrentNonlinearProperty(np);        

        _soilPropsGroupBox->setEnabled(
                    st->requiresSoilProperties() && _nonlinearPropsRequired);

        if (st->requiresSoilProperties()) {
            // Clear the connections
            disconnect(_stressSpinBox, 0, 0, 0);

            _stressSpinBox->setValue(st->meanStress());
            connect(_stressSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setMeanStress(double)));

            disconnect(_piSpinBox, 0, 0, 0);
            _piSpinBox->setValue(st->pi());
            connect(_piSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setPi(double)));

            disconnect(_ocrSpinBox, 0, 0, 0);
            _ocrSpinBox->setValue(st->ocr());
            connect(_ocrSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setOcr(double)));

            disconnect(_freqSpinBox, 0, 0, 0);
            _freqSpinBox->setValue(st->freq());
            connect(_freqSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setFreq(double)));

            disconnect(_nCyclesSpinBox, 0, 0, 0);
            _nCyclesSpinBox->setValue(st->nCycles());
            connect(_nCyclesSpinBox, SIGNAL(valueChanged(int)),
                    st, SLOT(setNCycles(int)));
        }
    }
}

void SoilTypePage::setCurrentNonlinearProperty(NonlinearProperty* np)
{
    _nlPropTableBox->setModel(np);
    _nlPropTableBox->setReadOnly(
            _readOnly
            || (!_readOnly && !qobject_cast<CustomNonlinearProperty* const>(np)));
}

void SoilTypePage::updateDampingRequired(bool b)
{
    _soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::DampingColumn, !b);
}

void SoilTypePage::updateNonlinearPropertiesRequired(bool b)
{
    _nonlinearPropsRequired = b;

    _soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::ModulusModelColumn, !b);
    _soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::DampingModelColumn, !b);
}

void SoilTypePage::updateVariedColumn(bool show)
{
    _soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::IsVariedColumn, !show);
}
