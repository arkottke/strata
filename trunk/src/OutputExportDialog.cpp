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

#include "OutputExportDialog.h"
#include "OutputCsvSelectionTableModel.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QTableView>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>


OutputExportDialog::OutputExportDialog( SiteResponseOutput * model, QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_model(model)
{
    createDialog();
}

void OutputExportDialog::selectDirectory()
{
    QString destDir = QFileDialog::getExistingDirectory( this, 
            tr("Select the output directory"), m_destDirLineEdit->text() );

    if ( destDir.isEmpty() )
        return;
    else
        m_destDirLineEdit->setText(destDir);
}

void OutputExportDialog::exportData()
{
    // Check to see if the directory is valid
    QDir destDir( m_destDirLineEdit->text() );

    if ( !destDir.exists() ) {
        // Prompt for the creation
        int ret = QMessageBox::question( this, tr("Strata"), QString( tr(
                        "The directory '%1' does not exist.\n Do you want to "
                        "create this directory?")).arg(destDir.absolutePath()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );

        if (ret == QMessageBox::No)
            return;
        else 
            // Create the directory
            destDir.mkpath(".");
    }

    // Output to CSV
    m_model->exportData( destDir.path(), ",", m_prefixLineEdit->text() );
    
    accept();
}

void OutputExportDialog::createDialog()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 1, 1);

    // Path selection
    QPushButton * selectDirPushButton = new QPushButton(tr("Select directory..."));
    connect( selectDirPushButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));

    m_destDirLineEdit = new QLineEdit;
    m_destDirLineEdit->setText( QDir::currentPath() );

    layout->addWidget( selectDirPushButton, 0, 0);
    layout->addWidget( m_destDirLineEdit, 0, 1 );

    // Prefix
    m_prefixLineEdit = new QLineEdit( m_model->filePrefix() );
    layout->addWidget( new QLabel("Prefix:"), 1, 0 );
    layout->addWidget( m_prefixLineEdit, 1, 1 );

    // View of possible output
    QTableView * tableView = new QTableView;
    tableView->setModel( new OutputCsvSelectionTableModel( m_model->output()));
    tableView->resizeColumnsToContents();

    layout->addWidget( tableView, 2, 0, 1, 2);

    // Button box
    QDialogButtonBox * buttonBox = new QDialogButtonBox( 
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);

    connect( buttonBox, SIGNAL(accepted()), this, SLOT(exportData()));
    connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget( buttonBox, 3, 0, 1, 2);

    // Set the layout
    setLayout(layout);

    setModal(true);
}
