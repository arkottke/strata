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
#include "SoilTypeTableModel.h"
#include "NonLinearPropertyTableModel.h"
#include "StringListDelegate.h"

#include <QDoubleValidator>
#include <QLabel>
#include <QTableView>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

SoilTypePage::SoilTypePage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f )
	: QWidget(parent, f), m_model(model)
{
    m_selectedNonLinearProperty = 0;
    m_selectedSoilType = 0;
	// Setup the Group Boxes
	createLayersGroupBox();
    createBedrockGroupBox();
	createVariationGroupBox();
    createSoilPropsGroupBox();
	createNlPropTableBox();

    // Set the layout
    QGridLayout * layout = new QGridLayout;
    
    layout->addWidget( m_layersTableBox, 0, 0, 2, 1);
    layout->addWidget( m_bedrockGroupBox, 2, 0);
    layout->addWidget( m_variationGroupBox, 3, 0);
    layout->addWidget( m_soilPropsGroupBox, 0, 1);
    layout->addWidget( m_nlPropTableBox, 1, 1, 3, 1);

    layout->setColumnStretch( 0, 1 );
    layout->setRowStretch( 1, 1 );

	setLayout(layout);

    // Load the values
    load();
    
    // Load the units
    updateUnits();
    
    // Intially hide the columns on the table
    setIsVaried(false);

    // Connections
    connect( m_model->units(), SIGNAL(systemChanged()), this, SLOT(updateUnits()));
}

void SoilTypePage::setModel( SiteResponseModel * model )
{
    m_model = model;

    delete m_layersTableBox->table()->model();
    m_layersTableBox->table()->setModel(
            new SoilTypeTableModel( m_model->nlPropertyLibrary(), m_model->siteProfile(), m_model->units()));
}

void SoilTypePage::createLayersGroupBox()
{
    m_layersTableBox = new TableGroupBox(
            new SoilTypeTableModel( m_model->nlPropertyLibrary(), m_model->siteProfile(), m_model->units()), tr("Soil Types"));

    m_layersTableBox->table()->setItemDelegateForColumn( 3, new StringListDelegate );
    m_layersTableBox->table()->setItemDelegateForColumn( 4, new StringListDelegate );

	// Connections
	connect(m_layersTableBox->table()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(setSelectedSoil(QModelIndex,QModelIndex)));
    connect(m_layersTableBox, SIGNAL(rowRemoved()), this, SLOT(unselectSoil()));
    connect(m_layersTableBox, SIGNAL(dataChanged()), this, SLOT(refreshSoilDetails()));
    connect(m_layersTableBox, SIGNAL(dataChanged()), this, SIGNAL(hasChanged()));
}

void SoilTypePage::createBedrockGroupBox()
{
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
	m_bedrockUntWtSpinBox = new QDoubleSpinBox;
    m_bedrockUntWtSpinBox->setRange(10,200);
    connect(m_bedrockUntWtSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

    layout->addWidget(new QLabel(tr("Unit weight:")));
    layout->addWidget(m_bedrockUntWtSpinBox);

    // Damping
    m_bedrockDampingSpinBox = new QDoubleSpinBox;
    m_bedrockDampingSpinBox->setSuffix(" %");
    m_bedrockDampingSpinBox->setDecimals(2);
    m_bedrockDampingSpinBox->setRange(0.01,5);
    m_bedrockDampingSpinBox->setSingleStep(0.01);
    connect(m_bedrockDampingSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));
    
    layout->addWidget(new QLabel(tr("Damping:")));
    layout->addWidget(m_bedrockDampingSpinBox);
    
    // Damping variation in the bedrock
    m_varyBedrockDampingCheckBox = new QCheckBox(tr("Vary the damping of the bedrock"));
    m_varyBedrockDampingCheckBox->setEnabled(false);
    connect(m_varyBedrockDampingCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(hasChanged()));

	layout->addWidget( m_varyBedrockDampingCheckBox);

    // Add a stretch to the end
    layout->addStretch(1);

    // Create the group box
    m_bedrockGroupBox = new QGroupBox(tr("Bedrock Layer"));
    m_bedrockGroupBox->setLayout(layout);
}

void SoilTypePage::createVariationGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 1, 1);

    // Model for the standard deviation
    m_dynPropModelComboBox = new QComboBox;
    m_dynPropModelComboBox->addItems(NonLinearPropertyVariation::modelList());
    connect(m_dynPropModelComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(hasChanged()));
    connect(m_dynPropModelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(loadStdevModels()));
    
    
    // Link for help on standard deviation models
    QLabel * stdevLabel = new QLabel(tr(
                "Standard deviation model (<a href=\"soil-type.html#stdev-models\">more information</a>)"));
    connect( stdevLabel, SIGNAL(linkActivated(QString)), this, SIGNAL(linkActivated(QString)));


    layout->addWidget( stdevLabel, 0, 0, 1, 2);
    layout->addWidget( m_dynPropModelComboBox, 0, 3);

    layout->addWidget( new QLabel(tr("Normalized shear modulus (G/G_max):")), 1, 0, 1, 2);

    // Shear modulus standard deviation, maximum, and minimum
    m_shearModStdevLineEdit = new QLineEdit;
    connect(m_shearModStdevLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));
    
    m_shearModMaxSpinBox = new QDoubleSpinBox;
    m_shearModMaxSpinBox->setRange( 1.00, 2.00 );
    m_shearModMaxSpinBox->setDecimals(2);
    m_shearModMaxSpinBox->setSingleStep(0.05);
    connect(m_shearModMaxSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));
    
    m_shearModMinSpinBox = new QDoubleSpinBox;
    m_shearModMinSpinBox->setRange( 0.05, 0.40 );
    m_shearModMinSpinBox->setDecimals(2);
    m_shearModMinSpinBox->setSingleStep(0.05);
    connect(m_shearModMinSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Stdev:")), 2, 0);
    layout->addWidget( m_shearModStdevLineEdit, 2, 1);
    layout->addWidget( new QLabel(tr("Max:")), 2, 2);
    layout->addWidget( m_shearModMaxSpinBox, 2, 3);
    layout->addWidget( new QLabel(tr("Min:")), 2, 4);
    layout->addWidget( m_shearModMinSpinBox, 2, 5);
    
    layout->addWidget( new QLabel(tr("Damping:")), 3, 0, 1, 2);
    // Damping standard deviation and minimum
    m_dampingStdevLineEdit = new QLineEdit;
    connect(m_dampingStdevLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    m_dampingMinSpinBox = new QDoubleSpinBox;
    m_dampingMinSpinBox->setSuffix(" %");
    m_dampingMinSpinBox->setRange( 0.01, 1 );
    m_dampingMinSpinBox->setDecimals(2);
    m_dampingMinSpinBox->setSingleStep(0.01);
    connect(m_dampingMinSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Stdev:")), 4, 0);
    layout->addWidget( m_dampingStdevLineEdit, 4, 1);
    layout->addWidget( new QLabel(tr("Min:")), 4, 4);
    layout->addWidget( m_dampingMinSpinBox, 4, 5);
	
    // Correlation
	m_correlSpinBox = new QDoubleSpinBox;
    m_correlSpinBox->setRange( -1, 1 );
    m_correlSpinBox->setDecimals(2);
    m_correlSpinBox->setSingleStep(0.1);
    connect(m_correlSpinBox, SIGNAL(valueChanged(double)), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(QString(tr("G/G_max, Damping Correlation Coefficent (%1):")).arg(QChar(0x03C1))), 5, 0, 1, 2);
	layout->addWidget( m_correlSpinBox, 5, 3);
    
	// Group box
	m_variationGroupBox = new QGroupBox(tr("Non-Linear Curve Variation Parameters"));
	m_variationGroupBox->setLayout(layout);
    m_variationGroupBox->setVisible(false);
}

void SoilTypePage::createSoilPropsGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    
    // Stress line
    m_stressLineEdit = new QLineEdit;
	m_stressLineEdit->setValidator(new QDoubleValidator(m_stressLineEdit));
    connect(m_stressLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget(new QLabel(tr("Mean stress (atm):")), 0, 0);
    layout->addWidget(m_stressLineEdit, 0, 2);

    // Plasticity line
    m_piLineEdit = new QLineEdit; 
	m_piLineEdit->setValidator( new QDoubleValidator(m_piLineEdit) );
    connect(m_piLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));
    
    layout->addWidget(new QLabel(tr("Plasticity Index (%):")), 1, 0);
    layout->addWidget(m_piLineEdit, 1, 2);

    // OCR line
    m_ocrLineEdit = new QLineEdit;
	m_ocrLineEdit->setValidator( new QDoubleValidator(m_ocrLineEdit) );
    connect(m_ocrLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget(new QLabel(tr("Over-consolidation ratio:")), 2, 0);
    layout->addWidget(m_ocrLineEdit, 2, 2);

    // Frequency line
    m_freqLineEdit = new QLineEdit;
	m_freqLineEdit->setValidator( new QDoubleValidator(m_ocrLineEdit) );
    connect(m_freqLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget(new QLabel(tr("Excitation frequency (Hz):")), 3, 0);
    layout->addWidget(m_freqLineEdit, 3, 2);
    
    // Cycles line
    m_nCyclesLineEdit = new QLineEdit;
	m_nCyclesLineEdit->setValidator( new QDoubleValidator(m_ocrLineEdit) );
    connect(m_nCyclesLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget(new QLabel(tr("Number of cycles:")), 4, 0);
    layout->addWidget(m_nCyclesLineEdit, 4, 2);

    // Create the group box
    m_soilPropsGroupBox = new QGroupBox(tr("Darendeli & Stokoe Model Parameters"));
    m_soilPropsGroupBox->setLayout(layout);
    m_soilPropsGroupBox->setEnabled(false);
}

void SoilTypePage::createNlPropTableBox()
{
    m_nlPropTableBox = new TableGroupBox(
            new NonLinearPropertyTableModel( 0 ), tr("Non-linear property"));
    m_nlPropTableBox->setEnabled(false);
}

void SoilTypePage::setIsVaried(bool isVaried)
{
    if (isVaried) {
        m_layersTableBox->table()->showColumn(6);
        m_variationGroupBox->setVisible(true);
        m_varyBedrockDampingCheckBox->setEnabled(true);
    } else {
        m_layersTableBox->table()->hideColumn(6);
        m_variationGroupBox->setVisible(false);
        m_varyBedrockDampingCheckBox->setEnabled(false);
    }
}

void SoilTypePage::updateUnits()
{
    m_bedrockUntWtSpinBox->setSuffix( " " + m_model->units()->untWt() );
}

void SoilTypePage::unselectSoil()
{
    m_nlPropTableBox->setEnabled(false);
    m_soilPropsGroupBox->setEnabled(false);

    m_selectedSoilType = 0;
    m_selectedNonLinearProperty = 0;
}

void SoilTypePage::setSelectedSoil(const QModelIndex & current, const QModelIndex & /*previous*/)
{
    // Save the index 
    m_selectedIndex = current;
    // The selected soil type
    m_selectedSoilType = m_model->siteProfile().soilTypes().at(m_selectedIndex.row());
    
    refreshSoilDetails();
}

void SoilTypePage::refreshSoilDetails()
{
    // Disable the nonlinear property group box
    m_nlPropTableBox->setEnabled(false);
    m_soilPropsGroupBox->setEnabled(false);
    
    if ( !m_selectedSoilType )
        return;

    if ( m_selectedIndex.column() == 3 || m_selectedIndex.column() == 4 ) {
        // Set the model for the dynamic properties
        if ( m_selectedIndex.column() == 3 )
            m_selectedNonLinearProperty = const_cast<NonLinearProperty*>(&(m_selectedSoilType->normShearMod()));
        else if ( m_selectedIndex.column() == 4 )
            m_selectedNonLinearProperty = const_cast<NonLinearProperty*>(&(m_selectedSoilType->damping()));

        // If the source is Darendeli and the curves are empty, re-compute the curves
        if ( m_selectedNonLinearProperty->source() == NonLinearProperty::Computed
                && m_selectedNonLinearProperty->avg().isEmpty() )
            m_selectedSoilType->computeDarendeliCurves();


        // Update the dynamic property group box
        static_cast<NonLinearPropertyTableModel*>(m_nlPropTableBox->table()->model())->setNonLinearProperty(m_selectedNonLinearProperty);

        // Enable the nonlinear property group box
        m_nlPropTableBox->setEnabled(true);
        
        if (m_selectedNonLinearProperty->source() == NonLinearProperty::Temporary)
            m_nlPropTableBox->setEditable(true);
        else
            m_nlPropTableBox->setEditable(false);
    
        // Refresh the width of the table
        m_nlPropTableBox->table()->resizeColumnsToContents();
    }

    // If the Stokoe-Darendeli option is selected for either model enable the
    // soil properties box.
    if ( m_selectedSoilType->normShearMod().source() == NonLinearProperty::Computed 
            || m_selectedSoilType->damping().source() == NonLinearProperty::Computed ) 
    {
        m_soilPropsGroupBox->setEnabled(true);
        // Load the soil properties
        loadSoilProperties();
        // Clear the connections
        disconnect(m_stressLineEdit, SIGNAL(editingFinished()), 0, 0);
        disconnect(m_piLineEdit, SIGNAL(editingFinished()), 0, 0);
        disconnect(m_ocrLineEdit, SIGNAL(editingFinished()), 0, 0);
        disconnect(m_freqLineEdit, SIGNAL(editingFinished()), 0, 0);
        disconnect(m_nCyclesLineEdit, SIGNAL(editingFinished()), 0, 0);
        // Form the connections
        connect(m_stressLineEdit, SIGNAL(editingFinished()),  this, SLOT(saveSoilProperties()));
        connect(m_piLineEdit, SIGNAL(editingFinished()),  this, SLOT(saveSoilProperties()));
        connect(m_ocrLineEdit, SIGNAL(editingFinished()),  this, SLOT(saveSoilProperties()));
        connect(m_freqLineEdit, SIGNAL(editingFinished()),  this, SLOT(saveSoilProperties()));
        connect(m_nCyclesLineEdit, SIGNAL(editingFinished()),  this, SLOT(saveSoilProperties()));
    } 
    else 
    {
        // Clear the line edits
	    m_stressLineEdit->clear();
	    m_piLineEdit->clear();
	    m_ocrLineEdit->clear();
	    m_freqLineEdit->clear();
	    m_nCyclesLineEdit->clear();
    }
}

void SoilTypePage::save()
{
    // Bedrock parameters
    m_model->siteProfile().bedrock()->setUntWt(
            m_bedrockUntWtSpinBox->value());
    m_model->siteProfile().bedrock()->setAvgDamping(
            m_bedrockDampingSpinBox->value());
    m_model->siteProfile().nonLinearPropertyVariation().setBedrockIsEnabled(m_varyBedrockDampingCheckBox->isChecked());

    // Variation parameters
    m_model->siteProfile().nonLinearPropertyVariation().setModel(
            (NonLinearPropertyVariation::Model) m_dynPropModelComboBox->currentIndex());
    m_model->siteProfile().nonLinearPropertyVariation().setShearModStdev(m_shearModStdevLineEdit->text());
    m_model->siteProfile().nonLinearPropertyVariation().setShearModMax(m_shearModMaxSpinBox->value());
    m_model->siteProfile().nonLinearPropertyVariation().setShearModMin(m_shearModMinSpinBox->value());
    m_model->siteProfile().nonLinearPropertyVariation().setDampingStdev(m_dampingStdevLineEdit->text());
    m_model->siteProfile().nonLinearPropertyVariation().setDampingMin(m_dampingMinSpinBox->value());
    m_model->siteProfile().nonLinearPropertyVariation().setCorrel(m_correlSpinBox->value());
}

void SoilTypePage::load()
{
    // Bedrock parameters
    m_bedrockUntWtSpinBox->setValue(m_model->siteProfile().bedrock()->untWt());
    m_bedrockDampingSpinBox->setValue(m_model->siteProfile().bedrock()->avgDamping());
    m_varyBedrockDampingCheckBox->setChecked(m_model->siteProfile().nonLinearPropertyVariation().bedrockIsEnabled());

    // Variation parameters
    m_dynPropModelComboBox->setCurrentIndex(
            m_model->siteProfile().nonLinearPropertyVariation().model());
    m_shearModMaxSpinBox->setValue(m_model->siteProfile().nonLinearPropertyVariation().shearModMax());
    m_shearModMinSpinBox->setValue(m_model->siteProfile().nonLinearPropertyVariation().shearModMin());
    m_dampingMinSpinBox->setValue(m_model->siteProfile().nonLinearPropertyVariation().dampingMin());
    m_correlSpinBox->setValue(m_model->siteProfile().nonLinearPropertyVariation().correl());
    
    loadStdevModels();
}

void SoilTypePage::loadStdevModels()
{
    m_shearModStdevLineEdit->setText(m_model->siteProfile().nonLinearPropertyVariation().shearModStdev());
    m_dampingStdevLineEdit->setText(m_model->siteProfile().nonLinearPropertyVariation().dampingStdev());

    // Disable the line edits for Darendeli and Stokoe
    if ( m_dynPropModelComboBox->currentIndex() == NonLinearPropertyVariation::Darendeli ) {
        m_shearModStdevLineEdit->setEnabled(false);
        m_dampingStdevLineEdit->setEnabled(false);
    } else {
        m_shearModStdevLineEdit->setEnabled(true);
        m_dampingStdevLineEdit->setEnabled(true);
    }
} 

void SoilTypePage::saveSoilProperties()
{
    if (!m_selectedSoilType)
        return;

    bool modified = false;

    // Save the line edits
    if ( m_stressLineEdit->isModified() ) {
        m_selectedSoilType->setMeanStress( m_stressLineEdit->text().toDouble() );
        modified = true;
    }

    if ( m_piLineEdit->isModified() ) {
        m_selectedSoilType->setPI( m_piLineEdit->text().toDouble() );
        modified = true;
    }

    if ( m_ocrLineEdit->isModified() ) {
        m_selectedSoilType->setOCR( m_ocrLineEdit->text().toDouble() );
        modified = true;
    }

    if ( m_freqLineEdit->isModified() ) {
        m_selectedSoilType->setFreq( m_freqLineEdit->text().toDouble() );
        modified = true;
    }

    if ( m_nCyclesLineEdit->isModified() ) {
        m_selectedSoilType->setNCycles( m_nCyclesLineEdit->text().toDouble() );
        modified = true;
    }

    if (modified) {
        // Compute the new values
        m_selectedSoilType->computeDarendeliCurves();
        // Update the table
        static_cast<NonLinearPropertyTableModel*>(m_nlPropTableBox->table()->model())->resetModel();
    }
}

void SoilTypePage::loadSoilProperties()
{
    if (!m_selectedSoilType)
        return;
    // Set the line edits
    m_stressLineEdit->setText( QString::number( m_selectedSoilType->meanStress()));
    m_piLineEdit->setText( QString::number( m_selectedSoilType->PI()));
    m_ocrLineEdit->setText( QString::number( m_selectedSoilType->OCR()));
    m_freqLineEdit->setText( QString::number( m_selectedSoilType->freq()));
    m_nCyclesLineEdit->setText( QString::number( m_selectedSoilType->nCycles()));
}
