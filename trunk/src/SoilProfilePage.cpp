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

#include "SoilProfilePage.h"

#include "AbstractDistribution.h"
#include "BedrockDepthVariation.h"
#include "LayerThicknessVariation.h"
#include "MyTableView.h"
#include "ProfileRandomizer.h"
#include "SoilProfile.h"
#include "SiteResponseModel.h"
#include "SoilTypeDelegate.h"
#include "TableGroupBox.h"
#include "Units.h"
#include "VelocityVariation.h"


#include <QDebug>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

SoilProfilePage::SoilProfilePage(QWidget * parent, Qt::WindowFlags f)
    : AbstractPage(parent, f)
{
    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(createTableGroupBox(), 1);
    layout->addWidget(createProfileRandomizerGroupBox());
    
    setLayout(layout);

    // Connections
    connect( Units::instance(), SIGNAL(systemChanged(int)),
             this, SLOT(updateUnits()));

}

void SoilProfilePage::setModel(SiteResponseModel *model)
{
    m_soilProfileTableGroup->setModel(model->siteProfile());
    m_soilTypeDelegate->setCatalog(model->siteProfile()->soilTypeCatalog());

    m_velocityVariation = model->siteProfile()->profileRandomizer()->velocityVariation();
    updateHiddenColumns();
    connect(m_velocityVariation, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateHiddenColumns()));

    ProfileRandomizer* pr = model->siteProfile()->profileRandomizer();

    m_profileVariationGroupBox->setVisible(pr->enabled());
    connect(pr, SIGNAL(enabledChanged(bool)),
            m_profileVariationGroupBox, SLOT(setVisible(bool)));

    m_isVelocityVariedCheckBox->setChecked(pr->velocityVariation()->enabled());
    connect(m_isVelocityVariedCheckBox, SIGNAL(toggled(bool)),
            pr->velocityVariation(), SLOT(setEnabled(bool)));
    connect(pr->velocityVariation(), SIGNAL(enabledChanged(bool)),
            m_isVelocityVariedCheckBox, SLOT(setChecked(bool)));

    m_isLayeringVariedCheckBox->setChecked(pr->layerThicknessVariation()->enabled());
    connect(m_isLayeringVariedCheckBox, SIGNAL(toggled(bool)),
            pr->layerThicknessVariation(), SLOT(setEnabled(bool)));
    connect(pr->layerThicknessVariation(), SIGNAL(enabledChanged(bool)),
            m_isLayeringVariedCheckBox, SLOT(setChecked(bool)));

    m_isBedrockDepthVariedCheckBox->setChecked(pr->bedrockDepthVariation()->enabled());
    connect(m_isBedrockDepthVariedCheckBox, SIGNAL(toggled(bool)),
            pr->bedrockDepthVariation(), SLOT(setEnabled(bool)));
    connect(pr->bedrockDepthVariation(), SIGNAL(enabledChanged(bool)),
            m_isBedrockDepthVariedCheckBox, SLOT(setChecked(bool)));


    // Velocity Variation
    VelocityVariation* vv =
            model->siteProfile()->profileRandomizer()->velocityVariation();

    setVelocityItemEnabled(vv->enabled());
    connect(vv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setVelocityItemEnabled(bool)));

    m_velocityVariationFrame->setEnabled(vv->enabled());
    connect(vv, SIGNAL(enabledChanged(bool)),
            m_velocityVariationFrame, SLOT(setEnabled(bool)));

    m_layerSpecificCheckBox->setChecked(vv->stdevIsLayerSpecific());
    connect(m_layerSpecificCheckBox, SIGNAL(toggled(bool)),
            vv, SLOT(setStdevIsLayerSpecific(bool)));
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            m_layerSpecificCheckBox, SLOT(setChecked(bool)));
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            this, SLOT(updateHiddenColumns()));

    m_stdevModelComboBox->setCurrentIndex(vv->stdevModel());
    connect(m_stdevModelComboBox, SIGNAL(currentIndexChanged(int)),
             vv, SLOT(setStdevModel(int)));

    m_stdevModelComboBox->setDisabled(vv->stdevIsLayerSpecific());
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            m_stdevModelComboBox, SLOT(setDisabled(bool)));

    m_stdevSpinBox->setValue(vv->stdev());
    connect(m_stdevSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setStdev(double)));
    connect(vv, SIGNAL(stdevChanged(double)),
            m_stdevSpinBox, SLOT(setValue(double)));

    m_stdevSpinBox->setEnabled(vv->stdevCustomEnabled());
    connect(vv, SIGNAL(stdevCustomEnabledChanged(bool)),
            m_stdevSpinBox, SLOT(setEnabled(bool)));

    m_correlModelComboBox->setCurrentIndex(vv->stdevModel());
    connect(m_correlModelComboBox, SIGNAL(currentIndexChanged(int)),
             vv, SLOT(setCorrelModel(int)));

    m_correlInitialSpinBox->setValue(vv->correlInitial());
    connect(m_correlInitialSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelInitial(double)));
    connect(vv, SIGNAL(correlInitialChanged(double)),
            m_correlInitialSpinBox, SLOT(setValue(double)));

    m_correlFinalSpinBox->setValue(vv->correlFinal());
    connect(m_correlFinalSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelFinal(double)));
    connect(vv, SIGNAL(correlFinalChanged(double)),
            m_correlFinalSpinBox, SLOT(setValue(double)));

    m_correlDeltaSpinBox->setValue(vv->correlDelta());
    connect(m_correlDeltaSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelDelta(double)));
    connect(vv, SIGNAL(correlDeltaChanged(double)),
            m_correlDeltaSpinBox, SLOT(setValue(double)));

    m_correlInterceptSpinBox->setValue(vv->correlIntercept());
    connect(m_correlInterceptSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelIntercept(double)));
    connect(vv, SIGNAL(correlInterceptChanged(double)),
            m_correlInterceptSpinBox, SLOT(setValue(double)));

    m_correlExponentSpinBox->setValue(vv->correlExponent());
    connect(m_correlExponentSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelExponent(double)));
    connect(vv, SIGNAL(correlExponentChanged(double)),
            m_correlExponentSpinBox, SLOT(setValue(double)));

    m_correlGroupBox->setEnabled(vv->correlCustomEnabled());
    connect(vv, SIGNAL(correlCustomEnabledChanged(bool)),
            m_correlGroupBox, SLOT(setEnabled(bool)));

    // Layering variation
    LayerThicknessVariation* ltv =
            model->siteProfile()->profileRandomizer()->layerThicknessVariation();

    m_layerVariationFrame->setEnabled(ltv->enabled());
    connect(ltv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setLayeringItemEnabled(bool)));
    connect(ltv, SIGNAL(enabledChanged(bool)),
            m_layerVariationFrame, SLOT(setEnabled(bool)));

    m_layeringModelComboBox->setCurrentIndex(ltv->model());
    connect(m_layeringModelComboBox, SIGNAL(currentIndexChanged(int)),
            ltv, SLOT(setModel(int)));

    m_layeringCoeffSpinBox->setValue(ltv->coeff());
    connect(m_layeringCoeffSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setCoeff(double)));
    connect(ltv, SIGNAL(coeffChanged(double)),
            m_layeringCoeffSpinBox, SLOT(setValue(double)));

    m_layeringCoeffSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            m_layeringCoeffSpinBox, SLOT(setEnabled(bool)));

    m_layeringInitialSpinBox->setValue(ltv->initial());
    connect(m_layeringInitialSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setInitial(double)));
    connect(ltv, SIGNAL(initialChanged(double)),
            m_layeringInitialSpinBox, SLOT(setValue(double)));

    m_layeringInitialSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            m_layeringInitialSpinBox, SLOT(setEnabled(bool)));

    m_layeringExponentSpinBox->setValue(ltv->exponent());
    connect(m_layeringExponentSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setExponent(double)));
    connect(ltv, SIGNAL(exponentChanged(double)),
            m_layeringExponentSpinBox, SLOT(setValue(double)));

    m_layeringExponentSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            m_layeringExponentSpinBox, SLOT(setEnabled(bool)));

    // Bedrock depth variation
    BedrockDepthVariation* bdv =
            model->siteProfile()->profileRandomizer()->bedrockDepthVariation();

    m_bedrockDepthFrame->setEnabled(bdv->enabled());
    connect(bdv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setBedrockDepthItemEnabled(bool)));

    m_bedrockDepthFrame->setEnabled(bdv->enabled());
    connect(bdv, SIGNAL(enabledChanged(bool)),
            m_bedrockDepthFrame, SLOT(setEnabled(bool)));

    m_bedrockModelComboBox->setCurrentIndex(bdv->type());
    connect(m_bedrockModelComboBox, SIGNAL(currentIndexChanged(int)),
            bdv, SLOT(setType(int)));

    m_bedrockStdevSpinBox->setValue(bdv->stdev());
    connect(m_bedrockStdevSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setStdev(double)));
    connect(bdv, SIGNAL(stdevChanged(double)),
            m_bedrockStdevSpinBox, SLOT(setValue(double)));

    m_bedrockStdevSpinBox->setEnabled(bdv->stdevRequired());
    connect(bdv, SIGNAL(stdevRequiredChanged(bool)),
            m_bedrockStdevSpinBox, SLOT(setEnabled(bool)));

    // Minimum
    m_bedrockDepthMinCheckBox->setChecked(bdv->hasMin());
    connect(m_bedrockDepthMinCheckBox, SIGNAL(toggled(bool)),
            bdv, SLOT(setHasMin(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            m_bedrockDepthMinCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            m_bedrockDepthMinCheckBox, SLOT(setDisabled(bool)));

    m_bedrockDepthMinSpinBox->setRange( 0, bdv->max());
    m_bedrockDepthMinSpinBox->setEnabled(bdv->hasMin());
    connect(bdv, SIGNAL(hasMinChanged(bool)),
            m_bedrockDepthMinSpinBox, SLOT(setEnabled(bool)));

    m_bedrockDepthMinSpinBox->setValue(bdv->min());
    connect(m_bedrockDepthMinSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setMin(double)));
    connect(m_bedrockDepthMinSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setBedrockDepthMin(double)));

    // Maximum
    m_bedrockDepthMaxCheckBox->setChecked(bdv->hasMax());

    connect(m_bedrockDepthMaxCheckBox, SIGNAL(toggled(bool)),
            bdv, SLOT(setHasMax(bool)));
    connect(bdv, SIGNAL(hasMaxChanged(bool)),
            m_bedrockDepthMaxCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            m_bedrockDepthMaxCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            m_bedrockDepthMaxCheckBox, SLOT(setDisabled(bool)));

    m_bedrockDepthMaxSpinBox->setRange( bdv->min(), 10000);
    m_bedrockDepthMaxSpinBox->setEnabled(bdv->hasMax());
    connect(bdv, SIGNAL(hasMaxChanged(bool)),
            m_bedrockDepthMaxSpinBox, SLOT(setEnabled(bool)));

    m_bedrockDepthMaxSpinBox->setValue(bdv->max());
    connect(m_bedrockDepthMaxSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setMax(double)));
    connect(m_bedrockDepthMaxSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setBedrockDepthMax(double)));
}

void SoilProfilePage::setReadOnly(bool readOnly)
{
    m_soilProfileTableGroup->setReadOnly(readOnly);

    m_isVelocityVariedCheckBox->setDisabled(readOnly);
    m_isLayeringVariedCheckBox->setDisabled(readOnly);
    m_isBedrockDepthVariedCheckBox->setDisabled(readOnly);

    m_layerSpecificCheckBox->setDisabled(readOnly);
    m_distributionComboBox->setDisabled(readOnly);
    m_stdevModelComboBox->setDisabled(readOnly);
    m_stdevSpinBox->setReadOnly(readOnly);
    m_correlModelComboBox->setDisabled(readOnly);

    m_correlInitialSpinBox->setReadOnly(readOnly);
    m_correlFinalSpinBox->setReadOnly(readOnly);
    m_correlDeltaSpinBox->setReadOnly(readOnly);
    m_correlInterceptSpinBox->setReadOnly(readOnly);
    m_correlExponentSpinBox->setReadOnly(readOnly);

    m_layeringModelComboBox->setDisabled(readOnly);
    m_layeringCoeffSpinBox->setReadOnly(readOnly);
    m_layeringInitialSpinBox->setReadOnly(readOnly);
    m_layeringExponentSpinBox->setReadOnly(readOnly);

    m_bedrockModelComboBox->setDisabled(readOnly);
    m_bedrockStdevSpinBox->setReadOnly(readOnly);
    m_bedrockDepthMinCheckBox->setDisabled(readOnly);
    m_bedrockDepthMinSpinBox->setReadOnly(readOnly);
    m_bedrockDepthMaxCheckBox->setDisabled(readOnly);
    m_bedrockDepthMaxSpinBox->setReadOnly(readOnly);
}

QGroupBox* SoilProfilePage::createTableGroupBox()
{
    // Create the TableGroupBox
    m_soilProfileTableGroup = new TableGroupBox(tr("Site Profile"));

    m_soilTypeDelegate = new SoilTypeDelegate;
    m_soilProfileTableGroup->setItemDelegateForColumn(2, m_soilTypeDelegate);

    return m_soilProfileTableGroup;
}

QGroupBox* SoilProfilePage::createProfileRandomizerGroupBox()
{   
    QVBoxLayout* layout = new QVBoxLayout;

    // Title
    layout->addWidget(new QLabel(tr("<b>Toro (1992) Site Variation Model</b>")));

    // Check Boxes controlling what options are enabled
    // Shear-wave velocity variation
    m_isVelocityVariedCheckBox = new QCheckBox(tr("Vary the shear-wave velocity of the layers"));
    layout->addWidget(m_isVelocityVariedCheckBox);

    // Layer thickness variation
    m_isLayeringVariedCheckBox = new QCheckBox(tr("Vary the layer thickness"));
    layout->addWidget(m_isLayeringVariedCheckBox);

    // Depth to bedrock variation
    m_isBedrockDepthVariedCheckBox = new QCheckBox(tr("Vary the depth to bedrock"));
    layout->addWidget(m_isBedrockDepthVariedCheckBox);

    //
    // Toolbox contains options
    //
    m_parameterToolBox = new QToolBox;

    m_parameterToolBox->addItem(createVelocityFrame(), tr("Velocity Variation Parameters"));
    m_parameterToolBox->addItem(createLayeringFrame(), tr("Layer Thickness Variation Parameters"));
    m_parameterToolBox->addItem(createBedrockDepthFrame(), tr("Bedrock Depth Variation Parameters"));

    layout->addWidget(m_parameterToolBox);
    layout->addStretch(1);
    
    // Create the group box
    m_profileVariationGroupBox= new QGroupBox(tr("Variation of the Site Profile"));
    m_profileVariationGroupBox->setLayout(layout);

    return m_profileVariationGroupBox;
}

QFrame* SoilProfilePage::createVelocityFrame()
{
    QFormLayout* layout = new QFormLayout;

    // Layer specific standard deviation
    m_layerSpecificCheckBox = new QCheckBox(tr("Layer specifc standard deviation"));
    layout->addRow(m_layerSpecificCheckBox);

    // Distribution  -- fixed at log normal at the moment
    m_distributionComboBox = new QComboBox;
    m_distributionComboBox->addItem(tr("Log Normal"));
    layout->addRow(tr("Distribution:"), m_distributionComboBox);

    // Standard deviation
    m_stdevModelComboBox = new QComboBox;
    m_stdevModelComboBox->addItems(VelocityVariation::modelList());
    layout->addRow(tr("Standard deviation:"), m_stdevModelComboBox);

    m_stdevSpinBox = new QDoubleSpinBox;
    m_stdevSpinBox->setDecimals(2);
    m_stdevSpinBox->setRange( 0.0, 1.0);
    m_stdevSpinBox->setSingleStep(0.01);
    layout->addRow("", m_stdevSpinBox);

    // Correlation
    m_correlModelComboBox = new QComboBox;
    m_correlModelComboBox->addItems(VelocityVariation::modelList());
    layout->addRow(tr("Correlation model:"), m_correlModelComboBox);

    /*
     * Correlation group box
     */
    QFormLayout* correlLayout = new QFormLayout;

    // Initial correlation
    m_correlInitialSpinBox = new QDoubleSpinBox;
    m_correlInitialSpinBox->setRange(-1, 1);
    m_correlInitialSpinBox->setDecimals(2);
    m_correlInitialSpinBox->setSingleStep(0.1);

    correlLayout->addRow(tr("Correl. coeff. at surface (%1_0):").arg(QChar(0x03C1)),
                         m_correlInitialSpinBox);

    // Final correlation
    m_correlFinalSpinBox = new QDoubleSpinBox;
    m_correlFinalSpinBox->setRange(-1, 1);
    m_correlFinalSpinBox->setDecimals(2);
    m_correlFinalSpinBox->setSingleStep(0.1);

    correlLayout->addRow(tr("Correl. coeff. at 200 m (%1_200):").arg(QChar(0x03C1)),
                         m_correlFinalSpinBox);

    // Change in correlation with depth
    m_correlDeltaSpinBox = new QDoubleSpinBox;
    m_correlDeltaSpinBox->setRange(0, 10);
    m_correlDeltaSpinBox->setDecimals(2);
    m_correlDeltaSpinBox->setSingleStep(1);

    correlLayout->addRow(tr("Change in correl. with depth (%1):").arg(QChar(0x0394)),
                         m_correlDeltaSpinBox);

    // Initial depth 
    m_correlInterceptSpinBox = new QDoubleSpinBox;
    m_correlInterceptSpinBox->setRange(0, 100);
    m_correlInterceptSpinBox->setDecimals(1);
    m_correlInterceptSpinBox->setSingleStep(1);
    m_correlInterceptSpinBox->setSuffix(" m");

    correlLayout->addRow(tr("Depth intercept (d_0):"), m_correlInterceptSpinBox);

    // Exponent
    m_correlExponentSpinBox = new QDoubleSpinBox;
    m_correlExponentSpinBox->setRange(0, 1);
    m_correlExponentSpinBox->setDecimals(3);
    m_correlExponentSpinBox->setSingleStep(0.01);

    correlLayout->addRow(tr("Exponent (b):"), m_correlExponentSpinBox);

    correlLayout->addRow(new QLabel(tr("Correlation applied in <b>meters<b>")));

    m_correlGroupBox = new QGroupBox(tr("Correlation Parameters"));
    m_correlGroupBox->setLayout(correlLayout);

    // Add the correlation group box to the layout
    layout->addRow(m_correlGroupBox);

    m_velocityVariationFrame = new QFrame;
    m_velocityVariationFrame->setLayout(layout);

    return m_velocityVariationFrame;
} 

QFrame* SoilProfilePage::createLayeringFrame()
{
    QFormLayout* layout = new QFormLayout;

    // Model cofficients
    m_layeringModelComboBox = new QComboBox;
    m_layeringModelComboBox->addItems(LayerThicknessVariation::modelList());

    layout->addRow(tr("Parameters:"), m_layeringModelComboBox);

    layout->addRow(new QLabel(
            tr("Layer rate model: %1(d) = <b><i>a</i></b> (d + <b><i>b</i></b>)<sup><b><i>c</i></b></sup>")
            .arg(QChar(0x03BB))));

    // Coefficient line
    m_layeringCoeffSpinBox = new QDoubleSpinBox;
    m_layeringCoeffSpinBox->setRange( 0, 100);
    m_layeringCoeffSpinBox->setDecimals(2);
    m_layeringCoeffSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Coefficient (<b><i>a</i></b>):"), m_layeringCoeffSpinBox);

    // Initial line
    m_layeringInitialSpinBox = new QDoubleSpinBox;
    m_layeringInitialSpinBox->setRange( 0, 100 );
    m_layeringInitialSpinBox->setDecimals(2);
    m_layeringInitialSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Initial (<b><i>b</b></i>):"), m_layeringInitialSpinBox);

    // Exponent line
    m_layeringExponentSpinBox = new QDoubleSpinBox;
    m_layeringExponentSpinBox->setRange(-5, 0);
    m_layeringExponentSpinBox->setDecimals(2);
    m_layeringExponentSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Exponent (<b><i>c</b></i>):"), m_layeringExponentSpinBox);

    layout->addRow(new QLabel(tr("Correlation applied in <b>meters<b>")));

    // Create the frame and set the layout
    m_layerVariationFrame = new QFrame;
    m_layerVariationFrame->setLayout(layout);

    return m_layerVariationFrame;
}

QFrame* SoilProfilePage::createBedrockDepthFrame()
{
    QFormLayout* layout = new QFormLayout;

    // Distribution
    m_bedrockModelComboBox = new QComboBox;
    m_bedrockModelComboBox->addItems(AbstractDistribution::typeList());

    layout->addRow(tr("Distribution:"), m_bedrockModelComboBox);

    // Standard deviation
    m_bedrockStdevSpinBox = new QDoubleSpinBox;
    m_bedrockStdevSpinBox->setRange( 0, 500 );
    m_bedrockStdevSpinBox->setSingleStep(0.1);
    m_bedrockStdevSpinBox->setDecimals(2);

    layout->addRow(tr("Standard deviation:"), m_bedrockStdevSpinBox);
    
    // Minimum
    m_bedrockDepthMinCheckBox = new QCheckBox(tr("Minimum depth to bedrock:"));
    m_bedrockDepthMinSpinBox = new QDoubleSpinBox;
    layout->addRow(m_bedrockDepthMinCheckBox, m_bedrockDepthMinSpinBox);

    // Maximum
    m_bedrockDepthMaxCheckBox = new QCheckBox(tr("Maximum depth to bedrock:"));
    m_bedrockDepthMaxSpinBox = new QDoubleSpinBox;
    layout->addRow(m_bedrockDepthMaxCheckBox, m_bedrockDepthMaxSpinBox);

    // Create the frame and set the layout
    m_bedrockDepthFrame = new QFrame;
    m_bedrockDepthFrame->setLayout(layout);

    return m_bedrockDepthFrame;
}

void SoilProfilePage::setVelocityItemEnabled(bool enabled)
{
    m_parameterToolBox->setItemEnabled(VelocityIndex, enabled);

    if (enabled) {
        m_parameterToolBox->setCurrentIndex(VelocityIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < m_parameterToolBox->count(); ++i) {
            if (m_parameterToolBox->isItemEnabled(i)) {
                m_parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::setLayeringItemEnabled(bool enabled)
{
    m_parameterToolBox->setItemEnabled(LayeringIndex, enabled);

    if (enabled) {
        m_parameterToolBox->setCurrentIndex(LayeringIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < m_parameterToolBox->count(); ++i) {
            if (m_parameterToolBox->isItemEnabled(i)) {
                m_parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::setBedrockDepthItemEnabled(bool enabled)
{
    m_parameterToolBox->setItemEnabled(BedrockDepthIndex, enabled);

    if (enabled) {
        m_parameterToolBox->setCurrentIndex(BedrockDepthIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < m_parameterToolBox->count(); ++i) {
            if (m_parameterToolBox->isItemEnabled(i)) {
                m_parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::updateUnits()
{
    m_bedrockDepthMinSpinBox->setSuffix(" " + Units::instance()->length());
    m_bedrockDepthMaxSpinBox->setSuffix(" " + Units::instance()->length());

    foreach (QComboBox* comboBox,
             QList<QComboBox*>() << m_stdevModelComboBox << m_correlModelComboBox) {
        int index = comboBox->currentIndex();

        comboBox->blockSignals(true);
        comboBox->clear();
        comboBox->addItems(VelocityVariation::modelList());
        comboBox->setCurrentIndex(index);
        comboBox->blockSignals(false);
    }
}

void SoilProfilePage::updateHiddenColumns()
{
    // Adjust all columns
    bool enabled = m_velocityVariation->enabled();
    for (int i = 4; i < 8; ++i)
        m_soilProfileTableGroup->setColumnHidden(i, !enabled);
    // Standard deviation column
    if (enabled) {
        m_soilProfileTableGroup->setColumnHidden(
                SoilProfile::StdevColumn,
                !m_velocityVariation->stdevIsLayerSpecific());
    }
}

void SoilProfilePage::setBedrockDepthMin(double min)
{
    m_bedrockDepthMaxSpinBox->setMinimum(min);
}

void SoilProfilePage::setBedrockDepthMax(double max)
{
    m_bedrockDepthMinSpinBox->setMaximum(max);
}
