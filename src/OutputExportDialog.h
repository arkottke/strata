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

#ifndef OUTPUT_EXPORT_DIALOG_H_
#define OUTPUT_EXPORT_DIALOG_H_

#include "SiteResponseOutput.h"

#include <QDialog>
#include <QLineEdit>

class OutputExportDialog : public QDialog
{
    Q_OBJECT

    public:
        OutputExportDialog( SiteResponseOutput * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    protected slots:
        void selectDirectory();

        void exportData();

    private:
        //! Site response model information
        SiteResponseOutput * m_model;

        //! Selected destination directory
        QLineEdit * m_destDirLineEdit;

        //! Selected fileName prefix
        QLineEdit * m_prefixLineEdit;

        //! Create the page
        void createDialog();
};
#endif
