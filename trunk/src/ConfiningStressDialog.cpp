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

    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(0, 1);
    int row = 0;

    // Water table depth
    m_waterDepthSpinBox = new QDoubleSpinBox;
    m_waterDepthSpinBox->setRange( 0, 5000);
    m_waterDepthSpinBox->setDecimals(1);
    m_waterDepthSpinBox->setSingleStep(5);
    m_waterDepthSpinBox->setSuffix(" " + Units::instance()->length());


    layout->addWidget( new QLabel(tr("Depth to water table:")), row, 0);
    layout->addWidget( m_waterDepthSpinBox, row, 1 );

    // Soil profile table
    ConfiningStressTableModel * tableModel = new ConfiningStressTableModel;

    m_waterDepthSpinBox->setValue( tableModel->waterTableDepth() );
    connect( m_waterDepthSpinBox, SIGNAL(valueChanged(double)), tableModel, SLOT(setWaterTableDepth(double)));

    TableGroupBox* tgb = new TableGroupBox(tr("Soil Profile"));
    tgb->setModel(tableModel);

    layout->addWidget(tgb, ++row, 0, 1, 2 );


    // Button box
    QDialogButtonBox * buttonBox = new QDialogButtonBox( QDialogButtonBox::Close );
    connect( buttonBox, SIGNAL(rejected()), SLOT(reject()));
    layout->addWidget( buttonBox, ++row, 0, 1, 2 );

    setLayout(layout);
}

void ConfiningStressDialog::updateLabels()
{
    m_waterDepthSpinBox->setSuffix(" " + Units::instance()->length());
}
