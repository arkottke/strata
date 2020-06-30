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

#include "SourceTheoryRvtMotionDialog.h"

#include "CrustalAmplification.h"
#include "CrustalModel.h"
#include "Dimension.h"
#include "DimensionLayout.h"
#include "PathDurationModel.h"
#include "SourceTheoryRvtMotion.h"
#include "TableGroupBox.h"

#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

SourceTheoryRvtMotionDialog::SourceTheoryRvtMotionDialog(
        SourceTheoryRvtMotion *motion,
        bool readOnly,
        QWidget *parent) :
    AbstractRvtMotionDialog(motion, readOnly, parent)
{
    init();
}

auto SourceTheoryRvtMotionDialog::createParametersLayout() -> QFormLayout*
{
    auto strm = qobject_cast<SourceTheoryRvtMotion*>(_motion);
    auto layout = AbstractRvtMotionDialog::createParametersLayout();

    const bool isCustomized = strm->isCustomized();

    // Customize
    auto checkbox = new QCheckBox(tr("Customize source parameters"));
    checkbox->setChecked(isCustomized);
    checkbox->setDisabled(_readOnly);

    connect(checkbox, SIGNAL(clicked(bool)), strm, SLOT(setIsCustomized(bool)));

    layout->addRow(checkbox);

    // Depth
    auto doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 20);
    doubleSpinBox->setDecimals(1);
    doubleSpinBox->setSingleStep(1);
    doubleSpinBox->setSuffix(" km");
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->depth());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setDepth(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Depth:"), doubleSpinBox);

    // Stress drop
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(5, 500);
    doubleSpinBox->setDecimals(0);
    doubleSpinBox->setSingleStep(5);
    doubleSpinBox->setSuffix(" bars");
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->stressDrop());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setStressDrop(double)));
    connect(strm, SIGNAL(stressDropChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Stress drop (%1%2)")).arg(QChar(0x0394)).arg(QChar(0x03c3)),
                   doubleSpinBox);

    // Geometric attenuation
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0.001, 1);
    doubleSpinBox->setDecimals(4);
    doubleSpinBox->setSingleStep(0.01);
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->geoAtten());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setGeoAtten(double)));
    connect(strm, SIGNAL(geoAttenChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Geometric atten. coeff.:"), doubleSpinBox);

    // Path attenuation
    layout->addRow(new QLabel(tr("Path attenuation, Q(f) = <b>a</b> f <sup><b>b</b></sup>")));

    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(50, 10000);
    doubleSpinBox->setDecimals(0);
    doubleSpinBox->setSingleStep(10);
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->pathAttenCoeff());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setPathAttenCoeff(double)));
    connect(strm, SIGNAL(pathAttenCoeffChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    auto label = new QLabel(tr("Coefficient (a):"));
    const int indent = 20;
    label->setIndent(indent);
    layout->addRow(label, doubleSpinBox);

    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0.0, 1.0);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.01);
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->pathAttenPower());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setPathAttenPower(double)));
    connect(strm, SIGNAL(pathAttenPowerChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    label = new QLabel(tr("Power (b):"));
    label->setIndent(indent);
    layout->addRow(label, doubleSpinBox);

    // Shear velocity
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(2.0, 5.0);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.1);
    doubleSpinBox->setSuffix(" km/sec");
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->shearVelocity());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setShearVelocity(double)));
    connect(strm, SIGNAL(shearVelocityChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Shear velocity (v<sub>s</sub>):"), doubleSpinBox);

    // Density
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(2.4, 3.5);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSingleStep(0.1);
    doubleSpinBox->setSuffix(" g/cc");
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->density());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setDensity(double)));
    connect(strm, SIGNAL(densityChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Density (%1)")).arg(QChar(0x03c1)), doubleSpinBox);

    // Site Attenuation
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0.001, 0.10);
    doubleSpinBox->setDecimals(4);
    doubleSpinBox->setSingleStep(0.001);
    doubleSpinBox->setSuffix(" sec");
    doubleSpinBox->setReadOnly(_readOnly);

    doubleSpinBox->setValue(strm->siteAtten());
    doubleSpinBox->setEnabled(isCustomized);
    connect(doubleSpinBox, SIGNAL(valueChanged(double)),
            strm, SLOT(setSiteAtten(double)));
    connect(strm, SIGNAL(siteAttenChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            doubleSpinBox, SLOT(setEnabled(bool)));

    layout->addRow(QString(tr("Site attenuation (%1<sub>0</sub>)")).arg(QChar(0x03ba)), doubleSpinBox);

    // Duration
    doubleSpinBox = new QDoubleSpinBox;
    doubleSpinBox->setRange(0, 1000);
    doubleSpinBox->setDecimals(2);
    doubleSpinBox->setSuffix(" sec");
    doubleSpinBox->setReadOnly(true);
    doubleSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);

    doubleSpinBox->setValue(strm->duration());
    connect(strm, SIGNAL(durationChanged(double)),
            doubleSpinBox, SLOT(setValue(double)));

    layout->addRow(tr("Duration:"), doubleSpinBox);

    // Path duration model
    auto comboBox = new QComboBox;
    comboBox->addItems(PathDurationModel::sourceList());
    comboBox->setCurrentIndex(strm->pathDuration()->source());
    comboBox->setDisabled(_readOnly | !isCustomized);

    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updatePathDurSource(int)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            strm->pathDuration(), SLOT(setSource(int)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            comboBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Path duration model:"), comboBox);

    // Crustal amp model
    comboBox = new QComboBox;
    comboBox->addItems(CrustalAmplification::sourceList());
    comboBox->setCurrentIndex(strm->crustalAmp()->source());
    comboBox->setDisabled(_readOnly | !isCustomized);

    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateCrustalAmpSource(int)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
            strm->crustalAmp(), SLOT(setSource(int)));
    connect(strm, SIGNAL(isCustomizedChanged(bool)),
            comboBox, SLOT(setEnabled(bool)));

    layout->addRow(tr("Crustal amplification model:"), comboBox);

    // Add additional tabs
    _paramsTabWidget = new QTabWidget;
    auto frame = new QFrame;
    frame->setLayout(layout);
    _paramsTabWidget->addTab(frame, tr("Parameters"));

    // Path duration model
    auto vboxLayout = new QVBoxLayout;
    _pathDurTableGroupBox = new TableGroupBox(tr("Path Duration"));
    _pathDurTableGroupBox->setModel(strm->pathDuration());

    vboxLayout->addWidget(_pathDurTableGroupBox);

    frame = new QFrame;
    frame->setLayout(vboxLayout);
    _paramsTabWidget->addTab(frame, tr("Path Duration"));

    // Crustal amplification
    vboxLayout = new QVBoxLayout;
    _crustalAmpGroupBox = new TableGroupBox(tr("Crustal Ampl."));
    _crustalAmpGroupBox->setModel(strm->crustalAmp());
    _crustalAmpGroupBox->setReadOnly(_readOnly);
    connect(strm->crustalAmp(), SIGNAL(readOnlyChanged(bool)),
            _crustalAmpGroupBox, SLOT(setReadOnly(bool)));

    vboxLayout->addWidget(_crustalAmpGroupBox);
    frame = new QFrame;
    frame->setLayout(vboxLayout);
    _paramsTabWidget->addTab(frame, tr("Crustal Ampl."));

    // Crustal model
    vboxLayout = new QVBoxLayout;
    _crustalModelGroupBox = new TableGroupBox(tr("Crustal Model"));
    _crustalModelGroupBox->setModel(strm->crustalAmp()->crustalModel());
    _crustalModelGroupBox->setReadOnly(_readOnly);

    vboxLayout->addWidget(_crustalModelGroupBox);

    frame = new QFrame;
    frame->setLayout(vboxLayout);
    _crustModelIndex = _paramsTabWidget->addTab(frame, tr("Crustal Model."));

    updatePathDurSource(static_cast<int>(strm->pathDuration()->source()));
    updateCrustalAmpSource(static_cast<int>(strm->crustalAmp()->source()));

    // Wrap the tab widget in a form layout to return the expected item
    auto formLayout = new QFormLayout;
    formLayout->addWidget(_paramsTabWidget);
    return formLayout;
}

void SourceTheoryRvtMotionDialog::updatePathDurSource(int source)
{
    auto s = static_cast<PathDurationModel::Source>(source);

    _pathDurTableGroupBox->setReadOnly(
                _readOnly || (s == PathDurationModel::Default));
}

void SourceTheoryRvtMotionDialog::updateCrustalAmpSource(int source)
{
    auto s = static_cast<CrustalAmplification::Source>(source);

    _crustalAmpGroupBox->setReadOnly(
                _readOnly
                || (s == CrustalAmplification::Default)
                || (s == CrustalAmplification::Calculated));
    _crustalModelGroupBox->setReadOnly(
                _readOnly
                || (s == CrustalAmplification::Default)
                || (s == CrustalAmplification::Specified));

    _paramsTabWidget->setTabEnabled(
                _crustModelIndex,
                s == CrustalAmplification::Calculated);
}
