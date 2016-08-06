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

#include "SoilTypePage.h"

#include "CustomNonlinearProperty.h"
#include "DampingFactory.h"
#include "DampingStandardDeviation.h"
#include "ModulusFactory.h"
#include "ModulusStandardDeviation.h"
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

    m_soilTypeCatalog = model->siteProfile()->soilTypeCatalog();
    m_soilTypeTableBox->setModel(m_soilTypeCatalog);
    m_modulusDelegate->setModel(
            m_soilTypeCatalog->nlCatalog()->modulusFactory());
    m_dampingDelegate->setModel(
            m_soilTypeCatalog->nlCatalog()->dampingFactory());

    connect(m_soilTypeTableBox, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(selectIndex(QModelIndex, QModelIndex)));

    m_soilPropsGroupBox->setEnabled(false);
    m_nlPropTableBox->setEnabled(false);

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

    m_bedrockUntWtSpinBox->setValue(rl->untWt());
    connect(m_bedrockUntWtSpinBox, SIGNAL(valueChanged(double)),
            rl, SLOT(setUntWt(double)));

    m_bedrockDampingSpinBox->setValue(rl->avgDamping());
    connect(m_bedrockDampingSpinBox, SIGNAL(valueChanged(double)),
            rl, SLOT(setAvgDamping(double)));

    m_waterTableDepthSpinBox->setValue(
            model->siteProfile()->waterTableDepth());
    connect(m_waterTableDepthSpinBox, SIGNAL(valueChanged(double)),
                model->siteProfile(), SLOT(setWaterTableDepth(double)));

    NonlinearPropertyRandomizer* npr
            = model->siteProfile()->nonlinearPropertyRandomizer();

    m_varyBedrockDampingCheckBox->setChecked(npr->bedrockIsEnabled());
    connect(m_varyBedrockDampingCheckBox, SIGNAL(toggled(bool)),
            npr, SLOT(setBedrockIsEnabled(bool)));

    m_varyBedrockDampingCheckBox->setVisible(npr->enabled());
    connect(npr, SIGNAL(enabledChanged(bool)),
            m_varyBedrockDampingCheckBox, SLOT(setVisible(bool)));

    m_nprModelComboBox->setCurrentIndex(npr->model());
    connect(m_nprModelComboBox, SIGNAL(currentIndexChanged(int)),
            npr, SLOT(setModel(int)));

    m_modulusUncertWidget->setUncertaintyModel(npr->model());
    m_modulusUncertWidget->setModel(npr->modulusUncert());
    connect(npr, SIGNAL(modelChanged(int)),
            m_modulusUncertWidget, SLOT(setUncertaintyModel(int)));

    m_dampingUncertWidget->setUncertaintyModel(npr->model());
    m_dampingUncertWidget->setModel(npr->dampingUncert());
    connect(npr, SIGNAL(modelChanged(int)),
            m_dampingUncertWidget, SLOT(setUncertaintyModel(int)));

    m_correlSpinBox->setValue(npr->correl());
    connect(m_correlSpinBox, SIGNAL(valueChanged(double)),
            npr, SLOT(setCorrel(double)));

    m_randomizerGroupBox->setVisible(npr->enabled());
    connect(npr, SIGNAL(enabledChanged(bool)),
            m_randomizerGroupBox, SLOT(setVisible(bool)));
}

void SoilTypePage::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;

    m_soilTypeTableBox->setReadOnly(readOnly);
    m_nlPropTableBox->setReadOnly(readOnly);

    m_bedrockUntWtSpinBox->setReadOnly(readOnly);
    m_bedrockDampingSpinBox->setReadOnly(readOnly);
    m_varyBedrockDampingCheckBox->setDisabled(readOnly);

    m_waterTableDepthSpinBox->setReadOnly(readOnly);

    m_nprModelComboBox->setDisabled(readOnly);
    m_modulusUncertWidget->setReadOnly(readOnly);
    m_dampingUncertWidget->setReadOnly(readOnly);
    m_correlSpinBox->setReadOnly(readOnly);

    m_stressSpinBox->setReadOnly(readOnly);
    m_piSpinBox->setReadOnly(readOnly);
    m_ocrSpinBox->setReadOnly(readOnly);
    m_freqSpinBox->setReadOnly(readOnly);
    m_nCyclesSpinBox->setReadOnly(readOnly);
}

QGroupBox* SoilTypePage::createWaterTableDepthGroupBox()
{
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
    m_waterTableDepthSpinBox = new QDoubleSpinBox;
    m_waterTableDepthSpinBox->setRange(0, std::numeric_limits<double>::max());
    m_waterTableDepthSpinBox->setDecimals(2);

    layout->addWidget(new QLabel(tr("Depth:")));
    layout->addWidget(m_waterTableDepthSpinBox);

    // Create the group box
    QGroupBox* groupBox = new QGroupBox(tr("Water Table Depth"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox* SoilTypePage::createLayersGroupBox()
{
    m_soilTypeTableBox = new TableGroupBox(tr("Soil Types"), this);
    m_modulusDelegate = new NonlinearPropertyDelegate;
    m_dampingDelegate = new NonlinearPropertyDelegate;

    m_soilTypeTableBox->setItemDelegateForColumn(3, m_modulusDelegate);
    m_soilTypeTableBox->setItemDelegateForColumn(4, m_dampingDelegate);

    return m_soilTypeTableBox;
}

QGroupBox* SoilTypePage::createBedrockGroupBox()
{  
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
    m_bedrockUntWtSpinBox = new QDoubleSpinBox;
    m_bedrockUntWtSpinBox->setRange(10,200);
    m_bedrockUntWtSpinBox->setDecimals(2);

    layout->addWidget(new QLabel(tr("Unit weight:")));
    layout->addWidget(m_bedrockUntWtSpinBox);

    // Damping
    m_bedrockDampingSpinBox = new QDoubleSpinBox;
    m_bedrockDampingSpinBox->setSuffix(" %");
    m_bedrockDampingSpinBox->setDecimals(2);
    m_bedrockDampingSpinBox->setRange(0.1,5);
    m_bedrockDampingSpinBox->setSingleStep(0.1);
    
    layout->addWidget(new QLabel(tr("Damping:")));
    layout->addWidget(m_bedrockDampingSpinBox);
    
    // Damping variation in the bedrock
    m_varyBedrockDampingCheckBox = new QCheckBox(tr("Vary the damping of the bedrock"));
    layout->addWidget(m_varyBedrockDampingCheckBox);

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
    m_nprModelComboBox = new QComboBox;
    m_nprModelComboBox->addItems(NonlinearPropertyRandomizer::modelList());

    // Link for help on standard deviation models
    QLabel* label = new QLabel(tr("Standard deviation model:"));
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(m_nprModelComboBox, 0, 2);

    // Modulus reduction parameters
    m_modulusUncertWidget = new NonlinearPropertyUncertaintyWidget(
            tr("Normalized shear modulus (G/G_max):"), layout, this);
    m_modulusUncertWidget->setDecimals(3);
    m_modulusUncertWidget->setMaxRange(1, 2);
    m_modulusUncertWidget->setMinRange(0.001, 0.40);

    m_dampingUncertWidget = new NonlinearPropertyUncertaintyWidget(
            tr("Damping:"), layout, this);
    m_dampingUncertWidget->setSuffix(" %");
    m_dampingUncertWidget->setMaxRange(10., 50.);
    m_dampingUncertWidget->setMinRange(0.01, 2);
    
    // Correlation
    m_correlSpinBox = new QDoubleSpinBox;
    m_correlSpinBox->setRange(-1, 1);
    m_correlSpinBox->setDecimals(2);
    m_correlSpinBox->setSingleStep(0.1);

    const int row = layout->rowCount();
    layout->addWidget(new QLabel(tr("G/G_max, Damping Correlation Coefficent (%1):").arg(QChar(0x03C1))),
                      row, 0, 1, 2);
    layout->addWidget(m_correlSpinBox, row, 2);
    
    // Group box
    m_randomizerGroupBox = new QGroupBox(tr("Nonlinear Curve Variation Parameters"));
    m_randomizerGroupBox->setLayout(layout);

    return m_randomizerGroupBox;
}

QGroupBox* SoilTypePage::createSoilPropsGroupBox()
{
    QFormLayout* layout = new QFormLayout;
    
    // Stress line
    m_stressSpinBox = new QDoubleSpinBox;
    m_stressSpinBox->setDecimals(2);
    m_stressSpinBox->setRange( 0.1, 200);
    m_stressSpinBox->setSingleStep(1);
    m_stressSpinBox->setSuffix(" atm");

    layout->addRow(tr("Mean effective stress:"), m_stressSpinBox);

    // Plasticity line
    m_piSpinBox = new QDoubleSpinBox;
    m_piSpinBox->setDecimals(0);
    m_piSpinBox->setRange( 0, 200);
    m_piSpinBox->setSingleStep(1);
    
    layout->addRow(tr("Plasticity Index:"), m_piSpinBox);

    // OCR line
    m_ocrSpinBox = new QDoubleSpinBox;
    m_ocrSpinBox->setDecimals(2);
    m_ocrSpinBox->setRange( 1, 20);
    m_ocrSpinBox->setSingleStep(1);

    layout->addRow(tr("Over-consolidation ratio:"), m_ocrSpinBox);

    // Frequency line
    m_freqSpinBox = new QDoubleSpinBox;
    m_freqSpinBox->setDecimals(1);
    m_freqSpinBox->setRange( 0.1, 100);
    m_freqSpinBox->setSingleStep(1);
    m_freqSpinBox->setSuffix(" Hz");

    layout->addRow(tr("Excitation frequency:"), m_freqSpinBox);
    
    // Cycles line
    m_nCyclesSpinBox = new QSpinBox;
    m_nCyclesSpinBox->setRange( 1, 100);

    layout->addRow(tr("Number of cycles:"), m_nCyclesSpinBox);

    // Create the group box
    m_soilPropsGroupBox = new QGroupBox(tr("Darendeli and Stokoe Model Parameters"));
    m_soilPropsGroupBox->setLayout(layout);


    connect(this, SIGNAL(soilPropertiesNeeded(bool)),
            m_soilPropsGroupBox, SLOT(setEnabled(bool)));

    return m_soilPropsGroupBox;
}

QGroupBox* SoilTypePage::createNlPropTableBox()
{
    m_nlPropTableBox = new TableGroupBox(tr("Nonlinear Property"));
    m_nlPropTableBox->table()->setItemDelegateForColumn(0, new OnlyIncreasingDelegate);


    return m_nlPropTableBox;
}

void SoilTypePage::updateUnits()
{
    m_bedrockUntWtSpinBox->setSuffix(" " + Units::instance()->untWt());
    m_waterTableDepthSpinBox->setSuffix(" " + Units::instance()->length());

    // Need to invalidate the size cache since setSuffix doesn't
    m_bedrockUntWtSpinBox->setRange(
            m_bedrockUntWtSpinBox->minimum(),
            m_bedrockUntWtSpinBox->maximum());
}

void SoilTypePage::selectIndex(const QModelIndex &current, const QModelIndex &previous)
{
    if (!m_soilTypeCatalog)
        return;

    if (previous.isValid()) {
        // Stop listening to the previous soil type
        disconnect(m_soilTypeCatalog->soilType(previous.row()),
                   SIGNAL(modulusModelChanged(NonlinearProperty*)), 0, 0);
        disconnect(m_soilTypeCatalog->soilType(previous.row()),
                   SIGNAL(dampingModelChanged(NonlinearProperty*)), 0, 0);
    }

    if (current.isValid()) {
        SoilType* const st = m_soilTypeCatalog->soilType(current.row());
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

        m_nlPropTableBox->setEnabled(np);
        if (np)
            setCurrentNonlinearProperty(np);        

        m_soilPropsGroupBox->setEnabled(st->requiresSoilProperties());

        if (st->requiresSoilProperties()) {
            // Clear the connections
            disconnect(m_stressSpinBox, 0, 0, 0);

            m_stressSpinBox->setValue(st->meanStress());
            connect(m_stressSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setMeanStress(double)));

            disconnect(m_piSpinBox, 0, 0, 0);
            m_piSpinBox->setValue(st->pi());
            connect(m_piSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setPi(double)));

            disconnect(m_ocrSpinBox, 0, 0, 0);
            m_ocrSpinBox->setValue(st->ocr());
            connect(m_ocrSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setOcr(double)));

            disconnect(m_freqSpinBox, 0, 0, 0);
            m_freqSpinBox->setValue(st->freq());
            connect(m_freqSpinBox, SIGNAL(valueChanged(double)),
                    st, SLOT(setFreq(double)));

            disconnect(m_nCyclesSpinBox, 0, 0, 0);
            m_nCyclesSpinBox->setValue(st->nCycles());
            connect(m_nCyclesSpinBox, SIGNAL(valueChanged(int)),
                    st, SLOT(setNCycles(int)));
        }
    }
}

void SoilTypePage::setCurrentNonlinearProperty(NonlinearProperty* np)
{
    m_nlPropTableBox->setModel(np);
    m_nlPropTableBox->setReadOnly(
            m_readOnly
            || (!m_readOnly && !qobject_cast<CustomNonlinearProperty* const>(np)));
}

void SoilTypePage::updateDampingRequired(bool b)
{
    m_soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::DampingColumn, !b);
}

void SoilTypePage::updateNonlinearPropertiesRequired(bool b)
{
    m_soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::ModulusModelColumn, !b);
    m_soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::DampingModelColumn, !b);
}

void SoilTypePage::updateVariedColumn(bool show)
{
    m_soilTypeTableBox->setColumnHidden(
            SoilTypeCatalog::IsVariedColumn, !show);
}
