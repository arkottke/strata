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

#include "MethodGroupBox.h"
#include "MotionLibrary.h"
#include "MyRandomNumGenerator.h"
#include "NonlinearPropertyRandomizer.h"
#include "OutputCatalog.h"
#include "ProfileRandomizer.h"
#include "SiteResponseModel.h"
#include "SoilProfile.h"
#include "Units.h"

#include <QDebug>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>

GeneralPage::GeneralPage(QWidget * parent, Qt::WindowFlags f )
    : AbstractPage(parent, f)
{
    m_methodGroupBox = new MethodGroupBox;

    // Layout of the widget
    QGridLayout * layout = new QGridLayout;
    layout->addWidget(createProjectGroupBox(), 0, 0, 4, 1 );
    layout->addWidget(createAnalysisGroupBox(), 0, 1);
    layout->addWidget(createVariationGroupBox(), 1, 1 );
    layout->addWidget(m_methodGroupBox, 2, 1 );
    layout->addWidget(createDiscretizationGroupBox(), 3, 1);

    // Add a row of stretching
    layout->setRowStretch( 4, 1);
    layout->setColumnStretch( 0, 1);

    setLayout(layout);
}

QGroupBox* GeneralPage::createProjectGroupBox()
{
    QGridLayout *layout = new QGridLayout;
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(2, 1);

    // Title
    m_titleLineEdit = new QLineEdit;

    layout->addWidget(new QLabel(tr("Title:")), 0, 0);
    layout->addWidget(m_titleLineEdit, 0, 1, 1, 2);

    // Notes
    m_notesTextEdit = new QTextEdit;

    layout->addWidget(new QLabel(tr("Notes:")), 1, 0);
    layout->addWidget(m_notesTextEdit, 1, 1, 2, 2);

    // File name prefix
    m_prefixLineEdit = new QLineEdit;

    layout->addWidget(new QLabel(tr("Filename prefix:")), 3, 0);
    layout->addWidget(m_prefixLineEdit, 3, 1);

    // Units
    m_unitsComboBox = new QComboBox;
    m_unitsComboBox->addItems(Units::systemList());

    layout->addWidget(new QLabel(tr("Units:")), 4, 0);
    layout->addWidget(m_unitsComboBox, 4, 1);
    layout->addWidget(
            new QLabel(tr("Note: only changes labels and gravity, no unit conversion.")), 4, 2);

    // Save motion data
    m_saveMotionDataCheckBox = new QCheckBox(tr("Save motion data within the input file."));
    layout->addWidget(m_saveMotionDataCheckBox, 5, 1, 1, 2);

    // Create the group box and add the layout
    QGroupBox* groupBox = new QGroupBox(tr("Project"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox*  GeneralPage::createAnalysisGroupBox()
{
    QFormLayout * layout = new QFormLayout;

    // Method
    m_methodComboBox = new QComboBox;
    m_methodComboBox->addItems(SiteResponseModel::methodList());
    layout->addRow(tr("Method:"), m_methodComboBox);

    // Approach
    m_approachComboBox = new QComboBox;
    m_approachComboBox->addItems(MotionLibrary::approachList());
    layout->addRow(tr("Approach:"), m_approachComboBox);

    // Site varied
    m_propertiesAreVariedCheckBox = new QCheckBox(tr("Vary the properties"));
    layout->addRow(m_propertiesAreVariedCheckBox);

    // Create the group box and add the layout
    QGroupBox* groupBox = new QGroupBox(tr("Type of Analysis"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox* GeneralPage::createVariationGroupBox()
{
    const int indent = 20;
    QFormLayout * layout = new QFormLayout;

    // Count
    m_countSpinBox = new QSpinBox;
    m_countSpinBox->setRange( 1, 1000);

    layout->addRow(tr("Number of realizations:"), m_countSpinBox);

    // Checkboxes for variation
    m_nlPropertiesAreVariedCheckBox = new QCheckBox(tr("Vary the nonlinear properties"));
    layout->addRow(m_nlPropertiesAreVariedCheckBox);

    QLabel* label = new QLabel(tr(
            "-- shear-modulus reduction curve\n"
            "-- damping ratio curve\n"
            "-- damping of the bedrock"));
    label->setIndent(indent);
    layout->addRow(label);

    m_siteIsVariedCheckBox = new QCheckBox(tr("Vary the site profile"));
    layout->addRow(m_siteIsVariedCheckBox);

    label = new QLabel(tr(
            "-- shear-wave velocity\n"
            "-- layer thickness\n"
            "-- depth to bedrock"));
    label->setIndent(indent);
    layout->addRow(label);

    // Control of seed
    m_specifiedSeedCheckBox = new QCheckBox(tr("Specify seed number"));
    m_seedSpinBox = new QSpinBox;
    m_seedSpinBox->setRange(0, 65535);
    m_seedSpinBox->setEnabled(false);
    connect(m_specifiedSeedCheckBox, SIGNAL(toggled(bool)),
            m_seedSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(m_specifiedSeedCheckBox, m_seedSpinBox);

    // Create the group box and add the layout
    m_variationGroupBox = new QGroupBox(tr("Site Property Variation"));
    m_variationGroupBox->setLayout(layout);

    return m_variationGroupBox;
}

QGroupBox* GeneralPage::createDiscretizationGroupBox()
{
    QFormLayout* layout = new QFormLayout;

    // Maximum frequency
    m_maxFreqSpinBox = new QDoubleSpinBox;
    m_maxFreqSpinBox->setRange( 15, 100);
    m_maxFreqSpinBox->setDecimals(0);
    m_maxFreqSpinBox->setSuffix(" Hz");

    layout->addRow(tr("Maximum frequency:"), m_maxFreqSpinBox);

    // Wave length fraction
    m_waveFractionSpinBox =  new QDoubleSpinBox;
    m_waveFractionSpinBox->setRange( 0.10, 0.35);
    m_waveFractionSpinBox->setDecimals(2);

    layout->addRow(tr("Wavelength fraction:"), m_waveFractionSpinBox);

    // Disable auto-discretization
    m_disableDiscretzationCheckBox = new QCheckBox(tr("Disable auto-discretization"));

    layout->addRow(m_disableDiscretzationCheckBox);

    // Group box 
    QGroupBox* groupBox = new QGroupBox(tr("Layer Discretization"));
    groupBox->setLayout(layout);

    return groupBox;
}

void GeneralPage::setModel(SiteResponseModel *model)
{   
    m_titleLineEdit->setText(model->outputCatalog()->title());
    connect(m_titleLineEdit, SIGNAL(textChanged(QString)),
             model->outputCatalog(), SLOT(setTitle(QString)));

    m_notesTextEdit->setDocument(model->notes());

    m_prefixLineEdit->setText(model->outputCatalog()->filePrefix());

    connect(m_prefixLineEdit, SIGNAL(textChanged(QString)),
             model->outputCatalog(), SLOT(setFilePrefix(QString)));

    m_unitsComboBox->setCurrentIndex(Units::instance()->system());
    connect(m_unitsComboBox, SIGNAL(currentIndexChanged(int)),
            Units::instance(), SLOT(setSystem(int)));

    m_saveMotionDataCheckBox->setChecked(model->motionLibrary()->saveData());
    connect(m_saveMotionDataCheckBox, SIGNAL(toggled(bool)),
            model->motionLibrary(), SLOT(setSaveData(bool)));

    m_methodComboBox->setCurrentIndex(model->method());
    connect(m_methodComboBox, SIGNAL(currentIndexChanged(int)),
            model, SLOT(setMethod(int)));

    m_approachComboBox->setCurrentIndex(model->motionLibrary()->approach());
    connect(m_approachComboBox, SIGNAL(currentIndexChanged(int)),
            model->motionLibrary(), SLOT(setApproach(int)));

    m_propertiesAreVariedCheckBox->setChecked(model->siteProfile()->isVaried());
    connect(m_propertiesAreVariedCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile(), SLOT(setIsVaried(bool)));

    m_variationGroupBox->setEnabled(model->siteProfile()->isVaried());
    connect(model->siteProfile(), SIGNAL(isVariedChanged(bool)),
            m_variationGroupBox, SLOT(setEnabled(bool)));

    m_countSpinBox->setValue(model->siteProfile()->profileCount());
    connect(m_countSpinBox, SIGNAL(valueChanged(int)),
            model->siteProfile(), SLOT(setProfileCount(int)));

    m_nlPropertiesAreVariedCheckBox->setChecked(
            model->siteProfile()->nonlinearPropertyRandomizer()->enabled());
    connect(m_nlPropertiesAreVariedCheckBox, SIGNAL(toggled(bool)),
             model->siteProfile()->nonlinearPropertyRandomizer(), SLOT(setEnabled(bool)));

    m_siteIsVariedCheckBox->setChecked(
            model->siteProfile()->profileRandomizer()->enabled());
    connect(m_siteIsVariedCheckBox, SIGNAL(toggled(bool)),
             model->siteProfile()->profileRandomizer(), SLOT(setEnabled(bool)));

    m_specifiedSeedCheckBox->setChecked(model->randNumGen()->seedSpecified());
    connect(m_specifiedSeedCheckBox, SIGNAL(toggled(bool)),
            model->randNumGen(), SLOT(setSeedSpecified(bool)));

    m_seedSpinBox->setValue(model->randNumGen()->seed());
    connect(m_seedSpinBox, SIGNAL(valueChanged(int)),
            model->randNumGen(), SLOT(setSeed(int)));
    connect(model->randNumGen(), SIGNAL(seedChanged(int)),
            m_seedSpinBox, SLOT(setValue(int)));

    m_methodGroupBox->setCalculator(model->calculator());
    connect(model, SIGNAL(calculatorChanged(AbstractCalculator*)),
            m_methodGroupBox, SLOT(setCalculator(AbstractCalculator*)));

    m_maxFreqSpinBox->setValue(model->siteProfile()->maxFreq());
    connect(m_maxFreqSpinBox, SIGNAL(valueChanged(double)),
             model->siteProfile(), SLOT(setMaxFreq(double)));

    m_waveFractionSpinBox->setValue(model->siteProfile()->waveFraction());
    connect(m_waveFractionSpinBox, SIGNAL(valueChanged(double)),
             model->siteProfile(), SLOT(setWaveFraction(double)));

    m_disableDiscretzationCheckBox->setChecked(model->siteProfile()->disableAutoDiscretization());
    connect(m_disableDiscretzationCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile(), SLOT(setDisableAutoDiscretization(bool)));
}

void GeneralPage::setReadOnly(bool readOnly)
{
    m_titleLineEdit->setReadOnly(readOnly);
    m_notesTextEdit->setReadOnly(readOnly);
    m_prefixLineEdit->setReadOnly(readOnly);
    m_unitsComboBox->setDisabled(readOnly);
    m_saveMotionDataCheckBox->setDisabled(readOnly);

    m_methodComboBox->setDisabled(readOnly);
    m_approachComboBox->setDisabled(readOnly);
    m_propertiesAreVariedCheckBox->setDisabled(readOnly);

    m_countSpinBox->setReadOnly(readOnly);
    m_nlPropertiesAreVariedCheckBox->setDisabled(readOnly);
    m_siteIsVariedCheckBox->setDisabled(readOnly);
    m_specifiedSeedCheckBox->setDisabled(readOnly);
    m_seedSpinBox->setDisabled(readOnly || !m_specifiedSeedCheckBox->isChecked());

    m_methodGroupBox->setReadOnly(readOnly);

    m_maxFreqSpinBox->setReadOnly(readOnly);
    m_waveFractionSpinBox->setReadOnly(readOnly);
    m_disableDiscretzationCheckBox->setDisabled(readOnly);
}
