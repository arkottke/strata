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

#include "NonlinearPropertyFactoryGroupBox.h"

#include <QDebug>
#include <QGridLayout>
#include <QIcon>
#include <QPushButton>

NonlinearPropertyFactoryGroupBox::NonlinearPropertyFactoryGroupBox(
        AbstractNonlinearPropertyFactory *model, const QString & title, QWidget *parent)
    : QGroupBox(title, parent), _model(model)
{
    auto * layout = new QGridLayout;

    // Add push button
    auto *pushButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(pushButton, SIGNAL(clicked()),
             this, SLOT(addRow()));
    layout->addWidget(pushButton, 0, 0);

    // Remove push button
    _removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    _removeButton->setEnabled(false);
    connect(_removeButton, SIGNAL(clicked()),
             this, SLOT(removeRow()));
    layout->addWidget(_removeButton, 0, 1);

    // List view
    _view = new QListView;
    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setModel(_model);

    connect(_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateCurrent(QModelIndex,QModelIndex)));
    connect(_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(modelsInserted(QModelIndex,int,int)));

    layout->addWidget(_view, 1, 0, 1, 3);
    setLayout(layout);
}

void NonlinearPropertyFactoryGroupBox::clearSelection()
{
    _view->selectionModel()->reset();
}

void NonlinearPropertyFactoryGroupBox::selectRow(int row)
{
    //_view->selectionModel()->setCurrentIndex(_model->index(row,0));
    _view->setCurrentIndex(_model->index(row,0));
}

void NonlinearPropertyFactoryGroupBox::addRow()
{
    // Always add one layer at the end of the list
    _model->insertRows(_model->rowCount(), 1);
}

void NonlinearPropertyFactoryGroupBox::removeRow()
{
    const int row = _view->currentIndex().row();
    // Select the previous row
    _view->setCurrentIndex(_model->index(row - 1, 0));

    _model->removeRows(row, 1);
}

void NonlinearPropertyFactoryGroupBox::updateCurrent(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    const bool readOnly = !(_model->flags(current) & Qt::ItemIsEditable);

    _removeButton->setDisabled(readOnly);

    emit nlPropertyChanged(_model->modelAt(current.row()), readOnly);
}

void NonlinearPropertyFactoryGroupBox::modelsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end);

    const QModelIndex &index = _model->index(start, 0, parent);

    // Select the current model
    _view->setCurrentIndex(index);
    _view->edit(index);
}
