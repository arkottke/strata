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

#include "TableGroupBox.h"

#include <QDebug>

TableGroupBox::TableGroupBox( MyAbstractTableModel * model, const QString & title, QWidget * parent)
    : QGroupBox(title, parent), m_model(model)
{
    m_readOnly = true;
    m_lastRowFixed = false;
    // Create the buttons
    m_addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    m_insertButton = new QPushButton(tr("Insert"));
    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    // Disable the insert and remove buttons
    m_removeButton->setEnabled(false);
    m_insertButton->setEnabled(false);
    // Create the button row
    m_buttonRow = new QHBoxLayout;
    m_buttonRow->addWidget(m_addButton);
    m_buttonRow->addSpacing(5);
    m_buttonRow->addWidget(m_insertButton);
    m_buttonRow->addSpacing(5);
    m_buttonRow->addWidget(m_removeButton);
    m_buttonRow->addStretch(1);

    // Create the tableview
    m_table = new MyTableView;
    m_table->setModel(m_model);
    //m_table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::ContiguousSelection);

    // Create the layout of the group box
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addItem(m_buttonRow);
    layout->addWidget(m_table);

    setLayout(layout);

    // Form connections
    connect(m_addButton, SIGNAL(clicked()), this, SLOT(addRow()));
    connect(m_insertButton, SIGNAL(clicked()), this, SLOT(insertRow()));
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    //connect(m_table->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(cellSelected(QModelIndex,QModelIndex)));
    connect(m_table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(cellSelected()));

    connect(m_table, SIGNAL(dataPasted()), SIGNAL(dataChanged()));
    connect(m_table->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(dataChanged()));
    connect(m_table->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(dataChanged()));
    connect(m_table->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(dataChanged()));

    //connect(m_table->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_table, SLOT(resizeColumnsToContents()));
    //connect(m_table->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), m_table, SLOT(resizeRowsToContents()));
}

MyTableView * TableGroupBox::table()
{
    return m_table;
}

QAbstractTableModel * TableGroupBox::model()
{
    return m_model;
}

bool TableGroupBox::readOnly() const
{
    return m_readOnly;
}

bool TableGroupBox::lastRowFixed() const
{
    return m_lastRowFixed;
}

void TableGroupBox::setLastRowFixed(bool lastRowFixed)
{
    m_lastRowFixed = lastRowFixed;
}

void TableGroupBox::addButton(QPushButton * pushButton )
{
    m_addedButtons << pushButton;

    pushButton->setParent(this);
    m_buttonRow->addWidget(pushButton);
}

void TableGroupBox::addRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();
	// Always add one layer at the end of the list
    if ( selectedRows.isEmpty() )
        m_model->insertRows( m_model->rowCount(), 1);
    else
        m_model->insertRows( m_model->rowCount(), selectedRows.size());
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::insertRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();
    // Insert a row above the previously selected row -- row previously defined
    // by updateSelectedRow().
    m_model->insertRows( selectedRows.first().row(), selectedRows.size());
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::removeRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();

    m_model->removeRows( selectedRows.first().row(), selectedRows.size());
	// Update the insert and remove buttons
    m_insertButton->setEnabled(false);
    m_removeButton->setEnabled(false);
    
    // Notify that the row has been removed
    emit rowRemoved();
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::cellSelected()//const QModelIndex & /* current */, const QModelIndex & /*previous*/)
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();

    if ( !selectedRows.isEmpty() ) {
        // Enable the insert button if the entire row is selected
        m_insertButton->setEnabled(true);

        // If the last row is set to be fixed, then the remove button is
        // disabled for the final row.
        if ( m_lastRowFixed && selectedRows.last().row() == m_table->model()->rowCount() - 1 )
            m_removeButton->setEnabled(false);
        else
            m_removeButton->setEnabled(true);

    } else {
        // Disable the buttons
        m_insertButton->setEnabled(false);
        m_removeButton->setEnabled(false);
    }
}

void TableGroupBox::setReadOnly(bool readOnly)
{ 
    m_readOnly = readOnly;
    // Hide the buttons
    m_addButton->setHidden(readOnly);
    m_insertButton->setHidden(readOnly);
    m_removeButton->setHidden(readOnly);
    
    for (int i = 0; i < m_addedButtons.size(); ++i) {
        m_addedButtons.at(i)->setHidden(readOnly); 
    }
    // Set the table is readOnly
    m_model->setReadOnly(readOnly);
}
