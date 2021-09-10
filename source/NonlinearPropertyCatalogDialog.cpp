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

#include "NonlinearPropertyCatalogDialog.h"

#include "DampingFactory.h"
#include "ModulusFactory.h"
#include "NonlinearProperty.h"
#include "NonlinearPropertyFactoryGroupBox.h"
#include "MyTableView.h"

#include <QGridLayout>
#include <QTableView>
#include <QLabel>
#include <QDialogButtonBox>

#include <QDebug>


NonlinearPropertyCatalogDialog::NonlinearPropertyCatalogDialog(NonlinearPropertyCatalog *catalog, QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    auto * layout = new QGridLayout;

    // Shear modulus list
    auto *modulus = new NonlinearPropertyFactoryGroupBox(
            catalog->modulusFactory(),
            tr("Shear Modulus Reduction Models"), this);

    connect(modulus, SIGNAL(nlPropertyChanged(NonlinearProperty*, bool)),
            this, SLOT(setDataModel(NonlinearProperty*, bool)));

    layout->addWidget(modulus, 0, 0);

    // Damping list
    auto *damping = new NonlinearPropertyFactoryGroupBox(
            catalog->dampingFactory(),
            tr("Damping Models"), this);

    connect(damping, SIGNAL(nlPropertyChanged(NonlinearProperty*, bool)),
            this, SLOT(setDataModel(NonlinearProperty*, bool)));

    layout->addWidget(damping, 1, 0);

    connect(modulus, SIGNAL(nlPropertyChanged(NonlinearProperty*,bool)),
            damping, SLOT(clearSelection()));
    connect(damping, SIGNAL(nlPropertyChanged(NonlinearProperty*,bool)),
            modulus, SLOT(clearSelection()));

   // Data group box
   _dataGroupBox = new TableGroupBox(tr("Nonlinear Curve Data"));

   layout->addWidget(_dataGroupBox, 0, 1, 2, 2);

   // Information regard permanent models
   layout->addWidget( new QLabel(tr("Gray background indicates immutable model.")), 2, 0 );

   // Button box
   auto * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
   connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

   layout->addWidget( buttonBox, 2, 1);

   setLayout(layout);
   setWindowTitle(tr("Nonlinear Property Manager - Strata"));

   // Select the first index
   modulus->selectRow(0);
}

void NonlinearPropertyCatalogDialog::setDataModel(NonlinearProperty *np, bool readOnly)
{
    _dataGroupBox->setModel(np);
    _dataGroupBox->setReadOnly(readOnly);
}

