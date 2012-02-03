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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef NONLINEAR_PROPERTY_CATALOG_DIALOG_H_
#define NONLINEAR_PROPERTY_CATALOG_DIALOG_H_

#include "NonlinearProperty.h"
#include "NonlinearPropertyCatalog.h"

#include "TableGroupBox.h"

#include <QDialog>
#include <QGridLayout>
#include <QListView>
#include <QPushButton>


class NonlinearPropertyCatalogDialog : public QDialog
{
    Q_OBJECT

public:
    NonlinearPropertyCatalogDialog(NonlinearPropertyCatalog* const catalog, QWidget * parent = 0, Qt::WindowFlags f = 0);

protected slots:
    void setDataModel(NonlinearProperty *np, bool readOnly);

protected:    
    //! Table showing the current model
    TableGroupBox *m_dataGroupBox;
};

#endif
