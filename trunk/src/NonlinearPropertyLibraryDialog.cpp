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

#include "NonlinearPropertyLibraryDialog.h"
#include "NonlinearPropertyListModel.h"
#include "NonlinearPropertyTableModel.h"

#include <QGridLayout>
#include <QTableView>
#include <QLabel>
#include <QDialogButtonBox>

#include <QDebug>
        
NonlinearPropertyLibraryDialog::NonlinearPropertyLibraryDialog( NonlinearPropertyLibrary * library, QWidget * parent, Qt::WindowFlags f )
    : QDialog( parent, f), m_library(library), m_selectedProperty(0)
{
    QGridLayout * layout = new QGridLayout;

    // Shear modulus list
    m_modulusGroupBox = new ListGroupBox( 
            new NonlinearPropertyListModel( m_library->modulus() ), 
            tr("Normalized Shear Modulus Models"));

    connect( m_modulusGroupBox, SIGNAL(rowSelected(QModelIndex)), this, SLOT(rowSelected(QModelIndex)));

    layout->addWidget( m_modulusGroupBox, 0, 0);

    // Damping list
    m_dampingGroupBox = new ListGroupBox( 
            new NonlinearPropertyListModel( m_library->damping() ), 
            tr("Damping Models"));

    connect( m_dampingGroupBox, SIGNAL(rowSelected(QModelIndex)), this, SLOT(rowSelected(QModelIndex)));

    layout->addWidget( m_dampingGroupBox, 1, 0);

   // Data group box
   m_dataGroupBox = new TableGroupBox( new NonlinearPropertyTableModel, tr("Nonlinear Curve Data"));

   layout->addWidget( m_dataGroupBox, 0, 1, 2, 2);
  
   // Information regard permanent models
   layout->addWidget( new QLabel(tr("* Indicates model can not be removed or edited.")), 2, 0 );

   // Button box
   QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
   connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

   layout->addWidget( buttonBox, 2, 1);

   setLayout( layout );

   setWindowTitle(tr("Nonlinear Property Manager - Strata"));
}

void NonlinearPropertyLibraryDialog::rowSelected(const QModelIndex &current)
{
    NonlinearPropertyListModel * model = 0;

    if ( current.model() == m_modulusGroupBox->model() )
        model =
            static_cast<NonlinearPropertyListModel*>(m_modulusGroupBox->model());

    else
        model =
            static_cast<NonlinearPropertyListModel*>(m_dampingGroupBox->model());
  
    // Grab the selected property
    m_selectedProperty = model->nlPropertyAt(current);

    // Update the group boxes
    if ( current.model() == m_modulusGroupBox->model() ) {
        m_dampingGroupBox->view()->selectionModel()->clearSelection();
        m_dampingGroupBox->removeEnabled(false);

        if ( m_selectedProperty->source() == NonlinearProperty::UserDefined )
            m_modulusGroupBox->removeEnabled(true);
        else
            m_modulusGroupBox->removeEnabled(false);
    } else {
        m_modulusGroupBox->view()->selectionModel()->clearSelection();
        m_modulusGroupBox->removeEnabled(false);

        if ( m_selectedProperty->source() == NonlinearProperty::UserDefined )
            m_dampingGroupBox->removeEnabled(true);
        else
            m_dampingGroupBox->removeEnabled(false);
    }

    // Set the model of the data group box
    static_cast<NonlinearPropertyTableModel*>(m_dataGroupBox->model())->setNonlinearProperty(m_selectedProperty);

    if ( m_selectedProperty->source() == NonlinearProperty::UserDefined )
        m_dataGroupBox->setReadOnly(false);
    else
        m_dataGroupBox->setReadOnly(true);
}
