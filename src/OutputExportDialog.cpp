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

#include "AbstractOutput.h"
#include "OutputCatalog.h"

#include <QDebug>

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTableWidgetItem>

OutputExportDialog::OutputExportDialog(OutputCatalog * model, QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f), m_model(model)
{
    createDialog();
}

void OutputExportDialog::selectDirectory()
{
    QSettings settings;
    // Prompt for a fileName
    QString dirName = QFileDialog::getExistingDirectory(
            this,
            tr("Select directory..."),
            settings.value("exportDirectory",
                           QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString());

    if (!dirName.isEmpty()) {
        settings.setValue("exportDirectory", dirName);
        m_destDirLineEdit->setText(dirName);
    }
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

    // Save the data to the model
    for (int i = 0; i < m_model->outputs().size(); ++i)
        m_model->outputs().at(i)->setExportEnabled(
                m_tableWidget->item(i, 0)->checkState());

    // Output to CSV
    m_model->exportData(destDir.path(), ",", m_prefixLineEdit->text() );

    // Save the path
    QSettings settings;
    settings.setValue( "outputExportDialog/path", destDir.absolutePath() );
    
    accept();
}

void OutputExportDialog::createDialog()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 1, 1);

    QSettings settings;

    // Path selection
    QPushButton * selectDirPushButton = new QPushButton(tr("Select directory..."), this);
    selectDirPushButton->setAutoDefault(false);
    connect( selectDirPushButton, SIGNAL(clicked()), this, SLOT(selectDirectory()));

    m_destDirLineEdit = new QLineEdit;

    m_destDirLineEdit->setText(
            settings.value(
                    "outputExportDialog/path",
                    QDesktopServices::storageLocation( QDesktopServices::DocumentsLocation )
                    ).toString());

    layout->addWidget( selectDirPushButton, 0, 0);
    layout->addWidget( m_destDirLineEdit, 0, 1 );

    // Prefix
    m_prefixLineEdit = new QLineEdit( m_model->filePrefix() );
    layout->addWidget( new QLabel("Prefix:"), 1, 0 );
    layout->addWidget( m_prefixLineEdit, 1, 1 );

    // View of possible output
    m_tableWidget = new QTableWidget(m_model->outputs().size(), 1, this);
    m_tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Output Name"));

    for (int i = 0; i < m_model->outputs().size(); ++i) {
        AbstractOutput* ao = m_model->outputs().at(i);

        QTableWidgetItem* item = new QTableWidgetItem(ao->fullName());

        item->setCheckState(ao->exportEnabled() ?
                            Qt::Checked : Qt::Unchecked);
        item->setFlags(Qt::ItemIsSelectable
                       | Qt::ItemIsUserCheckable
                       | Qt::ItemIsEnabled);

        m_tableWidget->setItem(i, 0, item);
    }

    m_tableWidget->resizeColumnsToContents();
    m_tableWidget->resizeRowsToContents();

    layout->addWidget(m_tableWidget, 2, 0, 1, 2);

    // Button box
    QDialogButtonBox * buttonBox = new QDialogButtonBox( 
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(exportData()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget( buttonBox, 3, 0, 1, 2);

    // Set the layout
    setLayout(layout);

    setModal(true);
}
