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

#include "OutputTableFrame.h"

#include "AbstractOutput.h"
#include "AbstractMutableOutputCatalog.h"
#include "DepthComboBoxDelegate.h"
#include "MyTableView.h"
#include "MotionTypeDelegate.h"
#include "RatiosOutputCatalog.h"
#include "SpectraOutputCatalog.h"
#include "TimeSeriesOutputCatalog.h"

#include <QDebug>
#include <QInputDialog>
#include <QGridLayout>
#include <QTableWidget>

OutputTableFrame::OutputTableFrame(QWidget *parent) :
    QFrame(parent)
{
    QGridLayout* layout = new QGridLayout;
    layout->setColumnStretch(2, 1);
    // Create the buttons
    m_addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(m_addButton, SIGNAL(clicked()), this, SLOT(add()));
    layout->addWidget(m_addButton, 0, 0);

    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(remove()));
    layout->addWidget(m_removeButton, 0, 1);

    // Create table
    m_tableView = new MyTableView(this);
    m_tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    layout->addWidget(m_tableView, 1, 0, 1, 3);

    setLayout(layout);
}


void OutputTableFrame::setModel(AbstractMutableOutputCatalog *amoc)
{
    m_outputCatalog = amoc;
    m_tableView->setModel(m_outputCatalog);
    updateButtons();
    connect(m_tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateButtons()));

    if (m_outputCatalog->needsOutputConditions()) {
        m_tableView->setItemDelegateForColumn(1, new DepthComboBoxDelegate);
        m_tableView->setItemDelegateForColumn(2, new MotionTypeDelegate);
    }

    if (m_outputCatalog->needsInputConditions()) {
        m_tableView->setItemDelegateForColumn(3, new DepthComboBoxDelegate);
        m_tableView->setItemDelegateForColumn(4, new MotionTypeDelegate);
    }
}

void OutputTableFrame::setReadOnly(bool readOnly)
{
    m_addButton->setHidden(readOnly);
    m_removeButton->setHidden(readOnly);
    m_tableView->setReadOnly(readOnly);
}


void OutputTableFrame::add()
{
    bool ok;

    QString item = QInputDialog::getItem(this, tr("Output Selection - Strata"),
                          "Select the output type to add",
                          m_outputCatalog->names(),
                          0, false, &ok);

    if (ok)
        m_outputCatalog->addRow(item);
}

void OutputTableFrame::remove()
{
    QModelIndexList selectedRows = m_tableView->selectionModel()->selectedRows();
    m_outputCatalog->removeRows(selectedRows.first().row(), selectedRows.size());
}

void OutputTableFrame::updateButtons()
{
    m_removeButton->setEnabled(m_tableView->selectionModel()->selectedRows().size());
}
