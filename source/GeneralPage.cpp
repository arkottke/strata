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

#include <QFormLayout>
#include <QLabel>

GeneralPage::GeneralPage(QWidget * parent, Qt::WindowFlags f )
        : AbstractPage(parent, f)
{
    _methodGroupBox = new MethodGroupBox;

    // Layout of the widget
    auto *layout = new QGridLayout;
    layout->addWidget(createProjectGroupBox(), 0, 0, 5, 1);
    layout->addWidget(createAnalysisGroupBox(), 0, 1);
    layout->addWidget(createVariationGroupBox(), 1, 1);
    layout->addWidget(_methodGroupBox, 2, 1);
    layout->addWidget(createDiscretizationGroupBox(), 3, 1);

    // Add a row of stretching
    layout->setRowStretch( 4, 1);
    layout->setColumnStretch( 0, 1);

    setLayout(layout);
}

QGroupBox* GeneralPage::createProjectGroupBox()
{
    auto *layout = new QGridLayout;
    layout->setRowStretch(2, 1);
    layout->setColumnStretch(2, 1);

    // Title
    _titleLineEdit = new QLineEdit;

    layout->addWidget(new QLabel(tr("Title:")), 0, 0);
    layout->addWidget(_titleLineEdit, 0, 1, 1, 2);

    // Notes
    _notesTextEdit = new QTextEdit;

    layout->addWidget(new QLabel(tr("Notes:")), 1, 0);
    layout->addWidget(_notesTextEdit, 1, 1, 2, 2);

    // File name prefix
    _prefixLineEdit = new QLineEdit;

    layout->addWidget(new QLabel(tr("Filename prefix:")), 3, 0);
    layout->addWidget(_prefixLineEdit, 3, 1);

    // Units
    _unitsComboBox = new QComboBox;
    _unitsComboBox->addItems(Units::systemList());

    layout->addWidget(new QLabel(tr("Units:")), 4, 0);
    layout->addWidget(_unitsComboBox, 4, 1);
    layout->addWidget(
            new QLabel(tr("Note: only changes labels and gravity, no unit conversion.")), 4, 2);

    // Save motion data
    _saveMotionDataCheckBox = new QCheckBox(tr("Save motion data within the input file."));
    layout->addWidget(_saveMotionDataCheckBox, 5, 1, 1, 2);

    // Create the group box and add the layout
    QGroupBox* groupBox = new QGroupBox(tr("Project"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox*  GeneralPage::createAnalysisGroupBox()
{
    auto layout = new QFormLayout;

    // Method
    _methodComboBox = new QComboBox;
    _methodComboBox->addItems(SiteResponseModel::methodList());
    layout->addRow(tr("Method:"), _methodComboBox);

    // Approach
    _approachComboBox = new QComboBox;
    _approachComboBox->addItems(MotionLibrary::approachList());
    layout->addRow(tr("Approach:"), _approachComboBox);

    // Site varied
    _propertiesAreVariedCheckBox = new QCheckBox(tr("Vary the properties"));
    layout->addRow(_propertiesAreVariedCheckBox);

    // Create the group box and add the layout
    QGroupBox* groupBox = new QGroupBox(tr("Type of Analysis"));
    groupBox->setLayout(layout);

    return groupBox;
}

QGroupBox* GeneralPage::createVariationGroupBox()
{
    const int indent = 20;
    auto * layout = new QFormLayout;

    // Count
    _countSpinBox = new QSpinBox;
    _countSpinBox->setRange( 1, 1000);

    layout->addRow(tr("Number of realizations:"), _countSpinBox);

    // Checkboxes for variation
    _nlPropertiesAreVariedCheckBox = new QCheckBox(tr("Vary the nonlinear properties"));
    layout->addRow(_nlPropertiesAreVariedCheckBox);

    QLabel* label = new QLabel(tr(
            "-- shear-modulus reduction curve\n"
                    "-- damping ratio curve\n"
                    "-- damping of the bedrock"));
    label->setIndent(indent);
    layout->addRow(label);

    _siteIsVariedCheckBox = new QCheckBox(tr("Vary the site profile"));
    layout->addRow(_siteIsVariedCheckBox);

    label = new QLabel(tr(
            "-- shear-wave velocity\n"
                    "-- layer thickness\n"
                    "-- depth to bedrock"));
    label->setIndent(indent);
    layout->addRow(label);

    // Control of seed
    _specifiedSeedCheckBox = new QCheckBox(tr("Specify seed number"));
    _seedSpinBox = new QSpinBox;
    _seedSpinBox->setRange(0, 65535);
    _seedSpinBox->setEnabled(false);
    connect(_specifiedSeedCheckBox, SIGNAL(toggled(bool)),
            _seedSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(_specifiedSeedCheckBox, _seedSpinBox);

    // Create the group box and add the layout
    _variationGroupBox = new QGroupBox(tr("Site Property Variation"));
    _variationGroupBox->setLayout(layout);

    return _variationGroupBox;
}

QGroupBox* GeneralPage::createDiscretizationGroupBox()
{
    auto *layout = new QFormLayout;

    // Maximum frequency
    _maxFreqSpinBox = new QDoubleSpinBox;
    _maxFreqSpinBox->setRange( 15, 100);
    _maxFreqSpinBox->setDecimals(0);
    _maxFreqSpinBox->setSuffix(" Hz");

    layout->addRow(tr("Maximum frequency:"), _maxFreqSpinBox);

    // Wave length fraction
    _waveFractionSpinBox =  new QDoubleSpinBox;
    _waveFractionSpinBox->setRange( 0.10, 0.35);
    _waveFractionSpinBox->setDecimals(2);

    layout->addRow(tr("Wavelength fraction:"), _waveFractionSpinBox);

    // Disable auto-discretization
    _disableDiscretzationCheckBox = new QCheckBox(tr("Disable auto-discretization"));

    layout->addRow(_disableDiscretzationCheckBox);

    // Group box 
    QGroupBox* groupBox = new QGroupBox(tr("Layer Discretization"));
    groupBox->setLayout(layout);

    return groupBox;
}

void GeneralPage::setModel(SiteResponseModel *model)
{
    _titleLineEdit->setText(model->outputCatalog()->title());
    connect(_titleLineEdit, SIGNAL(textChanged(QString)),
            model->outputCatalog(), SLOT(setTitle(QString)));

    _notesTextEdit->setDocument(model->notes());

    _prefixLineEdit->setText(model->outputCatalog()->filePrefix());

    connect(_prefixLineEdit, SIGNAL(textChanged(QString)),
            model->outputCatalog(), SLOT(setFilePrefix(QString)));

    _unitsComboBox->setCurrentIndex(Units::instance()->system());
    connect(_unitsComboBox, SIGNAL(currentIndexChanged(int)),
            Units::instance(), SLOT(setSystem(int)));

    _saveMotionDataCheckBox->setChecked(model->motionLibrary()->saveData());
    connect(_saveMotionDataCheckBox, SIGNAL(toggled(bool)),
            model->motionLibrary(), SLOT(setSaveData(bool)));

    _methodComboBox->setCurrentIndex(model->method());
    connect(_methodComboBox, SIGNAL(currentIndexChanged(int)),
            model, SLOT(setMethod(int)));

    _approachComboBox->setCurrentIndex(model->motionLibrary()->approach());
    connect(_approachComboBox, SIGNAL(currentIndexChanged(int)),
            model->motionLibrary(), SLOT(setApproach(int)));

    _propertiesAreVariedCheckBox->setChecked(model->siteProfile()->isVaried());
    connect(_propertiesAreVariedCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile(), SLOT(setIsVaried(bool)));

    _variationGroupBox->setEnabled(model->siteProfile()->isVaried());
    connect(model->siteProfile(), SIGNAL(isVariedChanged(bool)),
            _variationGroupBox, SLOT(setEnabled(bool)));

    _countSpinBox->setValue(model->siteProfile()->profileCount());
    connect(_countSpinBox, SIGNAL(valueChanged(int)),
            model->siteProfile(), SLOT(setProfileCount(int)));

    _nlPropertiesAreVariedCheckBox->setChecked(
            model->siteProfile()->nonlinearPropertyRandomizer()->enabled());
    connect(_nlPropertiesAreVariedCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile()->nonlinearPropertyRandomizer(), SLOT(setEnabled(bool)));

    _siteIsVariedCheckBox->setChecked(
            model->siteProfile()->profileRandomizer()->enabled());
    connect(_siteIsVariedCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile()->profileRandomizer(), SLOT(setEnabled(bool)));

    _specifiedSeedCheckBox->setChecked(model->randNumGen()->seedSpecified());
    connect(_specifiedSeedCheckBox, SIGNAL(toggled(bool)),
            model->randNumGen(), SLOT(setSeedSpecified(bool)));

    _seedSpinBox->setValue(model->randNumGen()->seed());
    connect(_seedSpinBox, SIGNAL(valueChanged(int)),
            model->randNumGen(), SLOT(setSeed(int)));
    connect(model->randNumGen(), SIGNAL(seedChanged(int)),
            _seedSpinBox, SLOT(setValue(int)));

    _methodGroupBox->setCalculator(model->calculator());
    connect(model, SIGNAL(calculatorChanged(AbstractCalculator*)),
            _methodGroupBox, SLOT(setCalculator(AbstractCalculator*)));

    _maxFreqSpinBox->setValue(model->siteProfile()->maxFreq());
    connect(_maxFreqSpinBox, SIGNAL(valueChanged(double)),
            model->siteProfile(), SLOT(setMaxFreq(double)));

    _waveFractionSpinBox->setValue(model->siteProfile()->waveFraction());
    connect(_waveFractionSpinBox, SIGNAL(valueChanged(double)),
            model->siteProfile(), SLOT(setWaveFraction(double)));

    _disableDiscretzationCheckBox->setChecked(model->siteProfile()->disableAutoDiscretization());
    connect(_disableDiscretzationCheckBox, SIGNAL(toggled(bool)),
            model->siteProfile(), SLOT(setDisableAutoDiscretization(bool)));
}

void GeneralPage::setReadOnly(bool readOnly)
{
    _titleLineEdit->setReadOnly(readOnly);
    _notesTextEdit->setReadOnly(readOnly);
    _prefixLineEdit->setReadOnly(readOnly);
    _unitsComboBox->setDisabled(readOnly);
    _saveMotionDataCheckBox->setDisabled(readOnly);

    _methodComboBox->setDisabled(readOnly);
    _approachComboBox->setDisabled(readOnly);
    _propertiesAreVariedCheckBox->setDisabled(readOnly);

    _countSpinBox->setReadOnly(readOnly);
    _nlPropertiesAreVariedCheckBox->setDisabled(readOnly);
    _siteIsVariedCheckBox->setDisabled(readOnly);
    _specifiedSeedCheckBox->setDisabled(readOnly);
    _seedSpinBox->setDisabled(readOnly || !_specifiedSeedCheckBox->isChecked());

    _methodGroupBox->setReadOnly(readOnly);

    _maxFreqSpinBox->setReadOnly(readOnly);
    _waveFractionSpinBox->setReadOnly(readOnly);
    _disableDiscretzationCheckBox->setDisabled(readOnly);
}