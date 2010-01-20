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

#include "NonlinearPropertyTableModel.h"
#include "SoilTypePage.h"
#include "SoilTypeTableModel.h"
#include "StringListDelegate.h"
#include "Units.h"

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
    m_selectedNonlinearProperty = 0;
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
    connect( Units::instance(), SIGNAL(systemChanged()), this, SLOT(updateUnits()));
}

void SoilTypePage::createLayersGroupBox()
{
    m_layersTableBox = new TableGroupBox(
            new SoilTypeTableModel( m_model->nlPropertyLibrary(), m_model->siteProfile(), Units::instance()), tr("Soil Types"));
    connect( m_layersTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
    connect( m_layersTableBox, SIGNAL(dataChanged()), SIGNAL(soilTypesChanged())); 

    m_layersTableBox->table()->setItemDelegateForColumn( 3, new StringListDelegate );
    m_layersTableBox->table()->setItemDelegateForColumn( 4, new StringListDelegate );

	// Connections
	connect(m_layersTableBox->table()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(setSelectedSoil(QModelIndex,QModelIndex)));
    connect(m_layersTableBox, SIGNAL(rowRemoved()), SLOT(unselectSoil()));
    connect(m_layersTableBox, SIGNAL(dataChanged()), SLOT(refreshSoilDetails()));
    connect(m_layersTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
}

void SoilTypePage::createBedrockGroupBox()
{
    QHBoxLayout * layout = new QHBoxLayout;

    // Unit weight
	m_bedrockUntWtSpinBox = new QDoubleSpinBox;
    m_bedrockUntWtSpinBox->setRange(10,200);
    m_bedrockUntWtSpinBox->setDecimals(2);

    connect(m_bedrockUntWtSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->bedrock(), SLOT(setUntWt(double)));

    layout->addWidget(new QLabel(tr("Unit weight:")));
    layout->addWidget(m_bedrockUntWtSpinBox);

    // Damping
    m_bedrockDampingSpinBox = new QDoubleSpinBox;
    m_bedrockDampingSpinBox->setSuffix(" %");
    m_bedrockDampingSpinBox->setDecimals(2);
    m_bedrockDampingSpinBox->setRange(0.1,5);
    m_bedrockDampingSpinBox->setSingleStep(0.1);
    connect(m_bedrockDampingSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->bedrock(), SLOT(setAvgDamping(double)));
    
    layout->addWidget(new QLabel(tr("Damping:")));
    layout->addWidget(m_bedrockDampingSpinBox);
    
    // Damping variation in the bedrock
    m_varyBedrockDampingCheckBox = new QCheckBox(tr("Vary the damping of the bedrock"));
    connect(m_varyBedrockDampingCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setBedrockIsEnabled(bool)));

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
    m_dynPropModelComboBox->addItems(NonlinearPropertyVariation::modelList());
    connect(m_dynPropModelComboBox, SIGNAL(currentIndexChanged(int)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setModel(int)));
    connect(m_dynPropModelComboBox, SIGNAL(currentIndexChanged(int)), SLOT(loadStdevModels()));
    
    
    // Link for help on standard deviation models
    QLabel * stdevLabel = new QLabel(tr(
                "Standard deviation model (<a href=\"qrc:/docs/soil-type.html#stdev-models\">more information</a>):"));
    connect( stdevLabel, SIGNAL(linkActivated(QString)), this, SIGNAL(linkActivated(QString)));


    layout->addWidget( stdevLabel, 0, 0, 1, 2);
    layout->addWidget( m_dynPropModelComboBox, 0, 3);

    layout->addWidget( new QLabel(tr("Normalized shear modulus (G/G_max):")), 1, 0, 1, 2);

    // Shear modulus standard deviation, maximum, and minimum
    m_shearModStdevLineEdit = new QLineEdit;
    connect(m_shearModStdevLineEdit, SIGNAL(textChanged(QString)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setShearModStdev(QString)));
    
    m_shearModMinSpinBox = new QDoubleSpinBox;
    m_shearModMinSpinBox->setRange( 0.05, 0.40 );
    m_shearModMinSpinBox->setDecimals(2);
    m_shearModMinSpinBox->setSingleStep(0.05);
    connect(m_shearModMinSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setShearModMin(double)));
    
    m_shearModMaxSpinBox = new QDoubleSpinBox;
    m_shearModMaxSpinBox->setRange( 1.00, 2.00 );
    m_shearModMaxSpinBox->setDecimals(2);
    m_shearModMaxSpinBox->setSingleStep(0.05);
    connect(m_shearModMaxSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setShearModMax(double)));

    layout->addWidget( new QLabel(tr("Stdev:")), 2, 0);
    layout->addWidget( m_shearModStdevLineEdit, 2, 1);
    layout->addWidget( new QLabel(tr("Min:")), 2, 2);
    layout->addWidget( m_shearModMinSpinBox, 2, 3);
    layout->addWidget( new QLabel(tr("Max:")), 2, 4);
    layout->addWidget( m_shearModMaxSpinBox, 2, 5);
    
    layout->addWidget( new QLabel(tr("Damping:")), 3, 0, 1, 2);
    // Damping standard deviation and minimum
    m_dampingStdevLineEdit = new QLineEdit;
    connect(m_dampingStdevLineEdit, SIGNAL(textChanged(QString)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setDampingStdev(QString)));

    m_dampingMinSpinBox = new QDoubleSpinBox;
    m_dampingMinSpinBox->setSuffix(" %");
    m_dampingMinSpinBox->setRange( 0.01, 1 );
    m_dampingMinSpinBox->setDecimals(2);
    m_dampingMinSpinBox->setSingleStep(0.01);
    connect(m_dampingMinSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setDampingMin(double)));
    
    m_dampingMaxSpinBox = new QDoubleSpinBox;
    m_dampingMaxSpinBox->setSuffix(" %");
    m_dampingMaxSpinBox->setRange( 10, 40 );
    m_dampingMaxSpinBox->setDecimals(1);
    m_dampingMaxSpinBox->setSingleStep(1);
    connect(m_dampingMaxSpinBox, SIGNAL(valueChanged(double)),
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setDampingMax(double)));

    layout->addWidget( new QLabel(tr("Stdev:")), 4, 0);
    layout->addWidget( m_dampingStdevLineEdit, 4, 1);
    layout->addWidget( new QLabel(tr("Min:")), 4, 2);
    layout->addWidget( m_dampingMinSpinBox, 4, 3);
    layout->addWidget( new QLabel(tr("Max:")), 4, 4);
    layout->addWidget( m_dampingMaxSpinBox, 4, 5);
	
    // Correlation
	m_correlSpinBox = new QDoubleSpinBox;
    m_correlSpinBox->setRange( -1, 1 );
    m_correlSpinBox->setDecimals(2);
    m_correlSpinBox->setSingleStep(0.1);
    connect(m_correlSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setCorrel(double)));

	layout->addWidget( new QLabel(QString(tr("G/G_max, Damping Correlation Coefficent (%1):")).arg(QChar(0x03C1))), 5, 0, 1, 2);
	layout->addWidget( m_correlSpinBox, 5, 3);
    
	// Group box
	m_variationGroupBox = new QGroupBox(tr("Nonlinear Curve Variation Parameters"));
	m_variationGroupBox->setLayout(layout);
    m_variationGroupBox->setVisible(false);
}

void SoilTypePage::createSoilPropsGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    
    // Stress line
    m_stressSpinBox = new QDoubleSpinBox;
    m_stressSpinBox->setDecimals(2);
    m_stressSpinBox->setRange( 0.1, 200);
    m_stressSpinBox->setSingleStep(1);
    m_stressSpinBox->setSuffix(" atm");

    layout->addWidget(new QLabel(tr("Mean effective stress:")), 0, 0);
    layout->addWidget(m_stressSpinBox, 0, 2);

    // Plasticity line
    m_piSpinBox = new QDoubleSpinBox; 
    m_piSpinBox->setDecimals(0);
    m_piSpinBox->setRange( 0, 200);
    m_piSpinBox->setSingleStep(1);
    
    layout->addWidget(new QLabel(tr("Plasticity Index:")), 1, 0);
    layout->addWidget(m_piSpinBox, 1, 2);

    // OCR line
    m_ocrSpinBox = new QDoubleSpinBox;
    m_ocrSpinBox->setDecimals(2);
    m_ocrSpinBox->setRange( 1, 20);
    m_ocrSpinBox->setSingleStep(1);

    layout->addWidget(new QLabel(tr("Over-consolidation ratio:")), 2, 0);
    layout->addWidget(m_ocrSpinBox, 2, 2);

    // Frequency line
    m_freqSpinBox = new QDoubleSpinBox;
    m_freqSpinBox->setDecimals(1);
    m_freqSpinBox->setRange( 0.1, 100);
    m_freqSpinBox->setSingleStep(1);
    m_freqSpinBox->setSuffix(" Hz");

    layout->addWidget(new QLabel(tr("Excitation frequency:")), 3, 0);
    layout->addWidget(m_freqSpinBox, 3, 2);
    
    // Cycles line
    m_nCyclesSpinBox = new QSpinBox;
    m_nCyclesSpinBox->setRange( 1, 100);

    layout->addWidget(new QLabel(tr("Number of cycles:")), 4, 0);
    layout->addWidget(m_nCyclesSpinBox, 4, 2);
    layout->setColumnStretch( 0, 1);


    // Create the group box
    m_soilPropsGroupBox = new QGroupBox(tr("Darendeli and Stokoe Model Parameters"));
    m_soilPropsGroupBox->setLayout(layout);
    m_soilPropsGroupBox->setEnabled(false);
}

void SoilTypePage::createNlPropTableBox()
{
    m_nlPropTableBox = new TableGroupBox(
            new NonlinearPropertyTableModel( 0 ), tr("Nonlinear Property"));
    connect( m_nlPropTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
    m_nlPropTableBox->setEnabled(false);
}

void SoilTypePage::setIsVaried(bool isVaried)
{
    if (isVaried) {
        m_layersTableBox->table()->showColumn(6);
    } else {
        m_layersTableBox->table()->hideColumn(6);
    }
    
    m_variationGroupBox->setVisible(isVaried);
    m_varyBedrockDampingCheckBox->setVisible(isVaried);
}

void SoilTypePage::setReadOnly(bool b)
{
    m_layersTableBox->setReadOnly(b);

    m_bedrockUntWtSpinBox->setReadOnly(b);
    m_bedrockDampingSpinBox->setReadOnly(b);
    m_varyBedrockDampingCheckBox->setDisabled(b);

    m_dynPropModelComboBox->setDisabled(b);

    m_shearModStdevLineEdit->setReadOnly(b);
    m_shearModMaxSpinBox->setReadOnly(b);
    m_shearModMinSpinBox->setReadOnly(b);

    m_dampingStdevLineEdit->setReadOnly(b);
    m_dampingMinSpinBox->setReadOnly(b);
    m_dampingMaxSpinBox->setReadOnly(b);

    m_correlSpinBox->setReadOnly(b);

    m_stressSpinBox->setReadOnly(b);
    m_piSpinBox->setReadOnly(b);
    m_ocrSpinBox->setReadOnly(b);
    m_freqSpinBox->setReadOnly(b);
    m_nCyclesSpinBox->setReadOnly(b);

    m_nlPropTableBox->setReadOnly(b);
}

void SoilTypePage::updateUnits()
{
    m_bedrockUntWtSpinBox->setSuffix( " " + Units::instance()->untWt() );
}

void SoilTypePage::unselectSoil()
{
    m_nlPropTableBox->setEnabled(false);
    m_soilPropsGroupBox->setEnabled(false);

    m_selectedSoilType = 0;
    m_selectedNonlinearProperty = 0;
}

void SoilTypePage::setSelectedSoil(const QModelIndex & current, const QModelIndex & /*previous*/)
{
    // if ( current.row() >= 0 ) {
    if ( current.isValid() ) {
        // Save the index 
        m_selectedIndex = current;
        // The selected soil type
        m_selectedSoilType = m_model->siteProfile()->soilTypes().at(m_selectedIndex.row());
    } else {
        m_selectedSoilType = 0;
    }

    refreshSoilDetails();
}

void SoilTypePage::refreshSoilDetails()
{
    // Disable the nonlinear property group box
    m_nlPropTableBox->setEnabled(false);
    m_soilPropsGroupBox->setEnabled(false);
    
    if ( !m_selectedSoilType ) {
        return;
    }
     
    if ( m_selectedIndex.column() == 3 || m_selectedIndex.column() == 4 ) {

        // Set the model for the dynamic properties
        if ( m_selectedIndex.column() == 3 )
            m_selectedNonlinearProperty = m_selectedSoilType->normShearMod();
        else if ( m_selectedIndex.column() == 4 )
            m_selectedNonlinearProperty = m_selectedSoilType->damping();

        // If the source is Darendeli and the curves are empty, re-compute the curves
        if ( m_selectedNonlinearProperty->source() == NonlinearProperty::Computed
                && m_selectedNonlinearProperty->avg().isEmpty() )
            m_selectedSoilType->computeDarendeliCurves();


        // Update the dynamic property group box
        static_cast<NonlinearPropertyTableModel*>(m_nlPropTableBox->table()->model())->setNonlinearProperty(m_selectedNonlinearProperty);

        // Enable the nonlinear property group box
        m_nlPropTableBox->setEnabled(true);
        
        if (m_selectedNonlinearProperty->source() == NonlinearProperty::Temporary)
            m_nlPropTableBox->setReadOnly(false);
        else
            m_nlPropTableBox->setReadOnly(true);
    }

    // If the Stokoe-Darendeli option is selected for either model enable the
    // soil properties box.
    if ( m_selectedSoilType->normShearMod()->source() == NonlinearProperty::Computed 
            || m_selectedSoilType->damping()->source() == NonlinearProperty::Computed ) 
    {
        m_soilPropsGroupBox->setEnabled(true);
        // Clear the connections
        disconnect(m_stressSpinBox, 0, 0, 0);
        disconnect(m_piSpinBox, 0, 0, 0);
        disconnect(m_ocrSpinBox, 0, 0, 0);
        disconnect(m_freqSpinBox, 0, 0, 0);
        disconnect(m_nCyclesSpinBox, 0, 0, 0);
        // Load the soil properties
        loadSoilProperties();
        
        // Form the connections
        connect(m_stressSpinBox, SIGNAL(valueChanged(double)),  
                m_selectedSoilType, SLOT(setMeanStress(double)));
        connect(m_piSpinBox, SIGNAL(valueChanged(double)),
                m_selectedSoilType, SLOT(setPI(double)));
        connect(m_ocrSpinBox, SIGNAL(valueChanged(double)),
                m_selectedSoilType, SLOT(setOCR(double)));
        connect(m_freqSpinBox, SIGNAL(valueChanged(double)),
                m_selectedSoilType, SLOT(setFreq(double)));
        connect(m_nCyclesSpinBox, SIGNAL(valueChanged(int)),
                m_selectedSoilType, SLOT(setNCycles(int)));
    } 
    else 
    {
        // Clear the line edits
	    m_stressSpinBox->clear();
	    m_piSpinBox->clear();
	    m_ocrSpinBox->clear();
	    m_freqSpinBox->clear();
	    m_nCyclesSpinBox->clear();
    }
}

void SoilTypePage::load()
{
    // Bedrock parameters
    m_bedrockUntWtSpinBox->setValue(m_model->siteProfile()->bedrock()->untWt());
    m_bedrockDampingSpinBox->setValue(m_model->siteProfile()->bedrock()->avgDamping());
    m_varyBedrockDampingCheckBox->setChecked(
            m_model->siteProfile()->nonLinearPropertyVariation()->bedrockIsEnabled());

    // Variation parameters
    const NonlinearPropertyVariation * npv = m_model->siteProfile()->nonLinearPropertyVariation();
    m_dynPropModelComboBox->setCurrentIndex(npv->model());
    m_shearModMinSpinBox->setValue(npv->shearModMin());
    m_shearModMaxSpinBox->setValue(npv->shearModMax());
    m_dampingMinSpinBox->setValue(npv->dampingMin());
    m_dampingMaxSpinBox->setValue(npv->dampingMax());
    m_correlSpinBox->setValue(npv->correl());
    
    loadStdevModels();
}

void SoilTypePage::loadStdevModels()
{
    m_shearModStdevLineEdit->setText(m_model->siteProfile()->nonLinearPropertyVariation()->shearModStdev());
    m_dampingStdevLineEdit->setText(m_model->siteProfile()->nonLinearPropertyVariation()->dampingStdev());

    // Disable the line edits for Darendeli and Stokoe
    if ( m_dynPropModelComboBox->currentIndex() == NonlinearPropertyVariation::Darendeli ) {
        m_shearModStdevLineEdit->setEnabled(false);
        m_dampingStdevLineEdit->setEnabled(false);
    } else {
        m_shearModStdevLineEdit->setEnabled(true);
        m_dampingStdevLineEdit->setEnabled(true);
    }
} 

void SoilTypePage::loadSoilProperties()
{
    if (!m_selectedSoilType)
        return;
    // Set the line edits
    m_stressSpinBox->setValue(m_selectedSoilType->meanStress());
    m_piSpinBox->setValue(m_selectedSoilType->PI());
    m_ocrSpinBox->setValue(m_selectedSoilType->OCR());
    m_freqSpinBox->setValue(m_selectedSoilType->freq());
    m_nCyclesSpinBox->setValue(m_selectedSoilType->nCycles());
}
