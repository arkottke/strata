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

#include "GeneralPage.h"
#include "SiteResponseOutput.h"
#include "Units.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>
#include <QDoubleValidator>

GeneralPage::GeneralPage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f )
	: QWidget(parent, f), m_model(model)
{
	// Setup the Group Boxes
	createProjectGroupBox();
	createAnalysisGroupBox();
	createVariationGroupBox();
    createDiscretizationGroupBox();
    createEquivLinearGroupBox();

	// Layout of the widget
	QGridLayout * layout = new QGridLayout;
	layout->addWidget( m_projectGroupBox, 0, 0, 4, 1 );
    layout->addWidget( m_analysisGroupBox, 0, 1);
	layout->addWidget( m_siteVarGroupBox, 1, 1 );
	layout->addWidget( m_equivLinearGroupBox, 2, 1 );
    layout->addWidget( m_discretizationGroupBox, 3, 1);

    // Add a row of stretching
    layout->setRowStretch( 4, 1);
    layout->setColumnStretch( 0, 1);

	setLayout(layout);
    
    // Connections
    connect(m_siteIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setIsSiteVaried(int)));
    connect(m_methodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMethod(int))); 
    
    // Load the values from the model
    load();
}

void GeneralPage::setModel( SiteResponseModel * model )
{
    m_model = model;
}
		
void GeneralPage::createProjectGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(2,1);
    layout->setRowStretch(2,1);
	// Title
	m_titleLineEdit = new QLineEdit;
    connect( m_titleLineEdit, SIGNAL(textChanged(QString)), m_model->output(), SLOT(setTitle(QString)));
    
    layout->addWidget( new QLabel(tr("Title:")), 0, 0);
	layout->addWidget( m_titleLineEdit, 0, 1, 1, 2);

	// Notes
	m_notesTextEdit = new QTextEdit;
    m_notesTextEdit->setDocument(m_model->notes());

	layout->addWidget( new QLabel(tr("Notes:")), 1, 0);
	layout->addWidget( m_notesTextEdit, 1, 1, 2, 2 );

	// File name prefix
	m_prefixLineEdit = new QLineEdit;
    connect( m_prefixLineEdit, SIGNAL(textChanged(QString)), 
            m_model->output(), SLOT(setFilePrefix(QString)));
	
    layout->addWidget( new QLabel(tr("Filename prefix:")), 3, 0);
	layout->addWidget( m_prefixLineEdit, 3, 1 );

    // Units
	m_unitsComboBox = new QComboBox;
	m_unitsComboBox->addItems(Units::systemList());
    connect( m_unitsComboBox, SIGNAL(currentIndexChanged(int)), Units::instance(), SLOT(setSystem(int)));

	layout->addWidget( new QLabel(tr("Units:")), 4, 0);
	layout->addWidget( m_unitsComboBox, 4, 1 );
	layout->addWidget( new QLabel(tr("Note: only changes labels and gravity, no unit conversion.")), 4, 2);

    // Save motion data
    m_saveMotionDataCheckBox = new QCheckBox(tr("Save motion data within the input file."));
    connect(m_saveMotionDataCheckBox, SIGNAL(toggled(bool)), m_model, SLOT(setSaveMotionData(bool)));
    layout->addWidget( m_saveMotionDataCheckBox, 5, 1 );

    // Create the group box and add the layout
	m_projectGroupBox = new QGroupBox(tr("Project"));
	m_projectGroupBox->setLayout(layout);
}

void GeneralPage::createAnalysisGroupBox()
{
	QGridLayout * layout = new QGridLayout;
	
    // Method
	m_methodComboBox = new QComboBox;
	m_methodComboBox->addItems(SiteResponseModel::methodList());
    connect( m_methodComboBox, SIGNAL(currentIndexChanged(int)), m_model, SLOT(setMethod(int)));

	layout->addWidget( new QLabel(tr("Calculation Method:")), 0, 0);
	layout->addWidget( m_methodComboBox, 0, 1 );
    
    // Site varied
	m_siteIsVariedCheckBox = new QCheckBox(tr("Vary the properties"));
    connect(m_siteIsVariedCheckBox, SIGNAL(toggled(bool)), m_model->siteProfile(), SLOT(setIsSiteVaried(bool)));

    layout->addWidget( m_siteIsVariedCheckBox, 1, 0, 1, 2 );

    // Create the group box and add the layout
	m_analysisGroupBox = new QGroupBox(tr("Type of Analysis"));
	m_analysisGroupBox->setLayout(layout);
}

void GeneralPage::createVariationGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(1,1);
    layout->setColumnMinimumWidth(0, 20);
	
    // Count
	m_countSpinBox = new QSpinBox;
	m_countSpinBox->setRange( 1, 1000);
    connect( m_countSpinBox, SIGNAL(valueChanged(int)), m_model->siteProfile(), SLOT(setProfileCount(int)));

	layout->addWidget( new QLabel(tr("Number of realizations:")), 0, 0, 1, 2);
	layout->addWidget( m_countSpinBox, 0, 2 );
	
	// Checkboxes for variation
	m_soilIsVariedCheckBox = new QCheckBox(tr("Vary the nonlinear soil properties"));
    connect( m_soilIsVariedCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setEnabled(bool)));

    layout->addWidget(m_soilIsVariedCheckBox, 1, 0, 1, 3);
    layout->addWidget(new QLabel(tr("-- shear modulus reduction and damping curves")), 2, 1, 1, 2);
	
    m_velIsVariedCheckBox = new QCheckBox(tr("Vary the site profile (velocity and/or layer thickness)"));	
    connect( m_soilIsVariedCheckBox, SIGNAL(toggled(bool)), 
            m_model->siteProfile()->nonLinearPropertyVariation(), SLOT(setEnabled(bool)));

	layout->addWidget(m_velIsVariedCheckBox, 3, 0, 1, 3);
    layout->addWidget(new QLabel(tr(
                    "-- shear wave velocity\n"
                    "-- layer thickness\n"
                    "-- depth to bedrock"
                    )), 4, 1, 1, 2);

    // Create the group box and add the layout
	m_siteVarGroupBox = new QGroupBox(tr("Site Property Variation"));
	m_siteVarGroupBox->setEnabled(false);
	m_siteVarGroupBox->setLayout(layout);

    // Connections 
    connect( m_soilIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setIsSoilVaried(int))); 
    connect( m_velIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setIsVelocityVaried(int))); 
}

void GeneralPage::createEquivLinearGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

	// Effective Strain Ratio Row
	m_strainRatioSpinBox = new QDoubleSpinBox;
    m_strainRatioSpinBox->setRange( 0.45, 0.80);
    m_strainRatioSpinBox->setDecimals(2);
    connect( m_strainRatioSpinBox, SIGNAL(valueChanged(double)), 
            m_model->calculator(), SLOT(setStrainRatio(double)));

    layout->addWidget( new QLabel(tr("Effective strain ratio:")), 0, 0);
	layout->addWidget( m_strainRatioSpinBox, 0, 2 );

	// Error tolerance row
	m_errorTolSpinBox = new QDoubleSpinBox;
    m_errorTolSpinBox->setRange( 0.5, 10.0);
    m_errorTolSpinBox->setDecimals(1);
    m_errorTolSpinBox->setSuffix(" %");
    connect( m_errorTolSpinBox, SIGNAL(valueChanged(double)), 
            m_model->calculator(), SLOT(setErrorTolerance(double)));

	layout->addWidget( new QLabel(tr("Error tolerance:")), 1, 0);
	layout->addWidget( m_errorTolSpinBox, 1, 2 );

	// Max Iterations row
	m_iterationsSpinBox = new QSpinBox;
	m_iterationsSpinBox->setMinimum(2);
	m_iterationsSpinBox->setMaximum(30);
    connect( m_iterationsSpinBox, SIGNAL(valueChanged(int)), 
            m_model->calculator(), SLOT(setMaxIterations(int)));

    layout->addWidget( new QLabel(tr("Maximum number of iterations:")), 2, 0);
	layout->addWidget( m_iterationsSpinBox, 2, 2 );

	// Add the layout of the widget
	m_equivLinearGroupBox = new QGroupBox(tr("Equivalent Linear Parameters"));
	m_equivLinearGroupBox->setLayout(layout);
}

void GeneralPage::createDiscretizationGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Maximum frequency
	m_maxFreqSpinBox = new QDoubleSpinBox;
    m_maxFreqSpinBox->setRange( 15, 100);
    m_maxFreqSpinBox->setDecimals(0);
    m_maxFreqSpinBox->setSuffix(" Hz");
    connect( m_maxFreqSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile(), SLOT(setMaxFreq(double)));

    layout->addWidget( new QLabel(tr("Maximum frequency:")), 0, 0);
    layout->addWidget( m_maxFreqSpinBox, 0, 1 );

    // Wave length fraction
	m_waveFractionSpinBox = new QDoubleSpinBox;
    m_waveFractionSpinBox->setRange( 0.10, 0.35);
    m_waveFractionSpinBox->setDecimals(2);
    connect( m_waveFractionSpinBox, SIGNAL(valueChanged(double)), 
            m_model->siteProfile(), SLOT(setWaveFraction(double)));

    layout->addWidget( new QLabel(tr("Wavelength fraction:")), 1, 0);
    layout->addWidget( m_waveFractionSpinBox, 1, 1 );

    // Group box 
    m_discretizationGroupBox = new QGroupBox(tr("Layer Discretization"));
    m_discretizationGroupBox->setLayout(layout);
}

void GeneralPage::setIsSiteVaried(int isVaried)
{
    // Enable the group box
    m_siteVarGroupBox->setEnabled(isVaried);

    // Emit signals that the soil and velocity variation might have changed
    if ( m_siteIsVariedCheckBox->isChecked() && m_soilIsVariedCheckBox->isChecked() )
        emit isSoilVariedChanged(true);
    else
        emit isSoilVariedChanged(false);
    
    if ( m_siteIsVariedCheckBox->isChecked() && m_velIsVariedCheckBox->isChecked() )
        emit isVelocityVariedChanged(true);
    else
        emit isVelocityVariedChanged(false);
}

void GeneralPage::setIsSoilVaried(int isVaried)
{
    m_model->siteProfile()->nonLinearPropertyVariation()->setEnabled(isVaried);

    emit isSoilVariedChanged(isVaried);
}

void GeneralPage::setIsVelocityVaried(int isVaried)
{
    m_model->siteProfile()->profileVariation()->setEnabled(isVaried);

    emit isVelocityVariedChanged(isVaried);
}

void GeneralPage::setMethod(int method)
{
    m_model->setMethod((SiteResponseModel::Method)method);

    emit methodChanged(method);
}

void GeneralPage::load()
{
    m_titleLineEdit->setText(m_model->output()->title());
    // FIXME m_notesTextEdit->setText(m_model->notes());
    m_prefixLineEdit->setText(m_model->output()->filePrefix());
    m_unitsComboBox->setCurrentIndex((int)Units::instance()->system());
    m_saveMotionDataCheckBox->setChecked(m_model->saveMotionData());

    // Type of analysis
    m_methodComboBox->setCurrentIndex((int)m_model->method());
    m_siteIsVariedCheckBox->setChecked(m_model->siteProfile()->isSiteVaried());

    // Variation
    m_countSpinBox->setValue(m_model->siteProfile()->profileCount());
    m_soilIsVariedCheckBox->setChecked(m_model->siteProfile()->nonLinearPropertyVariation()->enabled());
    m_velIsVariedCheckBox->setChecked(m_model->siteProfile()->profileVariation()->enabled());
    
    // Layer discretization
    m_maxFreqSpinBox->setValue(m_model->siteProfile()->maxFreq());
    m_waveFractionSpinBox->setValue(m_model->siteProfile()->waveFraction());

    // Equivalent linear properties 
    m_strainRatioSpinBox->setValue(m_model->calculator()->strainRatio());
    m_errorTolSpinBox->setValue(m_model->calculator()->errorTolerance());
    m_iterationsSpinBox->setValue(m_model->calculator()->maxIterations());
}

void GeneralPage::setReadOnly(bool b)
{
    m_titleLineEdit->setReadOnly(b);
    m_notesTextEdit->setReadOnly(b);
    m_prefixLineEdit->setReadOnly(b);
    m_unitsComboBox->setDisabled(b);
    m_saveMotionDataCheckBox->setDisabled(b);

     m_methodComboBox->setDisabled(b);
     m_siteIsVariedCheckBox->setDisabled(b);

     m_countSpinBox->setReadOnly(b);
     m_soilIsVariedCheckBox->setDisabled(b);
     m_velIsVariedCheckBox->setDisabled(b);

		
    m_strainRatioSpinBox->setReadOnly(b);
    m_errorTolSpinBox->setReadOnly(b);
    m_iterationsSpinBox->setReadOnly(b);
        
    m_maxFreqSpinBox->setReadOnly(b);
    m_waveFractionSpinBox->setReadOnly(b);


	//m_projectGroupBox->setDisabled(b);
	//m_analysisGroupBox->setDisabled(b);
	//m_equivLinearGroupBox->setDisabled(b);
	//m_discretizationGroupBox->setDisabled(b);

    //m_siteVarGroupBox->setDisabled(!(b && m_siteIsVariedCheckBox->isChecked()));
}
