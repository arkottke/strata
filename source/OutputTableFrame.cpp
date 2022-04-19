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

#include "OutputTableFrame.h"

#include "AbstractMutableOutputCatalog.h"
#include "AbstractOutput.h"
#include "DepthComboBoxDelegate.h"
#include "MotionTypeDelegate.h"
#include "MyTableView.h"
#include "RatiosOutputCatalog.h"
#include "SpectraOutputCatalog.h"
#include "TimeSeriesOutputCatalog.h"

#include <QDebug>
#include <QGridLayout>
#include <QInputDialog>
#include <QTableWidget>

OutputTableFrame::OutputTableFrame(QWidget *parent) : QFrame(parent) {
  auto *layout = new QGridLayout;
  layout->setColumnStretch(2, 1);
  // Create the buttons
  _addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
  connect(_addButton, SIGNAL(clicked()), this, SLOT(add()));
  layout->addWidget(_addButton, 0, 0);

  _removeButton =
      new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
  connect(_removeButton, SIGNAL(clicked()), this, SLOT(remove()));
  layout->addWidget(_removeButton, 0, 1);

  // Create table
  _tableView = new MyTableView(this);
  _tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
  layout->addWidget(_tableView, 1, 0, 1, 3);

  setLayout(layout);
}

void OutputTableFrame::setModel(AbstractMutableOutputCatalog *amoc) {
  _outputCatalog = amoc;
  _tableView->setModel(_outputCatalog);
  updateButtons();
  connect(_tableView->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this,
          SLOT(updateButtons()));

  if (_outputCatalog->needsOutputConditions()) {
    _tableView->setItemDelegateForColumn(1, new DepthComboBoxDelegate);
    _tableView->setItemDelegateForColumn(2, new MotionTypeDelegate);
  }

  if (_outputCatalog->needsInputConditions()) {
    _tableView->setItemDelegateForColumn(3, new DepthComboBoxDelegate);
    _tableView->setItemDelegateForColumn(4, new MotionTypeDelegate);
  }
}

void OutputTableFrame::setReadOnly(bool readOnly) {
  _addButton->setHidden(readOnly);
  _removeButton->setHidden(readOnly);
  _tableView->setReadOnly(readOnly);
}

void OutputTableFrame::add() {
  bool ok;

  QString item = QInputDialog::getItem(this, tr("Output Selection - Strata"),
                                       "Select the output type to add",
                                       _outputCatalog->names(), 0, false, &ok);

  if (ok)
    _outputCatalog->addRow(item);
}

void OutputTableFrame::remove() {
  QModelIndexList selectedRows = _tableView->selectionModel()->selectedRows();
  _outputCatalog->removeRows(selectedRows.first().row(), selectedRows.size());
}

void OutputTableFrame::updateButtons() {
  _removeButton->setEnabled(
      _tableView->selectionModel()->selectedRows().size());
}
