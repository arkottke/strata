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

#include "OutputPage.h"
#include "ResponseLocationTableModel.h"
#include "RatioLocationTableModel.h"
#include "StringListDelegate.h"
#include "DepthComboBoxDelegate.h"
#include "SoilTypeOutputTableModel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QDebug>

OutputPage::OutputPage( SiteResponseModel * model, QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent,f), m_model(model)
{
    // Left Column
    QVBoxLayout * leftColumn = new QVBoxLayout;

    // Response output
    m_responseTableBox = new TableGroupBox( new ResponseLocationTableModel(m_model->output()), tr("Response Location Output"));
    m_responseTableBox->table()->setItemDelegateForColumn(0, new DepthComboBoxDelegate);
    m_responseTableBox->table()->setItemDelegateForColumn(1, new StringListDelegate);

    connect(m_responseTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
    connect(m_responseTableBox, SIGNAL(dataChanged()), SLOT(updateWidget()));

    leftColumn->addWidget(m_responseTableBox);

    // Ratio output
    m_ratioTableBox = new TableGroupBox( new RatioLocationTableModel(m_model->output()), tr("Ratio Output"));

    m_ratioTableBox->table()->setItemDelegateForColumn(0, new DepthComboBoxDelegate);
    m_ratioTableBox->table()->setItemDelegateForColumn(1, new StringListDelegate);
    m_ratioTableBox->table()->setItemDelegateForColumn(2, new DepthComboBoxDelegate);
    m_ratioTableBox->table()->setItemDelegateForColumn(3, new StringListDelegate);
    
    connect(m_ratioTableBox, SIGNAL(dataChanged()), m_model, SLOT(setModified()));
    connect(m_ratioTableBox, SIGNAL(dataChanged()), SLOT(updateWidget()));
    
    leftColumn->addWidget(m_ratioTableBox);

    // Right Column
    QVBoxLayout * rightColumn = new QVBoxLayout;

    // Profiles
    createProfilesGroupBox();
    rightColumn->addWidget(m_profilesGroupBox);
    
    // Soil Type 
    createSoilTypesGroupBox();
    rightColumn->addWidget(m_soilTypesGroupBox);

    // Response spectrum group box
    createRespSpecGroupBox();
    rightColumn->addWidget(m_respSpecGroupBox);
    
    // Frequency group box
    createFreqGroupBox();
    rightColumn->addWidget(m_freqGroupBox);

    // Outpug group box
    createLogGroupBox();
    rightColumn->addWidget(m_logGroupBox);
    
    rightColumn->addStretch(1);

    // Overall layout
    QHBoxLayout * layout = new QHBoxLayout;

    layout->addItem( leftColumn);
    layout->addItem( rightColumn);

    layout->setStretchFactor( leftColumn, 1);

    // Set general layout
    setLayout(layout);

    // Load the values
    load();
}

void OutputPage::refresh()
{
    // Refresh the model data
    qobject_cast<SoilTypeOutputTableModel*>(m_soilTypeTableView->model())->resetModel();

    m_soilTypeTableView->resizeColumnsToContents();
    m_soilTypeTableView->resizeRowsToContents();
}

void OutputPage::setMethod(int type)
{
    if (type == SiteResponseModel::RecordedMotions) {
        for (int i = 4; i < m_responseTableBox->model()->columnCount(); ++i)
            m_responseTableBox->table()->showColumn(i);
    } else {
        for (int i = 4; i < m_responseTableBox->model()->columnCount(); ++i)
            m_responseTableBox->table()->hideColumn(i);
    }
}

void OutputPage::setReadOnly(bool b)
{
	m_responseTableBox->setReadOnly(b);
	m_ratioTableBox->setReadOnly(b);

	qobject_cast<MyAbstractTableModel*>(m_soilTypeTableView->model())->setReadOnly(b);

	m_initialShearVelCheckBox->setDisabled(b);
	m_finalShearVelCheckBox->setDisabled(b);
	m_finalShearModCheckBox->setDisabled(b);
	m_finalDampingCheckBox->setDisabled(b);
	m_vTotalStressCheckBox->setDisabled(b);
	m_maxShearStressCheckBox->setDisabled(b);
	m_stressReducCoeffCheckBox->setDisabled(b);
	m_maxShearStrainCheckBox->setDisabled(b);
	m_maxAccelCheckBox->setDisabled(b);
	m_maxVelCheckBox->setDisabled(b);
	m_stressRatioCheckBox->setDisabled(b);
	m_maxErrorCheckBox->setDisabled(b);

	m_dampingSpinBox->setReadOnly(b);
	m_periodMinSpinBox->setReadOnly(b);
	m_periodMaxSpinBox->setReadOnly(b);
	m_periodNptsSpinBox->setReadOnly(b);
	m_periodSpacingComboBox->setDisabled(b);
        
	m_freqMinSpinBox->setReadOnly(b);
	m_freqMaxSpinBox->setReadOnly(b);
	m_freqNptsSpinBox->setReadOnly(b);
	m_freqSpacingComboBox->setDisabled(b);

	m_logLevelComboBox->setDisabled(b);
}

void OutputPage::createSoilTypesGroupBox()
{
    QVBoxLayout * layout = new QVBoxLayout;

    MyAbstractTableModel * model = new SoilTypeOutputTableModel(m_model->siteProfile()->soilTypes());
    connect( m_model->siteProfile(), SIGNAL(soilTypesChanged()), model, SLOT(resetModel()));
    // Create the table view
    m_soilTypeTableView = new QTableView;
    m_soilTypeTableView->setModel(model);
    connect( m_soilTypeTableView->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_model, SLOT(setModified()));

    layout->addWidget(m_soilTypeTableView);
    
    // Create the group box
    m_soilTypesGroupBox = new QGroupBox(tr("Nonlinear Curves"));

    m_soilTypesGroupBox->setLayout(layout);
}

void OutputPage::createProfilesGroupBox()
{
    QVBoxLayout * layout = new QVBoxLayout;
    
    m_maxAccelCheckBox = new QCheckBox(tr("Maximum acceleration"));
    connect( m_maxAccelCheckBox, SIGNAL(toggled(bool)), 
	    m_model->output()->maxAccel(), SLOT(setEnabled(bool)));
        
    layout->addWidget(m_maxAccelCheckBox);
    
    m_maxVelCheckBox = new QCheckBox(tr("Maximum velocity"));
    connect( m_maxVelCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->maxVel(), SLOT(setEnabled(bool)));
    layout->addWidget(m_maxVelCheckBox);
    
    m_stressReducCoeffCheckBox = new QCheckBox(tr("Stress reduction coefficient (r_d)"));
    connect( m_stressReducCoeffCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->stressReducCoeff(), SLOT(setEnabled(bool)));
    layout->addWidget(m_stressReducCoeffCheckBox);
    
    m_maxShearStrainCheckBox = new QCheckBox(QString(tr("Maximum shear strain, %1_max")).arg(QChar(0x03B3)));
    connect( m_maxShearStrainCheckBox, SIGNAL(toggled(bool)), 
	    m_model->output()->maxShearStrain(), SLOT(setEnabled(bool)));
    layout->addWidget(m_maxShearStrainCheckBox);
    
    m_maxShearStressCheckBox = new QCheckBox(QString(tr("Maximum shear stress, %1_max")).arg(QChar(0x03C4)));
    connect( m_maxShearStressCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->maxShearStress(), SLOT(setEnabled(bool)));
    layout->addWidget(m_maxShearStressCheckBox);

    m_stressRatioCheckBox = new QCheckBox(QString(tr("Stress Ratio, %1_max / %2_v")).arg(QChar(0x03C4)).arg(QChar(0x03C3)));
    connect( m_stressRatioCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->stressRatio(), SLOT(setEnabled(bool)));
    layout->addWidget(m_stressRatioCheckBox);

    m_vTotalStressCheckBox = new QCheckBox(QString(tr("Vertical total stress, %1_v")).arg(QChar(0x03C3)));
    connect( m_vTotalStressCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->vTotalStress(), SLOT(setEnabled(bool)));
    layout->addWidget(m_vTotalStressCheckBox);

    m_initialShearVelCheckBox = new QCheckBox(tr("Initial shear-wave velocity"));
    connect( m_initialShearVelCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->initialShearVel(), SLOT(setEnabled(bool)));
    layout->addWidget(m_initialShearVelCheckBox);

    m_finalShearVelCheckBox = new QCheckBox(tr("Final shear-wave velocity"));
    connect( m_finalShearVelCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->finalShearVel(), SLOT(setEnabled(bool)));
    layout->addWidget(m_finalShearVelCheckBox);

    m_finalShearModCheckBox = new QCheckBox(tr("Final shear modulus"));
    connect( m_finalShearModCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->finalShearMod(), SLOT(setEnabled(bool)));
    layout->addWidget(m_finalShearModCheckBox);

    m_finalDampingCheckBox = new QCheckBox(tr("Final damping"));
    connect( m_finalDampingCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->finalDamping(), SLOT(setEnabled(bool)));
    layout->addWidget(m_finalDampingCheckBox);
    
    m_maxErrorCheckBox = new QCheckBox(QString(tr("Maximum Error")));
    connect( m_maxErrorCheckBox, SIGNAL(toggled(bool)),
	    m_model->output()->maxError(), SLOT(setEnabled(bool)));
    layout->addWidget(m_maxErrorCheckBox);

    // Create the group box
    m_profilesGroupBox = new QGroupBox(tr("Profiles"));
    m_profilesGroupBox->setLayout(layout);
}

void OutputPage::createRespSpecGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(1,1);
    layout->setColumnMinimumWidth(0,10);

    // Damping
    m_dampingSpinBox = new QDoubleSpinBox;
    m_dampingSpinBox->setSuffix(" %");
    m_dampingSpinBox->setRange( 1, 50);
    m_dampingSpinBox->setSingleStep(1);
    m_dampingSpinBox->setDecimals(1);
    connect( m_dampingSpinBox, SIGNAL(valueChanged(double)),
            m_model->output(), SLOT(setDamping(double)));


    layout->addWidget(new QLabel(tr("Damping:")), 0, 0, 1, 2);
    layout->addWidget(m_dampingSpinBox, 0, 2);
   
    layout->addWidget(new QLabel(tr("Period:")), 1, 0, 1, 2);
    // Period Min
    m_periodMinSpinBox = new QDoubleSpinBox;
    m_periodMinSpinBox->setRange(0, 1);
    m_periodMinSpinBox->setSingleStep(0.01);
    m_periodMinSpinBox->setSuffix(" s");
    connect( m_periodMinSpinBox, SIGNAL(valueChanged(double)),
            m_model->output()->period(), SLOT(setMin(double)));

    layout->addWidget( new QLabel(tr("Minimum:")), 2, 1);
    layout->addWidget( m_periodMinSpinBox, 2, 2);
    
    // Period Maximum
    m_periodMaxSpinBox = new QDoubleSpinBox;
    m_periodMaxSpinBox->setRange(1, 20);
    m_periodMaxSpinBox->setSingleStep(1);
    m_periodMaxSpinBox->setSuffix(" s");
    connect( m_periodMaxSpinBox, SIGNAL(valueChanged(double)),
            m_model->output()->period(), SLOT(setMax(double)));

    layout->addWidget( new QLabel(tr("Maximum:")), 3, 1);
    layout->addWidget( m_periodMaxSpinBox, 3, 2);

    // Period number of points
    m_periodNptsSpinBox = new QSpinBox;
    m_periodNptsSpinBox->setRange( 32, 256);
    connect( m_periodNptsSpinBox, SIGNAL(valueChanged(int)), 
            m_model->output()->period(), SLOT(setSize(int)));

    layout->addWidget( new QLabel(tr("No. of points:")), 4, 1);
    layout->addWidget( m_periodNptsSpinBox, 4, 2);

    // Spacing
    m_periodSpacingComboBox = new QComboBox;
    m_periodSpacingComboBox->addItems(Dimension::spacingList());
    connect( m_periodSpacingComboBox, SIGNAL(currentIndexChanged(int)),
            m_model->output()->period(), SLOT(setSpacing(int)));

    layout->addWidget( new QLabel(tr("Spacing:")), 5, 1);
    layout->addWidget( m_periodSpacingComboBox, 5, 2);

    m_respSpecGroupBox = new QGroupBox(tr("Response Spectrum Properties"));
    m_respSpecGroupBox->setLayout(layout);
    m_respSpecGroupBox->setVisible(false);
}

void OutputPage::createFreqGroupBox() 
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Frequency Min
    m_freqMinSpinBox = new QDoubleSpinBox;
    m_freqMinSpinBox->setRange(0, 5);
    m_freqMinSpinBox->setSingleStep(0.01);
    m_freqMinSpinBox->setSuffix(" Hz");
    connect( m_freqMinSpinBox, SIGNAL(valueChanged(double)),
            m_model->output()->freq(), SLOT(setMin(double)));

    layout->addWidget( new QLabel(tr("Frequency Minimum:")), 1, 0);
    layout->addWidget( m_freqMinSpinBox, 1, 1);
    
    // Frequency Maximum
    m_freqMaxSpinBox = new QDoubleSpinBox;
    m_freqMaxSpinBox->setRange(10, 100);
    m_freqMaxSpinBox->setSingleStep(1);
    m_freqMaxSpinBox->setSuffix(" Hz");
    connect( m_freqMaxSpinBox, SIGNAL(valueChanged(double)),
            m_model->output()->freq(), SLOT(setMax(double)));

    layout->addWidget( new QLabel(tr("Frequency Maximum:")), 2, 0);
    layout->addWidget( m_freqMaxSpinBox, 2, 1);

    // Frequency number of points
    m_freqNptsSpinBox = new QSpinBox;
    m_freqNptsSpinBox->setRange( 64, 8192);
    connect( m_freqNptsSpinBox, SIGNAL(valueChanged(int)),
            m_model->output()->freq(), SLOT(setSize(int)));

    layout->addWidget( new QLabel(tr("No. of points:")), 3, 0);
    layout->addWidget( m_freqNptsSpinBox, 3, 1);
    
    // Spacing
    m_freqSpacingComboBox = new QComboBox;
    m_freqSpacingComboBox->addItems(Dimension::spacingList());
    connect( m_freqSpacingComboBox, SIGNAL(currentIndexChanged(int)),
            m_model->output()->freq(), SLOT(setSpacing(int)));

    layout->addWidget( new QLabel(tr("Spacing:")), 4, 0);
    layout->addWidget( m_freqSpacingComboBox, 4, 1);

    // Create the group box
    m_freqGroupBox = new QGroupBox(tr("FAS Frequency Properties")) ;
    m_freqGroupBox->setLayout(layout);
    m_freqGroupBox->setVisible(false);
}

void OutputPage::createLogGroupBox()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0,1);

    // Logging combo box
    m_logLevelComboBox = new QComboBox;
    m_logLevelComboBox->addItems(TextLog::levelList());
    connect(m_logLevelComboBox, SIGNAL(currentIndexChanged(int)), m_model->output()->textLog(), SLOT(setLevel(int)));

    layout->addWidget(new QLabel(tr("Logging level:")), 0, 0);
    layout->addWidget(m_logLevelComboBox, 0, 1);

    m_logGroupBox = new QGroupBox(tr("Logging Properties"));
    m_logGroupBox->setLayout(layout);
}

void OutputPage::load()
{
    // Profiles
    m_initialShearVelCheckBox->setChecked(
            m_model->output()->initialShearVel()->enabled());
    m_finalShearVelCheckBox->setChecked(
            m_model->output()->finalShearVel()->enabled());
    m_finalShearModCheckBox->setChecked(
            m_model->output()->finalShearMod()->enabled());
    m_finalDampingCheckBox->setChecked(
            m_model->output()->finalDamping()->enabled());
    m_vTotalStressCheckBox->setChecked(
            m_model->output()->vTotalStress()->enabled());
    m_maxShearStressCheckBox->setChecked(
            m_model->output()->maxShearStress()->enabled());
    m_stressReducCoeffCheckBox->setChecked(
            m_model->output()->stressReducCoeff()->enabled());
    m_maxShearStrainCheckBox->setChecked(
            m_model->output()->maxShearStrain()->enabled());
    m_maxAccelCheckBox->setChecked(
            m_model->output()->maxAccel()->enabled());
    m_maxVelCheckBox->setChecked(
            m_model->output()->maxVel()->enabled());
    m_stressRatioCheckBox->setChecked(
            m_model->output()->stressRatio()->enabled());
    m_maxErrorCheckBox->setChecked(
            m_model->output()->maxError()->enabled());

    // Response Spectrum information
    m_dampingSpinBox->setValue(
            m_model->output()->damping());

    // Period information
    m_periodMinSpinBox->setValue(
            m_model->output()->period()->min());
    m_periodMaxSpinBox->setValue(
            m_model->output()->period()->max());
    m_periodNptsSpinBox->setValue(
            m_model->output()->period()->size());
    m_periodSpacingComboBox->setCurrentIndex(
            m_model->output()->period()->spacing());
    
    // Frequency information
    m_freqMinSpinBox->setValue(
            m_model->output()->freq()->min());
    m_freqMaxSpinBox->setValue(
            m_model->output()->freq()->max());
    m_freqNptsSpinBox->setValue(
            m_model->output()->freq()->size());
    m_freqSpacingComboBox->setCurrentIndex(
            m_model->output()->freq()->spacing());
    
    // Logging information
    m_logLevelComboBox->setCurrentIndex(
            m_model->output()->textLog()->level());

    // Show the frequency and period groups
    updateWidget(); 
}

void OutputPage::updateWidget()
{
    if (m_model->output()->periodIsNeeded()) {
        m_respSpecGroupBox->setVisible(true);
    } else {
        m_respSpecGroupBox->setVisible(false);
    }
    
    if (m_model->output()->freqIsNeeded()) {
        m_freqGroupBox->setVisible(true);
    } else {
        m_freqGroupBox->setVisible(false);
    }
}
