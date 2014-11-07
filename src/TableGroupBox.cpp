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

#include "MyTableView.h"
#include "MyAbstractTableModel.h"

#include <QDebug>

TableGroupBox::TableGroupBox(const QString & title, QWidget * parent)
    : QGroupBox(title, parent)
{
    m_lastRowFixed = false;

    m_buttonRow = new QHBoxLayout;
    const int spacing = 5;

    // Create the buttons
    m_addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(m_addButton, SIGNAL(clicked()), this, SLOT(addRow()));
    m_buttonRow->addWidget(m_addButton);
    m_buttonRow->addSpacing(spacing);

    m_insertButton = new QPushButton(tr("Insert"));
    m_insertButton->setEnabled(false);
    connect(m_insertButton, SIGNAL(clicked()), this, SLOT(insertRow()));
    m_buttonRow->addWidget(m_insertButton);
    m_buttonRow->addSpacing(spacing);

    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    m_removeButton->setEnabled(false);
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    m_buttonRow->addWidget(m_removeButton);
    m_buttonRow->addStretch(1);

    // Create the tableview
    m_table = new MyTableView;
    m_table->setSelectionMode(QAbstractItemView::ContiguousSelection);

    // Create the layout of the group box
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addItem(m_buttonRow);
    layout->addWidget(m_table);

    setLayout(layout);
}

void TableGroupBox::setModel(QAbstractTableModel *model)
{
    m_table->setModel(model);    
    m_table->resizeRowsToContents();
    m_table->resizeColumnsToContents();

    connect(m_table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(cellSelected()));
    connect(m_table->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SIGNAL(currentChanged(QModelIndex, QModelIndex)));
}

void TableGroupBox::setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate)
{
    m_table->setItemDelegateForColumn(column, delegate);
}

void TableGroupBox::setColumnHidden(int column, bool hide)
{
    m_table->setColumnHidden(column, hide);
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

MyTableView* const TableGroupBox::table()
{
    return m_table;
}

void TableGroupBox::addRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();
    // Always add one layer at the end of the list
    if (selectedRows.isEmpty()) {
        m_table->model()->insertRows(m_table->model()->rowCount(), 1);
    } else {
        m_table->model()->insertRows(m_table->model()->rowCount(), selectedRows.size());
    }
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::insertRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();
    // Insert a row above the previously selected row -- row previously defined
    // by updateSelectedRow().
    m_table->model()->insertRows(selectedRows.first().row(), selectedRows.size());
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::removeRow()
{
    QModelIndexList selectedRows = m_table->selectionModel()->selectedRows();

    m_table->model()->removeRows( selectedRows.first().row(), selectedRows.size());
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

    if (!selectedRows.isEmpty()) {
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
    m_table->setReadOnly(readOnly);
    // Hide the buttons
    m_addButton->setHidden(readOnly);
    m_insertButton->setHidden(readOnly);
    m_removeButton->setHidden(readOnly);

    foreach (QPushButton* button, m_addedButtons)
        button->setHidden(readOnly);
}
