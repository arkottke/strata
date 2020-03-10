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

#include "ConfiningStressDialog.h"
#include "ConfiningStressTableModel.h"
#include "Units.h"
#include "TableGroupBox.h"
#include "MyTableView.h"

#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

ConfiningStressDialog::ConfiningStressDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent,f)
{   
    connect( Units::instance(), SIGNAL(systemChanged(int)),
             this, SLOT(updateLabels()));

    auto * layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    int row = 0;

    // Water table depth
    _waterDepthSpinBox = new QDoubleSpinBox;
    _waterDepthSpinBox->setRange( 0, 5000);
    _waterDepthSpinBox->setDecimals(1);
    _waterDepthSpinBox->setSingleStep(5);
    _waterDepthSpinBox->setSuffix(" " + Units::instance()->length());


    layout->addWidget( new QLabel(tr("Depth to water table:")), row, 0);
    layout->addWidget( _waterDepthSpinBox, row, 1 );

    // Soil profile table
    auto * tableModel = new ConfiningStressTableModel;

    _waterDepthSpinBox->setValue( tableModel->waterTableDepth() );
    connect( _waterDepthSpinBox, SIGNAL(valueChanged(double)), tableModel, SLOT(setWaterTableDepth(double)));

    auto* tgb = new TableGroupBox(tr("Soil Profile"));
    tgb->setModel(tableModel);

    layout->addWidget(tgb, ++row, 0, 1, 2 );


    // Button box
    auto * buttonBox = new QDialogButtonBox( QDialogButtonBox::Close );
    connect( buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget( buttonBox, ++row, 0, 1, 2 );

    setLayout(layout);
}

void ConfiningStressDialog::updateLabels()
{
    _waterDepthSpinBox->setSuffix(" " + Units::instance()->length());
}
