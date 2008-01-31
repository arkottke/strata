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
    connect( m_titleLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));
	
    layout->addWidget( new QLabel(tr("Title:")), 0, 0);
	layout->addWidget( m_titleLineEdit, 0, 1, 1, 2);

	// Notes
	m_notesTextEdit = new QTextEdit;
    connect( m_notesTextEdit, SIGNAL(textChanged()), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(tr("Notes:")), 1, 0);
	layout->addWidget( m_notesTextEdit, 1, 1, 2, 2 );

	// File name prefix
	m_prefixLineEdit = new QLineEdit;
    connect( m_prefixLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));
	
    layout->addWidget( new QLabel(tr("Filename prefix:")), 3, 0);
	layout->addWidget( m_prefixLineEdit, 3, 1 );

    // Units
	m_unitsComboBox = new QComboBox;
	m_unitsComboBox->addItems(Units::systemList());
    connect( m_unitsComboBox, SIGNAL(currentIndexChanged(int)), m_model->units(), SLOT(setSystem(int)));
    connect( m_unitsComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(tr("Units:")), 4, 0);
	layout->addWidget( m_unitsComboBox, 4, 1 );

    // Save motion data
    m_saveMotionDataCheckBox = new QCheckBox(tr("Save motion data within the input file."));
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
    connect( m_methodComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(tr("Calculation Method:")), 0, 0);
	layout->addWidget( m_methodComboBox, 0, 1 );
    
    // Site varied
	m_siteIsVariedCheckBox = new QCheckBox(tr("Vary the properties"));
    connect( m_siteIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(hasChanged()));

    layout->addWidget( m_siteIsVariedCheckBox, 1, 0, 1, 2 );

    
    // Create the group box and add the layout
	m_analysisGroupBox = new QGroupBox(tr("Type of Analysis"));
	m_analysisGroupBox->setLayout(layout);
}

void GeneralPage::createVariationGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);
    layout->setColumnMinimumWidth(0, 20);
	
    // Count
	m_countSpinBox = new QSpinBox;
	m_countSpinBox->setValue(20);
	m_countSpinBox->setMinimum(1);
	m_countSpinBox->setMaximum(5000);
    connect( m_countSpinBox, SIGNAL(valueChanged(int)), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(tr("Number of realizations:")), 0, 0, 1, 2);
	layout->addWidget( m_countSpinBox, 0, 2 );
	
	// Checkboxes for variation
	m_soilIsVariedCheckBox = new QCheckBox(tr("Vary the non-linear soil properties"));
    connect( m_soilIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(hasChanged()));

    layout->addWidget(m_soilIsVariedCheckBox, 1, 0, 1, 3);
    layout->addWidget(new QLabel(tr("-- shear modulus reduction and damping curves")), 2, 1);
	
    m_velIsVariedCheckBox = new QCheckBox(tr("Vary the site profile (velocity and/or layer thickness)"));	
    connect( m_velIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SIGNAL(hasChanged()));

	layout->addWidget(m_velIsVariedCheckBox, 3, 0, 1, 3);
    layout->addWidget(new QLabel(tr(
                    "-- shear wave velocity\n"
                    "-- layer thickness\n"
                    "-- depth to bedrock"
                    )), 4, 1);

    // Create the group box and add the layout
	m_siteVarGroupBox = new QGroupBox(tr("Site Property Variation"));
	m_siteVarGroupBox->setEnabled(false);
	m_siteVarGroupBox->setLayout(layout);

    // Connections 
    connect( m_soilIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setIsSoilVaried(int))); 
    connect( m_velIsVariedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(setIsVelocityVaried(int))); 
}

void GeneralPage::createDiscretizationGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Maximum frequency
    m_maxFreqLineEdit = new QLineEdit;
    m_maxFreqLineEdit->setValidator( new QDoubleValidator(m_maxFreqLineEdit));
    connect( m_maxFreqLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Maximum frequency:")), 0, 0);
    layout->addWidget( m_maxFreqLineEdit, 0, 1 );

    // Wave length fraction
    m_waveFractionLineEdit = new QLineEdit;
    m_waveFractionLineEdit->setValidator( new QDoubleValidator(m_waveFractionLineEdit));
    connect( m_waveFractionLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Wavelength fraction:")), 1, 0);
    layout->addWidget( m_waveFractionLineEdit, 1, 1 );

    // Group box 
    m_discretizationGroupBox = new QGroupBox(tr("Layer Discretization"));
    m_discretizationGroupBox->setLayout(layout);
}

void GeneralPage::createEquivLinearGroupBox()
{
	QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

	// Effective Strain Ratio Row
	m_strainRatioLineEdit = new QLineEdit;
	m_strainRatioLineEdit->setValidator( new QDoubleValidator(m_strainRatioLineEdit) );
    connect( m_strainRatioLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Effective strain ratio:")), 0, 0);
	layout->addWidget( m_strainRatioLineEdit, 0, 2 );

	// Error tolerance row
	m_errorTolLineEdit = new QLineEdit;
	m_errorTolLineEdit->setValidator( new QDoubleValidator(m_errorTolLineEdit) );
    connect( m_errorTolLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(hasChanged()));

	layout->addWidget( new QLabel(tr("Error tolerance (%):")), 1, 0);
	layout->addWidget( m_errorTolLineEdit, 1, 2 );

	// Max Iterations row
	m_iterationsSpinBox = new QSpinBox;
	m_iterationsSpinBox->setValue(8);
	m_iterationsSpinBox->setMinimum(2);
	m_iterationsSpinBox->setMaximum(30);
    connect( m_iterationsSpinBox, SIGNAL(valueChanged(QString)), this, SIGNAL(hasChanged()));

    layout->addWidget( new QLabel(tr("Maximum number of iterations:")), 2, 0);
	layout->addWidget( m_iterationsSpinBox, 2, 2 );

	// Add the layout of the widget
	m_equivLinearGroupBox = new QGroupBox(tr("Equivalent Linear Parameters"));
	m_equivLinearGroupBox->setLayout(layout);
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
    m_model->siteProfile().nonLinearPropertyVariation().setEnabled(isVaried);

    emit isSoilVariedChanged(isVaried);
}

void GeneralPage::setIsVelocityVaried(int isVaried)
{
    m_model->siteProfile().profileVariation().setEnabled(isVaried);

    emit isVelocityVariedChanged(isVaried);
}

void GeneralPage::setMethod(int method)
{
    m_model->setMethod((SiteResponseModel::Method)method);

    emit methodChanged(method);
}

void GeneralPage::load()
{
    m_titleLineEdit->setText(m_model->title());
    m_notesTextEdit->setText(m_model->notes());
    m_prefixLineEdit->setText(m_model->output()->filePrefix());
    m_unitsComboBox->setCurrentIndex((int)m_model->units()->system());
    m_saveMotionDataCheckBox->setChecked(m_model->saveMotionData());

    // Type of analysis
    m_methodComboBox->setCurrentIndex((int)m_model->method());
    m_siteIsVariedCheckBox->setChecked(m_model->siteProfile().isSiteVaried());

    // Variation
    m_countSpinBox->setValue(m_model->siteProfile().profileCount());
    m_soilIsVariedCheckBox->setChecked(m_model->siteProfile().nonLinearPropertyVariation().enabled());
    m_velIsVariedCheckBox->setChecked(m_model->siteProfile().profileVariation().enabled());
    
    // Layer discretization
    m_maxFreqLineEdit->setText(QString::number(m_model->siteProfile().maxFreq()));
    m_waveFractionLineEdit->setText(QString::number(m_model->siteProfile().waveFraction()));

    // Equivalent linear properties 
    m_strainRatioLineEdit->setText(QString::number(m_model->calculator().strainRatio()));
    m_errorTolLineEdit->setText(QString::number(m_model->calculator().errorTolerance()));
    m_iterationsSpinBox->setValue(m_model->calculator().maxIterations());
}

void GeneralPage::save()
{
    m_model->setTitle(m_titleLineEdit->text());
    m_model->setNotes(m_notesTextEdit->toPlainText());
    m_model->output()->setFilePrefix(m_prefixLineEdit->text());
    m_model->units()->setSystem(m_unitsComboBox->currentIndex());
    m_model->setSaveMotionData(m_saveMotionDataCheckBox->isChecked());

    // Type of analysis
    m_model->setMethod((SiteResponseModel::Method)m_methodComboBox->currentIndex());
    m_model->siteProfile().setIsSiteVaried(m_siteIsVariedCheckBox->isChecked());

    m_model->siteProfile().setProfileCount(m_countSpinBox->value());
    m_model->siteProfile().nonLinearPropertyVariation().setEnabled(m_soilIsVariedCheckBox->isChecked());
    m_model->siteProfile().profileVariation().setEnabled(m_velIsVariedCheckBox->isChecked());
    
    // Layer discretization
    m_model->siteProfile().setMaxFreq( m_maxFreqLineEdit->text().toDouble());
    m_model->siteProfile().setWaveFraction( m_waveFractionLineEdit->text().toDouble());

    // Equivalent linear properties 
    m_model->calculator().setStrainRatio(m_strainRatioLineEdit->text().toDouble());
    m_model->calculator().setErrorTolerance(m_errorTolLineEdit->text().toDouble());
    m_model->calculator().setMaxIterations(m_iterationsSpinBox->value());
}
