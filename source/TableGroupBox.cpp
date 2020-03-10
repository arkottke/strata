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

#include "TableGroupBox.h"

#include "MyTableView.h"
#include "MyAbstractTableModel.h"

#include <QDebug>

TableGroupBox::TableGroupBox(const QString & title, QWidget * parent)
    : QGroupBox(title, parent)
{
    _lastRowFixed = false;

    _buttonRow = new QHBoxLayout;
    const int spacing = 5;

    // Create the buttons
    _addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(_addButton, SIGNAL(clicked()), this, SLOT(addRow()));
    _buttonRow->addWidget(_addButton);
    _buttonRow->addSpacing(spacing);

    _insertButton = new QPushButton(tr("Insert"));
    _insertButton->setEnabled(false);
    connect(_insertButton, SIGNAL(clicked()), this, SLOT(insertRow()));
    _buttonRow->addWidget(_insertButton);
    _buttonRow->addSpacing(spacing);

    _removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    _removeButton->setEnabled(false);
    connect(_removeButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    _buttonRow->addWidget(_removeButton);
    _buttonRow->addStretch(1);

    // Create the tableview
    _table = new MyTableView;
    _table->setSelectionMode(QAbstractItemView::ContiguousSelection);

    // Create the layout of the group box
    auto * layout = new QVBoxLayout;
    layout->addItem(_buttonRow);
    layout->addWidget(_table);

    setLayout(layout);
}

void TableGroupBox::setModel(QAbstractTableModel *model)
{
    _table->setModel(model);    
    _table->resizeRowsToContents();
    _table->resizeColumnsToContents();

    connect(_table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(cellSelected()));
    connect(_table->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SIGNAL(currentChanged(QModelIndex, QModelIndex)));
}

void TableGroupBox::setItemDelegateForColumn(int column, QAbstractItemDelegate* delegate)
{
    _table->setItemDelegateForColumn(column, delegate);
}

void TableGroupBox::setColumnHidden(int column, bool hide)
{
    _table->setColumnHidden(column, hide);
}

auto TableGroupBox::lastRowFixed() const -> bool
{
    return _lastRowFixed;
}

void TableGroupBox::setLastRowFixed(bool lastRowFixed)
{
    _lastRowFixed = lastRowFixed;
}

void TableGroupBox::addButton(QPushButton * pushButton )
{
    _addedButtons << pushButton;

    pushButton->setParent(this);
    _buttonRow->addWidget(pushButton);
}

auto TableGroupBox::table() -> MyTableView*
{
    return _table;
}

void TableGroupBox::addRow()
{
    QModelIndexList selectedRows = _table->selectionModel()->selectedRows();
    // Always add one layer at the end of the list
    if (selectedRows.isEmpty()) {
        _table->model()->insertRows(_table->model()->rowCount(), 1);
    } else {
        _table->model()->insertRows(_table->model()->rowCount(), selectedRows.size());
    }
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::insertRow()
{
    QModelIndexList selectedRows = _table->selectionModel()->selectedRows();
    // Insert a row above the previously selected row -- row previously defined
    // by updateSelectedRow().
    _table->model()->insertRows(selectedRows.first().row(), selectedRows.size());
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::removeRow()
{
    QModelIndexList selectedRows = _table->selectionModel()->selectedRows();

    _table->model()->removeRows( selectedRows.first().row(), selectedRows.size());
	// Update the insert and remove buttons
    _insertButton->setEnabled(false);
    _removeButton->setEnabled(false);
    
    // Notify that the row has been removed
    emit rowRemoved();
    // Signal that the data has changed
    emit dataChanged();
}

void TableGroupBox::cellSelected()//const QModelIndex & /* current */, const QModelIndex & /*previous*/)
{
    QModelIndexList selectedRows = _table->selectionModel()->selectedRows();

    if (!selectedRows.isEmpty()) {
        // Enable the insert button if the entire row is selected
        _insertButton->setEnabled(true);

        // If the last row is set to be fixed, then the remove button is
        // disabled for the final row.
        if ( _lastRowFixed && selectedRows.last().row() == _table->model()->rowCount() - 1 )
            _removeButton->setEnabled(false);
        else
            _removeButton->setEnabled(true);

    } else {
        // Disable the buttons
        _insertButton->setEnabled(false);
        _removeButton->setEnabled(false);
    }
}

void TableGroupBox::setReadOnly(bool readOnly)
{ 
    _table->setReadOnly(readOnly);

    for (auto *button : {_addButton, _insertButton, _removeButton})
        button->setHidden(readOnly);

    for (auto *button : _addedButtons)
        button->setHidden(readOnly);
}
