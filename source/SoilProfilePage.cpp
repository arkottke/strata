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
    auto* layout = new QHBoxLayout;

    layout->addWidget(createTableGroupBox(), 1);
    layout->addWidget(createProfileRandomizerGroupBox());
    
    setLayout(layout);

    // Connections
    connect( Units::instance(), SIGNAL(systemChanged(int)),
             this, SLOT(updateUnits()));
}

void SoilProfilePage::setModel(SiteResponseModel *model)
{
    _soilProfileTableGroup->setModel(model->siteProfile());
    _soilTypeDelegate->setCatalog(model->siteProfile()->soilTypeCatalog());

    _velocityVariation = model->siteProfile()->profileRandomizer()->velocityVariation();
    updateHiddenColumns();
    connect(_velocityVariation, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateHiddenColumns()));

    ProfileRandomizer* pr = model->siteProfile()->profileRandomizer();

    _profileVariationGroupBox->setVisible(pr->enabled());
    connect(pr, SIGNAL(enabledChanged(bool)),
            _profileVariationGroupBox, SLOT(setVisible(bool)));

    _isVelocityVariedCheckBox->setChecked(pr->velocityVariation()->enabled());
    connect(_isVelocityVariedCheckBox, SIGNAL(toggled(bool)),
            pr->velocityVariation(), SLOT(setEnabled(bool)));
    connect(pr->velocityVariation(), SIGNAL(enabledChanged(bool)),
            _isVelocityVariedCheckBox, SLOT(setChecked(bool)));

    _isLayeringVariedCheckBox->setChecked(pr->layerThicknessVariation()->enabled());
    connect(_isLayeringVariedCheckBox, SIGNAL(toggled(bool)),
            pr->layerThicknessVariation(), SLOT(setEnabled(bool)));
    connect(pr->layerThicknessVariation(), SIGNAL(enabledChanged(bool)),
            _isLayeringVariedCheckBox, SLOT(setChecked(bool)));

    _isBedrockDepthVariedCheckBox->setChecked(pr->bedrockDepthVariation()->enabled());
    connect(_isBedrockDepthVariedCheckBox, SIGNAL(toggled(bool)),
            pr->bedrockDepthVariation(), SLOT(setEnabled(bool)));
    connect(pr->bedrockDepthVariation(), SIGNAL(enabledChanged(bool)),
            _isBedrockDepthVariedCheckBox, SLOT(setChecked(bool)));

    // Velocity Variation
    VelocityVariation* vv =
            model->siteProfile()->profileRandomizer()->velocityVariation();

    setVelocityItemEnabled(vv->enabled());
    connect(vv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setVelocityItemEnabled(bool)));

    _velocityVariationFrame->setEnabled(vv->enabled());
    connect(vv, SIGNAL(enabledChanged(bool)),
            _velocityVariationFrame, SLOT(setEnabled(bool)));

    _layerSpecificCheckBox->setChecked(vv->stdevIsLayerSpecific());
    connect(_layerSpecificCheckBox, SIGNAL(toggled(bool)),
            vv, SLOT(setStdevIsLayerSpecific(bool)));
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            _layerSpecificCheckBox, SLOT(setChecked(bool)));
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            this, SLOT(updateHiddenColumns()));

    _stdevModelComboBox->setCurrentIndex(vv->stdevModel());
    connect(_stdevModelComboBox, SIGNAL(currentIndexChanged(int)),
             vv, SLOT(setStdevModel(int)));

    _stdevModelComboBox->setDisabled(vv->stdevIsLayerSpecific());
    connect(vv, SIGNAL(stdevIsLayerSpecificChanged(bool)),
            _stdevModelComboBox, SLOT(setDisabled(bool)));

    _stdevSpinBox->setValue(vv->stdev());
    connect(_stdevSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setStdev(double)));
    connect(vv, SIGNAL(stdevChanged(double)),
            _stdevSpinBox, SLOT(setValue(double)));

    _stdevSpinBox->setEnabled(vv->stdevCustomEnabled());
    connect(vv, SIGNAL(stdevCustomEnabledChanged(bool)),
            _stdevSpinBox, SLOT(setEnabled(bool)));

    _correlModelComboBox->setCurrentIndex(vv->stdevModel());
    connect(_correlModelComboBox, SIGNAL(currentIndexChanged(int)),
             vv, SLOT(setCorrelModel(int)));

    _correlInitialSpinBox->setValue(vv->correlInitial());
    connect(_correlInitialSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelInitial(double)));
    connect(vv, SIGNAL(correlInitialChanged(double)),
            _correlInitialSpinBox, SLOT(setValue(double)));

    _correlFinalSpinBox->setValue(vv->correlFinal());
    connect(_correlFinalSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelFinal(double)));
    connect(vv, SIGNAL(correlFinalChanged(double)),
            _correlFinalSpinBox, SLOT(setValue(double)));

    _correlDeltaSpinBox->setValue(vv->correlDelta());
    connect(_correlDeltaSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelDelta(double)));
    connect(vv, SIGNAL(correlDeltaChanged(double)),
            _correlDeltaSpinBox, SLOT(setValue(double)));

    _correlInterceptSpinBox->setValue(vv->correlIntercept());
    connect(_correlInterceptSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelIntercept(double)));
    connect(vv, SIGNAL(correlInterceptChanged(double)),
            _correlInterceptSpinBox, SLOT(setValue(double)));

    _correlExponentSpinBox->setValue(vv->correlExponent());
    connect(_correlExponentSpinBox, SIGNAL(valueChanged(double)),
            vv, SLOT(setCorrelExponent(double)));
    connect(vv, SIGNAL(correlExponentChanged(double)),
            _correlExponentSpinBox, SLOT(setValue(double)));

    _correlGroupBox->setEnabled(vv->correlCustomEnabled());
    connect(vv, SIGNAL(correlCustomEnabledChanged(bool)),
            _correlGroupBox, SLOT(setEnabled(bool)));

    // Layering variation
    LayerThicknessVariation* ltv =
            model->siteProfile()->profileRandomizer()->layerThicknessVariation();

    _layerVariationFrame->setEnabled(ltv->enabled());
    connect(ltv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setLayeringItemEnabled(bool)));
    connect(ltv, SIGNAL(enabledChanged(bool)),
            _layerVariationFrame, SLOT(setEnabled(bool)));

    _layeringModelComboBox->setCurrentIndex(ltv->model());
    connect(_layeringModelComboBox, SIGNAL(currentIndexChanged(int)),
            ltv, SLOT(setModel(int)));

    _layeringCoeffSpinBox->setValue(ltv->coeff());
    connect(_layeringCoeffSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setCoeff(double)));
    connect(ltv, SIGNAL(coeffChanged(double)),
            _layeringCoeffSpinBox, SLOT(setValue(double)));

    _layeringCoeffSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            _layeringCoeffSpinBox, SLOT(setEnabled(bool)));

    _layeringInitialSpinBox->setValue(ltv->initial());
    connect(_layeringInitialSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setInitial(double)));
    connect(ltv, SIGNAL(initialChanged(double)),
            _layeringInitialSpinBox, SLOT(setValue(double)));

    _layeringInitialSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            _layeringInitialSpinBox, SLOT(setEnabled(bool)));

    _layeringExponentSpinBox->setValue(ltv->exponent());
    connect(_layeringExponentSpinBox, SIGNAL(valueChanged(double)),
            ltv, SLOT(setExponent(double)));
    connect(ltv, SIGNAL(exponentChanged(double)),
            _layeringExponentSpinBox, SLOT(setValue(double)));

    _layeringExponentSpinBox->setEnabled(ltv->customEnabled());
    connect(ltv, SIGNAL(customEnabledChanged(bool)),
            _layeringExponentSpinBox, SLOT(setEnabled(bool)));

    // Bedrock depth variation
    BedrockDepthVariation* bdv =
            model->siteProfile()->profileRandomizer()->bedrockDepthVariation();

    _bedrockDepthFrame->setEnabled(bdv->enabled());
    connect(bdv, SIGNAL(enabledChanged(bool)),
            this, SLOT(setBedrockDepthItemEnabled(bool)));

    _bedrockDepthFrame->setEnabled(bdv->enabled());
    connect(bdv, SIGNAL(enabledChanged(bool)),
            _bedrockDepthFrame, SLOT(setEnabled(bool)));

    _bedrockModelComboBox->setCurrentIndex(bdv->type());
    connect(_bedrockModelComboBox, SIGNAL(currentIndexChanged(int)),
            bdv, SLOT(setType(int)));

    _bedrockStdevSpinBox->setValue(bdv->stdev());
    connect(_bedrockStdevSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setStdev(double)));
    connect(bdv, SIGNAL(stdevChanged(double)),
            _bedrockStdevSpinBox, SLOT(setValue(double)));

    _bedrockStdevSpinBox->setEnabled(bdv->stdevRequired());
    connect(bdv, SIGNAL(stdevRequiredChanged(bool)),
            _bedrockStdevSpinBox, SLOT(setEnabled(bool)));

    // Minimum
    _bedrockDepthMinCheckBox->setChecked(bdv->hasMin());
    connect(_bedrockDepthMinCheckBox, SIGNAL(toggled(bool)),
            bdv, SLOT(setHasMin(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            _bedrockDepthMinCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            _bedrockDepthMinCheckBox, SLOT(setDisabled(bool)));

    _bedrockDepthMinSpinBox->setRange( 0, bdv->max());
    _bedrockDepthMinSpinBox->setEnabled(bdv->hasMin());
    connect(bdv, SIGNAL(hasMinChanged(bool)),
            _bedrockDepthMinSpinBox, SLOT(setEnabled(bool)));

    _bedrockDepthMinSpinBox->setValue(bdv->min());
    connect(_bedrockDepthMinSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setMin(double)));
    connect(_bedrockDepthMinSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setBedrockDepthMin(double)));

    // Maximum
    _bedrockDepthMaxCheckBox->setChecked(bdv->hasMax());

    connect(_bedrockDepthMaxCheckBox, SIGNAL(toggled(bool)),
            bdv, SLOT(setHasMax(bool)));
    connect(bdv, SIGNAL(hasMaxChanged(bool)),
            _bedrockDepthMaxCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            _bedrockDepthMaxCheckBox, SLOT(setChecked(bool)));
    connect(bdv, SIGNAL(requiresLimits(bool)), 
            _bedrockDepthMaxCheckBox, SLOT(setDisabled(bool)));

    _bedrockDepthMaxSpinBox->setRange( bdv->min(), 10000);
    _bedrockDepthMaxSpinBox->setEnabled(bdv->hasMax());
    connect(bdv, SIGNAL(hasMaxChanged(bool)),
            _bedrockDepthMaxSpinBox, SLOT(setEnabled(bool)));

    _bedrockDepthMaxSpinBox->setValue(bdv->max());
    connect(_bedrockDepthMaxSpinBox, SIGNAL(valueChanged(double)),
            bdv, SLOT(setMax(double)));
    connect(_bedrockDepthMaxSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(setBedrockDepthMax(double)));
}

void SoilProfilePage::setReadOnly(bool readOnly)
{
    _soilProfileTableGroup->setReadOnly(readOnly);

    _isVelocityVariedCheckBox->setDisabled(readOnly);
    _isLayeringVariedCheckBox->setDisabled(readOnly);
    _isBedrockDepthVariedCheckBox->setDisabled(readOnly);

    _layerSpecificCheckBox->setDisabled(readOnly);
    _distributionComboBox->setDisabled(readOnly);
    _stdevModelComboBox->setDisabled(readOnly);
    _stdevSpinBox->setReadOnly(readOnly);
    _correlModelComboBox->setDisabled(readOnly);

    _correlInitialSpinBox->setReadOnly(readOnly);
    _correlFinalSpinBox->setReadOnly(readOnly);
    _correlDeltaSpinBox->setReadOnly(readOnly);
    _correlInterceptSpinBox->setReadOnly(readOnly);
    _correlExponentSpinBox->setReadOnly(readOnly);

    _layeringModelComboBox->setDisabled(readOnly);
    _layeringCoeffSpinBox->setReadOnly(readOnly);
    _layeringInitialSpinBox->setReadOnly(readOnly);
    _layeringExponentSpinBox->setReadOnly(readOnly);

    _bedrockModelComboBox->setDisabled(readOnly);
    _bedrockStdevSpinBox->setReadOnly(readOnly);
    _bedrockDepthMinCheckBox->setDisabled(readOnly);
    _bedrockDepthMinSpinBox->setReadOnly(readOnly);
    _bedrockDepthMaxCheckBox->setDisabled(readOnly);
    _bedrockDepthMaxSpinBox->setReadOnly(readOnly);
}

auto SoilProfilePage::createTableGroupBox() -> QGroupBox*
{
    // Create the TableGroupBox
    _soilProfileTableGroup = new TableGroupBox(tr("Site Profile"));

    _soilTypeDelegate = new SoilTypeDelegate;
    _soilProfileTableGroup->setItemDelegateForColumn(2, _soilTypeDelegate);

    return _soilProfileTableGroup;
}

auto SoilProfilePage::createProfileRandomizerGroupBox() -> QGroupBox*
{   
    auto* layout = new QVBoxLayout;

    // Title
    layout->addWidget(new QLabel(tr("<b>Toro (1992) Site Variation Model</b>")));

    // Check Boxes controlling what options are enabled
    // Shear-wave velocity variation
    _isVelocityVariedCheckBox = new QCheckBox(tr("Vary the shear-wave velocity of the layers"));
    layout->addWidget(_isVelocityVariedCheckBox);

    // Layer thickness variation
    _isLayeringVariedCheckBox = new QCheckBox(tr("Vary the layer thickness"));
    layout->addWidget(_isLayeringVariedCheckBox);

    // Depth to bedrock variation
    _isBedrockDepthVariedCheckBox = new QCheckBox(tr("Vary the depth to bedrock"));
    layout->addWidget(_isBedrockDepthVariedCheckBox);

    //
    // Toolbox contains options
    //
    _parameterToolBox = new QToolBox;

    _parameterToolBox->addItem(createVelocityFrame(), tr("Velocity Variation Parameters"));
    _parameterToolBox->addItem(createLayeringFrame(), tr("Layer Thickness Variation Parameters"));
    _parameterToolBox->addItem(createBedrockDepthFrame(), tr("Bedrock Depth Variation Parameters"));

    layout->addWidget(_parameterToolBox);
    layout->addStretch(1);
    
    // Create the group box
    _profileVariationGroupBox= new QGroupBox(tr("Variation of the Site Profile"));
    _profileVariationGroupBox->setLayout(layout);

    return _profileVariationGroupBox;
}

auto SoilProfilePage::createVelocityFrame() -> QFrame*
{
    auto* layout = new QFormLayout;

    // Layer specific standard deviation
    _layerSpecificCheckBox = new QCheckBox(tr("Layer specifc standard deviation"));
    layout->addRow(_layerSpecificCheckBox);

    // Distribution  -- fixed at log normal at the moment
    _distributionComboBox = new QComboBox;
    _distributionComboBox->addItem(tr("Log Normal"));
    layout->addRow(tr("Distribution:"), _distributionComboBox);

    // Standard deviation
    _stdevModelComboBox = new QComboBox;
    _stdevModelComboBox->addItems(VelocityVariation::modelList());
    layout->addRow(tr("Standard deviation:"), _stdevModelComboBox);

    _stdevSpinBox = new QDoubleSpinBox;
    _stdevSpinBox->setDecimals(3);
    _stdevSpinBox->setRange( 0.0, 1.0);
    _stdevSpinBox->setSingleStep(0.01);
    layout->addRow("", _stdevSpinBox);

    // Correlation
    _correlModelComboBox = new QComboBox;
    _correlModelComboBox->addItems(VelocityVariation::modelList());
    layout->addRow(tr("Correlation model:"), _correlModelComboBox);

    /*
     * Correlation group box
     */
    auto* correlLayout = new QFormLayout;

    // Initial correlation
    _correlInitialSpinBox = new QDoubleSpinBox;
    _correlInitialSpinBox->setRange(-1, 1);
    _correlInitialSpinBox->setDecimals(3);
    _correlInitialSpinBox->setSingleStep(0.1);

    correlLayout->addRow(tr("Correl. coeff. at surface (%1_0):").arg(QChar(0x03C1)),
                         _correlInitialSpinBox);

    // Final correlation
    _correlFinalSpinBox = new QDoubleSpinBox;
    _correlFinalSpinBox->setRange(-1, 1);
    _correlFinalSpinBox->setDecimals(3);
    _correlFinalSpinBox->setSingleStep(0.1);

    correlLayout->addRow(tr("Correl. coeff. at 200 m (%1_200):").arg(QChar(0x03C1)),
                         _correlFinalSpinBox);

    // Change in correlation with depth
    _correlDeltaSpinBox = new QDoubleSpinBox;
    _correlDeltaSpinBox->setRange(0, 10);
    _correlDeltaSpinBox->setDecimals(3);
    _correlDeltaSpinBox->setSingleStep(1);

    correlLayout->addRow(tr("Change in correl. with depth (%1):").arg(QChar(0x0394)),
                         _correlDeltaSpinBox);

    // Initial depth 
    _correlInterceptSpinBox = new QDoubleSpinBox;
    _correlInterceptSpinBox->setRange(0, 100);
    _correlInterceptSpinBox->setDecimals(2);
    _correlInterceptSpinBox->setSingleStep(1);
    _correlInterceptSpinBox->setSuffix(" m");

    correlLayout->addRow(tr("Depth intercept (d_0):"), _correlInterceptSpinBox);

    // Exponent
    _correlExponentSpinBox = new QDoubleSpinBox;
    _correlExponentSpinBox->setRange(0, 1);
    _correlExponentSpinBox->setDecimals(4);
    _correlExponentSpinBox->setSingleStep(0.01);

    correlLayout->addRow(tr("Exponent (b):"), _correlExponentSpinBox);

    correlLayout->addRow(new QLabel(tr("Correlation applied in <b>meters<b>")));

    _correlGroupBox = new QGroupBox(tr("Correlation Parameters"));
    _correlGroupBox->setLayout(correlLayout);

    // Add the correlation group box to the layout
    layout->addRow(_correlGroupBox);

    _velocityVariationFrame = new QFrame;
    _velocityVariationFrame->setLayout(layout);

    return _velocityVariationFrame;
} 

auto SoilProfilePage::createLayeringFrame() -> QFrame*
{
    auto* layout = new QFormLayout;

    // Model cofficients
    _layeringModelComboBox = new QComboBox;
    _layeringModelComboBox->addItems(LayerThicknessVariation::modelList());

    layout->addRow(tr("Parameters:"), _layeringModelComboBox);

    layout->addRow(new QLabel(
            tr("Layer rate model: %1(d) = <b><i>a</i></b> (d + <b><i>b</i></b>)<sup><b><i>c</i></b></sup>")
            .arg(QChar(0x03BB))));

    // Coefficient line
    _layeringCoeffSpinBox = new QDoubleSpinBox;
    _layeringCoeffSpinBox->setRange( 0, 100);
    _layeringCoeffSpinBox->setDecimals(3);
    _layeringCoeffSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Coefficient (<b><i>a</i></b>):"), _layeringCoeffSpinBox);

    // Initial line
    _layeringInitialSpinBox = new QDoubleSpinBox;
    _layeringInitialSpinBox->setRange( 0, 100 );
    _layeringInitialSpinBox->setDecimals(3);
    _layeringInitialSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Initial (<b><i>b</b></i>):"), _layeringInitialSpinBox);

    // Exponent line
    _layeringExponentSpinBox = new QDoubleSpinBox;
    _layeringExponentSpinBox->setRange(-5, 0);
    _layeringExponentSpinBox->setDecimals(3);
    _layeringExponentSpinBox->setSingleStep(0.01);

    layout->addRow(tr("Exponent (<b><i>c</b></i>):"), _layeringExponentSpinBox);

    layout->addRow(new QLabel(tr("Correlation applied in <b>meters<b>")));

    // Create the frame and set the layout
    _layerVariationFrame = new QFrame;
    _layerVariationFrame->setLayout(layout);

    return _layerVariationFrame;
}

auto SoilProfilePage::createBedrockDepthFrame() -> QFrame*
{
    auto* layout = new QFormLayout;

    // Distribution
    _bedrockModelComboBox = new QComboBox;
    _bedrockModelComboBox->addItems(AbstractDistribution::typeList());

    layout->addRow(tr("Distribution:"), _bedrockModelComboBox);

    // Standard deviation
    _bedrockStdevSpinBox = new QDoubleSpinBox;
    _bedrockStdevSpinBox->setRange( 0, 500 );
    _bedrockStdevSpinBox->setSingleStep(0.1);
    _bedrockStdevSpinBox->setDecimals(3);

    layout->addRow(tr("Standard deviation:"), _bedrockStdevSpinBox);
    
    // Minimum
    _bedrockDepthMinCheckBox = new QCheckBox(tr("Minimum depth to bedrock:"));
    _bedrockDepthMinSpinBox = new QDoubleSpinBox;
    layout->addRow(_bedrockDepthMinCheckBox, _bedrockDepthMinSpinBox);

    // Maximum
    _bedrockDepthMaxCheckBox = new QCheckBox(tr("Maximum depth to bedrock:"));
    _bedrockDepthMaxSpinBox = new QDoubleSpinBox;
    layout->addRow(_bedrockDepthMaxCheckBox, _bedrockDepthMaxSpinBox);

    // Create the frame and set the layout
    _bedrockDepthFrame = new QFrame;
    _bedrockDepthFrame->setLayout(layout);

    return _bedrockDepthFrame;
}

void SoilProfilePage::setVelocityItemEnabled(bool enabled)
{
    _parameterToolBox->setItemEnabled(VelocityIndex, enabled);

    if (enabled) {
        _parameterToolBox->setCurrentIndex(VelocityIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < _parameterToolBox->count(); ++i) {
            if (_parameterToolBox->isItemEnabled(i)) {
                _parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::setLayeringItemEnabled(bool enabled)
{
    _parameterToolBox->setItemEnabled(LayeringIndex, enabled);

    if (enabled) {
        _parameterToolBox->setCurrentIndex(LayeringIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < _parameterToolBox->count(); ++i) {
            if (_parameterToolBox->isItemEnabled(i)) {
                _parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::setBedrockDepthItemEnabled(bool enabled)
{
    _parameterToolBox->setItemEnabled(BedrockDepthIndex, enabled);

    if (enabled) {
        _parameterToolBox->setCurrentIndex(BedrockDepthIndex);
    } else {
        // Select another enabled item
        for (int i = 0; i < _parameterToolBox->count(); ++i) {
            if (_parameterToolBox->isItemEnabled(i)) {
                _parameterToolBox->setCurrentIndex(i);
            }
        }
    }
}

void SoilProfilePage::updateUnits()
{
    _bedrockDepthMinSpinBox->setSuffix(" " + Units::instance()->length());
    _bedrockDepthMaxSpinBox->setSuffix(" " + Units::instance()->length());

    foreach (QComboBox* comboBox,
             QList<QComboBox*>() << _stdevModelComboBox << _correlModelComboBox) {
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
    bool enabled = _velocityVariation->enabled();
    for (int i = 4; i < 8; ++i)
        _soilProfileTableGroup->setColumnHidden(i, !enabled);
    // Standard deviation column
    if (enabled) {
        _soilProfileTableGroup->setColumnHidden(
                SoilProfile::StdevColumn,
                !_velocityVariation->stdevIsLayerSpecific());
    }
}

void SoilProfilePage::setBedrockDepthMin(double min)
{
    _bedrockDepthMaxSpinBox->setMinimum(min);
}

void SoilProfilePage::setBedrockDepthMax(double max)
{
    _bedrockDepthMinSpinBox->setMaximum(max);
}
