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
#include "SoilProfileModel.h"
#include "StringListDelegate.h"
#include "Units.h"

#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

SoilProfilePage::SoilProfilePage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f)
    : QWidget( parent, f ), m_model(model)
{
    // Setup the two group boxes
    createTableGroupBox();
    createProfileVariationGroupBox();
    
    // Create the layout
    QGridLayout * layout = new QGridLayout;
    layout->addWidget(m_tableGroupBox, 0, 0 );
    layout->addWidget(m_profileVariationGroupBox, 0, 1);
    
    layout->setColumnStretch( 0, 1);
    
    setLayout(layout);

    // Load the initial values
    load();

    // Load the units
    updateUnits();

    // Intially hide the columns on the table
    setVelocityIsVaried(false);

    // Connections
    connect( Units::instance(), SIGNAL(systemChanged()), this, SLOT(updateUnits()));
}

void SoilProfilePage::createTableGroupBox()
{
    // Create the TableGroupBox
    m_tableGroupBox = new TableGroupBox( new SoilProfileModel(m_model->siteProfile(), Units::instance()), tr("Velocity Layers"));

    m_tableGroupBox->table()->setItemDelegateForColumn( 2, new StringListDelegate );
    m_tableGroupBox->setLastRowFixed(true);

    connect( m_tableGroupBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
}

void SoilProfilePage::createProfileVariationGroupBox()
{
    QGridLayout * layout = new QGridLayout;

    // Title
    layout->addWidget(new QLabel(tr("<b>Toro (1992) Site Variation Model</b>")), 0, 0);

    // Shear-wave velocity checkbox
    m_isVelocityVariedCheckBox = new QCheckBox(tr("Vary the shear-wave velocity of the layers"));

    connect( m_isVelocityVariedCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->profileVariation(), SLOT(setVaryVelocity(bool)));

    layout->addWidget(m_isVelocityVariedCheckBox, 1, 0, 1, 2);
    
    createVelocityVariationGroupBox();
    
    connect( m_isVelocityVariedCheckBox, SIGNAL(toggled(bool)), m_velocityVariationGroupBox, SLOT(setVisible(bool)));
    connect( m_isVelocityVariedCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVelocityIsVaried(bool)));
    
    layout->addWidget( m_velocityVariationGroupBox, 2, 0, 1, 2 );
    
    // Layer thicknesses variation
    m_isLayeringVariedCheckBox = new QCheckBox(tr("Vary the layer thickness"));
    layout->addWidget(m_isLayeringVariedCheckBox, 3, 0);
    connect( m_isLayeringVariedCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->profileVariation(), SLOT(setVaryLayering(bool)));
    
    createLayeringGroupBox();
    connect(m_isLayeringVariedCheckBox, SIGNAL(toggled(bool)), m_layerVariationGroupBox, SLOT(setVisible(bool)));
    
    layout->addWidget( m_layerVariationGroupBox, 4, 0, 1, 2);
    
    // Depth to bedrock
    m_isBedrockDepthVariedCheckBox = new QCheckBox(tr("Vary the depth to bedrock"));
    layout->addWidget(m_isBedrockDepthVariedCheckBox, 5, 0, 1, 2);
    connect( m_isBedrockDepthVariedCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->profileVariation(), SLOT(setVaryBedrockDepth(bool)));

    createBedrockDepthGroupBox();
    
    connect( m_isBedrockDepthVariedCheckBox, SIGNAL(toggled(bool)), m_bedrockDepthGroupBox, SLOT(setVisible(bool)));
   
    layout->addWidget(m_bedrockDepthGroupBox, 6, 0, 1, 2);
    
    // Add a stretch row
    layout->setRowStretch( 7, 1 );
    
    // Create the group box
    m_profileVariationGroupBox = new QGroupBox(tr("Variation of the Site Profile"));
    m_profileVariationGroupBox->setVisible(false);
    m_profileVariationGroupBox->setLayout(layout);
}

void SoilProfilePage::createVelocityVariationGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);
    // Layer specific standard deviation
    m_layerSpecificCheckBox = new QCheckBox(tr("Layer specifc standard deviation"));
    connect(m_layerSpecificCheckBox, SIGNAL(toggled(bool)), this, SLOT(setIsLayerSpecific(bool)));
    connect( m_layerSpecificCheckBox, SIGNAL(toggled(bool)),
            m_model->siteProfile()->profileVariation(), SLOT(setStdevIsLayerSpecific(bool)));
    
    layout->addWidget(m_layerSpecificCheckBox, 1, 0, 1, 2);

    // Distribution
    m_distributionComboBox = new QComboBox;
    m_distributionComboBox->addItem(tr("Log Normal"));
    connect( m_distributionComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->profileVariation(), SLOT(setStdevModel(int)));

    layout->addWidget(new QLabel(tr("Distribution:")), 2, 0);
    layout->addWidget( m_distributionComboBox, 2, 1);

    // Standard deviation
    m_stdevModelComboBox = new QComboBox;
    m_stdevModelComboBox->addItems( ProfileVariation::velocityModelList() );
    connect( m_stdevModelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateStdevModel(int)));
    connect( m_stdevModelComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->profileVariation(), SLOT(setStdevModel(int)));

    m_stdevSpinBox = new QDoubleSpinBox;
    m_stdevSpinBox->setDecimals(2);
    m_stdevSpinBox->setRange( 0.0, 1.0);
    m_stdevSpinBox->setSingleStep(0.1);
    
    connect( m_stdevSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->profileVariation(), SLOT(setStdev(double)));

    layout->addWidget(new QLabel(tr("Standard deviation:")), 3, 0);
    layout->addWidget(m_stdevModelComboBox, 3, 1);
    layout->addWidget(m_stdevSpinBox, 4, 1);

    // Correlation
    m_correlModelComboBox = new QComboBox;
    m_correlModelComboBox->addItems( ProfileVariation::velocityModelList() );
    connect( m_correlModelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCorrelModel(int)));
    connect( m_correlModelComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelModel(int)));

    layout->addWidget(new QLabel(tr("Correlation model:")), 5, 0);
    layout->addWidget(m_correlModelComboBox, 5, 1);

    /*
     * Correlation group box
     */
    QGridLayout * correlLayout = new QGridLayout;
    correlLayout->setColumnStretch( 0, 1);

    // Initial correlation
    m_correlInitialSpinBox = new QDoubleSpinBox;
    m_correlInitialSpinBox->setRange(-1, 1);
    m_correlInitialSpinBox->setDecimals(2);
    m_correlInitialSpinBox->setSingleStep(0.1);

    connect( m_correlInitialSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelInitial(double)));

    correlLayout->addWidget(new QLabel(
                QString(tr("Correl. coeff. at surface (%1_0):")).arg(QChar(0x03C1))), 0, 0);
    correlLayout->addWidget(m_correlInitialSpinBox, 0, 1);

    // Final correlation
    m_correlFinalSpinBox = new QDoubleSpinBox;
    m_correlFinalSpinBox->setRange(-1, 1);
    m_correlFinalSpinBox->setDecimals(2);
    m_correlFinalSpinBox->setSingleStep(0.1);

    connect( m_correlFinalSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelFinal(double)));

    correlLayout->addWidget(new QLabel(
                QString(tr("Correl. coeff. at 200 m (%1_200):")).arg(QChar(0x03C1))), 1, 0);
    correlLayout->addWidget(m_correlFinalSpinBox, 1, 1);

    // Change in correlation with depth
    m_correlDeltaSpinBox = new QDoubleSpinBox;
    m_correlFinalSpinBox->setRange(0, 10);
    m_correlFinalSpinBox->setDecimals(2);
    m_correlFinalSpinBox->setSingleStep(1);
    
    connect( m_correlDeltaSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelDelta(double)));

    correlLayout->addWidget(new QLabel(
                QString(tr("Change in correl. with depth (%1):")).arg(QChar(0x0394))), 2, 0);
    correlLayout->addWidget(m_correlDeltaSpinBox, 2, 1);

    // Initial depth 
    m_depthInterceptSpinBox = new QDoubleSpinBox;
    m_depthInterceptSpinBox->setRange(0, 100);
    m_depthInterceptSpinBox->setDecimals(1);
    m_depthInterceptSpinBox->setSingleStep(1);
    m_depthInterceptSpinBox->setSuffix(" m");

    connect( m_depthInterceptSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelIntercept(double)));

    correlLayout->addWidget(new QLabel(tr("Depth intercept (d_0):")), 3, 0 );
    correlLayout->addWidget(m_depthInterceptSpinBox, 3, 1);

    // Exponent
    m_exponentSpinBox = new QDoubleSpinBox;
    m_exponentSpinBox->setRange(0, 1);
    m_exponentSpinBox->setDecimals(3);
    m_exponentSpinBox->setSingleStep(0.01);

    connect( m_exponentSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->profileVariation(), SLOT(setCorrelExponent(double)));

    correlLayout->addWidget(new QLabel(tr("Exponent (b):")), 4, 0 );
    correlLayout->addWidget(m_exponentSpinBox, 4, 1);
   
    m_correlGroupBox = new QGroupBox(tr("Correlation Parameters"));
    m_correlGroupBox->setLayout(correlLayout);
    
    // Add the correlation group box to the layout
    layout->addWidget(m_correlGroupBox, 6, 0, 1, 2);

    // Group box -- default is disabled
    m_velocityVariationGroupBox = new QGroupBox(tr("Velocity Variation Parameters"));

    m_velocityVariationGroupBox->setLayout(layout);
} 

void SoilProfilePage::createLayeringGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Model cofficients
    m_layeringModelComboBox = new QComboBox;
    m_layeringModelComboBox->addItems(ProfileVariation::layeringModelList());
    
    connect( m_layeringModelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateLayeringModel(int)));
    connect( m_layeringModelComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->profileVariation(), SLOT(setLayeringModel(int)));

    layout->addWidget( new QLabel(tr("Cofficients:")), 0, 0);
    layout->addWidget(m_layeringModelComboBox, 0, 1);

    layout->addWidget(new QLabel(
                QString("Layer rate model: %1(d) = <b><i>a</i></b> (d + <b><i>b</i></b>)<sup><b><i>c</i></b></sup>").arg(QChar(0x03BB))), 1, 0, 1, 3);
    // Coefficient line
    m_layeringCoeffSpinBox = new QDoubleSpinBox;
    m_layeringCoeffSpinBox->setRange( 0, 10 );
    m_layeringCoeffSpinBox->setDecimals(2);
    m_layeringCoeffSpinBox->setSingleStep(0.1);

    connect( m_layeringCoeffSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setLayeringCoeff(double)));
    
    layout->addWidget(new QLabel(tr("Coefficient (<b><i>a</i></b>):")), 2, 0);
    layout->addWidget(m_layeringCoeffSpinBox, 2, 1);

    // Initial line
    m_layeringInitialSpinBox = new QDoubleSpinBox;
    m_layeringInitialSpinBox->setRange( 0, 100 );
    m_layeringInitialSpinBox->setDecimals(2);
    m_layeringInitialSpinBox->setSingleStep(0.1);

    connect( m_layeringInitialSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setLayeringInitial(double)));
    
    layout->addWidget(new QLabel(tr("Initial (<b><i>b</b></i>):")), 3, 0);
    layout->addWidget(m_layeringInitialSpinBox, 3, 1);

    // Exponent line
    m_layeringExponentSpinBox = new QDoubleSpinBox;
    m_layeringExponentSpinBox->setRange( -2, 0 );
    m_layeringExponentSpinBox->setDecimals(2);
    m_layeringExponentSpinBox->setSingleStep(0.01);
    connect( m_layeringExponentSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation(), SLOT(setLayeringExponent(double)));

    layout->addWidget(new QLabel(tr("Exponent (<b><i>c</b></i>):")), 4, 0);
    layout->addWidget(m_layeringExponentSpinBox, 4, 1);
    
    // Create the group box and set the layout
    m_layerVariationGroupBox = new QGroupBox(tr("Layer Thickness Variation"));
    m_layerVariationGroupBox->setVisible(false);

    m_layerVariationGroupBox->setLayout(layout);
}

void SoilProfilePage::createBedrockDepthGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Distribution
    m_bedrockModelComboBox = new QComboBox;
    m_bedrockModelComboBox->addItems(Distribution::typeList());

    connect(m_bedrockModelComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateBedrockModel(int)));
    connect(m_bedrockModelComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setType(int)));

    layout->addWidget( new QLabel(tr("Distribution:")), 0, 0);
    layout->addWidget( m_bedrockModelComboBox, 0, 1);

    // Standard deviation
    m_bedrockStdevSpinBox = new QDoubleSpinBox;
    m_bedrockStdevSpinBox->setRange( 0, 500 );
    m_bedrockStdevSpinBox->setSingleStep(0.1);
    m_bedrockStdevSpinBox->setDecimals(2);
    connect( m_bedrockStdevSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setStdev(double)));
                

    layout->addWidget( new QLabel(tr("Standard deviation:")), 1, 0);
    layout->addWidget( m_bedrockStdevSpinBox, 1, 1);

    
    // Minimum
    m_bedrockDepthMinCheckBox = new QCheckBox(tr("Minimum depth to bedrock:"));
    connect( m_bedrockDepthMinCheckBox, SIGNAL(toggled(bool)),
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setHasMin(bool)));
    
    m_bedrockDepthMinSpinBox = new QDoubleSpinBox;
    m_bedrockDepthMinSpinBox->setEnabled(false);
    m_bedrockDepthMinSpinBox->setRange( 0, 10000);
    connect( m_bedrockDepthMinSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setMin(double)));
    connect( m_bedrockDepthMinCheckBox, SIGNAL(toggled(bool)), 
            m_bedrockDepthMinSpinBox, SLOT(setEnabled(bool)));

    layout->addWidget( m_bedrockDepthMinCheckBox, 2, 0);
    layout->addWidget( m_bedrockDepthMinSpinBox, 2, 1);

    // Maximum
    m_bedrockDepthMaxCheckBox = new QCheckBox(tr("Maximum depth to bedrock:"));
    connect( m_bedrockDepthMaxCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setHasMax(bool)));
    
    m_bedrockDepthMaxSpinBox = new QDoubleSpinBox;
    m_bedrockDepthMaxSpinBox->setEnabled(false);
    m_bedrockDepthMaxSpinBox->setRange( 0, 10000);
    connect( m_bedrockDepthMaxSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->profileVariation()->bedrockDepth(), SLOT(setMax(double)));

    connect( m_bedrockDepthMaxCheckBox, SIGNAL(toggled(bool)), 
            m_bedrockDepthMaxSpinBox, SLOT(setEnabled(bool)));

    layout->addWidget( m_bedrockDepthMaxCheckBox, 3, 0);
    layout->addWidget( m_bedrockDepthMaxSpinBox, 3, 1);

    // Create the group box
    m_bedrockDepthGroupBox = new QGroupBox(tr("Bedrock Depth Variation"));
    //m_bedrockDepthGroupBox->setEnabled(false);
    m_bedrockDepthGroupBox->setVisible(false);

    m_bedrockDepthGroupBox->setLayout(layout);
}

void SoilProfilePage::setIsVaried(bool isVaried)
{
    m_profileVariationGroupBox->setVisible(isVaried);

    if ( isVaried && m_isVelocityVariedCheckBox->isChecked() )
        setVelocityIsVaried(true);
    else
        setVelocityIsVaried(false);
}

void SoilProfilePage::setVelocityIsVaried(bool isVaried)
{
    if (isVaried){
        // Enable the isVaried column
        if ( m_layerSpecificCheckBox->checkState() )
            m_tableGroupBox->table()->showColumn(4);
        m_tableGroupBox->table()->showColumn(5);
        m_tableGroupBox->table()->showColumn(6);
        m_tableGroupBox->table()->showColumn(7);
        
    } else {
        // Disable the isVaried column
        m_tableGroupBox->table()->hideColumn(4);
        m_tableGroupBox->table()->hideColumn(5);
        m_tableGroupBox->table()->hideColumn(6);
        m_tableGroupBox->table()->hideColumn(7);
    }    
}

void SoilProfilePage::setReadOnly(bool b)
{
    m_tableGroupBox->setReadOnly(b);
    m_profileVariationGroupBox->setDisabled(b);
}

void SoilProfilePage::updateStdevModel(int model)
{
    // Enable the standard deviation in the model is custom
   if ( model == ProfileVariation::Custom )
       m_stdevSpinBox->setEnabled(true);
   else
       m_stdevSpinBox->setEnabled(false);
   // Update the model properties
   m_model->siteProfile()->profileVariation()->setStdevModel((ProfileVariation::VelocityModel)model);
   // Load the standard deviation
   loadStdev();
}

void SoilProfilePage::updateCorrelModel(int model)
{
   // Enable the group box if the model is custom 0
   if ( model == ProfileVariation::Custom ) {
       m_correlInitialSpinBox->setEnabled(true);
       m_correlFinalSpinBox->setEnabled(true);
       m_correlDeltaSpinBox->setEnabled(true);
       m_depthInterceptSpinBox->setEnabled(true);
       m_exponentSpinBox->setEnabled(true);
   } else {
       m_correlInitialSpinBox->setEnabled(false);
       m_correlFinalSpinBox->setEnabled(false);
       m_correlDeltaSpinBox->setEnabled(false);
       m_depthInterceptSpinBox->setEnabled(false);
       m_exponentSpinBox->setEnabled(false);
   }
   // Update the model properties
   m_model->siteProfile()->profileVariation()->setCorrelModel((ProfileVariation::VelocityModel)model);
   // Load the properties
   loadCorrel();
}

void SoilProfilePage::updateLayeringModel(int model)
{
    // Enable the group box if the model is custom 0
    if ( model == ProfileVariation::CustomLayering ) {
        m_layeringCoeffSpinBox->setEnabled(true);
        m_layeringInitialSpinBox->setEnabled(true);
        m_layeringExponentSpinBox->setEnabled(true);
    } else {
        m_layeringCoeffSpinBox->setEnabled(false);
        m_layeringInitialSpinBox->setEnabled(false);
        m_layeringExponentSpinBox->setEnabled(false);
    }
    // Set the model
    m_model->siteProfile()->profileVariation()->setLayeringModel((ProfileVariation::LayeringModel)model);
    // Load the parameters
    loadLayering();
}

void SoilProfilePage::updateBedrockModel(int model)
{
    if ( model == Distribution::Uniform ) {
        // Disable the median and standard deviation
        m_bedrockStdevSpinBox->setEnabled(false);

        m_bedrockDepthMinCheckBox->setChecked(true);
        m_bedrockDepthMaxCheckBox->setChecked(true);
    } else {
        // Enable the median and standard deviation
        m_bedrockStdevSpinBox->setEnabled(true);
    }
}    
        
void SoilProfilePage::setIsLayerSpecific(bool state)
{
    if (state) 
    {
        // Save the old data
        m_model->siteProfile()->profileVariation()->setStdev(m_stdevSpinBox->text().toDouble());

        m_stdevSpinBox->clear();
        // Show the appropriate columns
        m_tableGroupBox->table()->showColumn(4);
        // Enable the standard deviation line edit
        m_stdevModelComboBox->setEnabled(false);
        if (m_stdevModelComboBox->currentIndex() == ProfileVariation::Custom)
            m_stdevSpinBox->setEnabled(false);
    } else  {
        m_stdevSpinBox->setValue(m_model->siteProfile()->profileVariation()->stdev());
        // Hide the appropriate columns
        m_tableGroupBox->table()->hideColumn(4);
        // Disable the standard deviation line edit
        m_stdevModelComboBox->setEnabled(true);
        if (m_stdevModelComboBox->currentIndex() == ProfileVariation::Custom) {
            m_stdevSpinBox->setEnabled(true);
        }
    }
}
    
void SoilProfilePage::load()
{
    // Site profile variation
    m_isVelocityVariedCheckBox->setChecked(m_model->siteProfile()->profileVariation()->isVelocityVaried());
    m_isLayeringVariedCheckBox->setChecked(m_model->siteProfile()->profileVariation()->isLayeringVaried());
    m_isBedrockDepthVariedCheckBox->setChecked(m_model->siteProfile()->profileVariation()->isBedrockDepthVaried());
    m_layerSpecificCheckBox->setChecked(m_model->siteProfile()->profileVariation()->stdevIsLayerSpecific());

    // Load the standard deviation
    loadStdev();

    // Load the correlation
    loadCorrel();

    // Load the layering variation
    loadLayering();

    // Load the bedrock variation parameters
    m_bedrockModelComboBox->setCurrentIndex(
            m_model->siteProfile()->profileVariation()->bedrockDepth()->type());
    m_bedrockStdevSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->bedrockDepth()->stdev());

    m_bedrockDepthMinCheckBox->setChecked(
            m_model->siteProfile()->profileVariation()->bedrockDepth()->hasMin());
    m_bedrockDepthMinSpinBox->setValue( 
            m_model->siteProfile()->profileVariation()->bedrockDepth()->min());
    m_bedrockDepthMaxCheckBox->setChecked(
            m_model->siteProfile()->profileVariation()->bedrockDepth()->hasMax());
    m_bedrockDepthMaxSpinBox->setValue( 
            m_model->siteProfile()->profileVariation()->bedrockDepth()->max());
}

void SoilProfilePage::updateUnits()
{
    m_bedrockDepthMinSpinBox->setSuffix(" " + Units::instance()->length());
    m_bedrockDepthMaxSpinBox->setSuffix(" " + Units::instance()->length());

    QList<QComboBox*> list =  QList<QComboBox*>() << m_stdevModelComboBox << m_correlModelComboBox;

    foreach( QComboBox * comboBox, list ) {
        int index = comboBox->currentIndex();

        comboBox->blockSignals(true);
        comboBox->clear();
        comboBox->addItems( ProfileVariation::velocityModelList() );
        comboBox->setCurrentIndex(index);
        comboBox->blockSignals(false);
    }
}

void SoilProfilePage::loadStdev()
{
    // Standard deviation model
    m_stdevModelComboBox->setCurrentIndex((int)m_model->siteProfile()->profileVariation()->stdevModel());
    // Standard deviation value 
    m_stdevSpinBox->setValue(m_model->siteProfile()->profileVariation()->stdev());
}

void SoilProfilePage::loadCorrel()
{
    // Correlation combo box
    m_correlModelComboBox->setCurrentIndex((int)m_model->siteProfile()->profileVariation()->correlModel());
    // Values
    m_correlInitialSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->correlInitial());
    m_correlFinalSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->correlFinal());
    m_correlDeltaSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->correlDelta());
    m_depthInterceptSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->correlIntercept());
    m_exponentSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->correlExponent());
}

void SoilProfilePage::loadLayering()
{
    m_layeringModelComboBox->setCurrentIndex(
            m_model->siteProfile()->profileVariation()->layeringModel());
    m_layeringCoeffSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->layeringCoeff());
    m_layeringInitialSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->layeringInitial());
    m_layeringExponentSpinBox->setValue(
                m_model->siteProfile()->profileVariation()->layeringExponent());
}
