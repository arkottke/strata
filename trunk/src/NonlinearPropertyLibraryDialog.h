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

#ifndef NONLINEAR_PROPERTY_LIBRARY_DIALOG_H_
#define NONLINEAR_PROPERTY_LIBRARY_DIALOG_H_

#include <QDialog>
#include <QPushButton>

#include "NonlinearProperty.h"
#include "NonlinearPropertyLibrary.h"
#include "NonlinearPropertyLibraryTableModel.h"
#include "TableGroupBox.h"
#include "ListGroupBox.h"

class NonlinearPropertyLibraryDialog : public QDialog
{
    Q_OBJECT

    public:
        NonlinearPropertyLibraryDialog( NonlinearPropertyLibrary * library, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    protected slots:
        void rowSelected(const QModelIndex & current);

    private:
        //! Library of nonlinear curves
        NonlinearPropertyLibrary * m_library;

        //! Selected nonlinear curve
        NonlinearProperty * m_selectedProperty;

        //! List group of the shear modulus models
        ListGroupBox * m_modulusGroupBox;

        //! List group of the damping models
        ListGroupBox * m_dampingGroupBox;

        //! Table showing the current model
        TableGroupBox * m_dataGroupBox;
};
#endif
