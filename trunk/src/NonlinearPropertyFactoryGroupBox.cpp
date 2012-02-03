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

#include "NonlinearPropertyFactoryGroupBox.h"

#include <QDebug>
#include <QGridLayout>
#include <QIcon>
#include <QPushButton>

NonlinearPropertyFactoryGroupBox::NonlinearPropertyFactoryGroupBox(
        AbstractNonlinearPropertyFactory *model, const QString & title, QWidget *parent)
    : QGroupBox(title, parent), m_model(model)
{
    QGridLayout * layout = new QGridLayout;

    // Add push button
    QPushButton *pushButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect(pushButton, SIGNAL(clicked()),
             this, SLOT(addRow()));
    layout->addWidget(pushButton, 0, 0);

    // Remove push button
    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    m_removeButton->setEnabled(false);
    connect(m_removeButton, SIGNAL(clicked()),
             this, SLOT(removeRow()));
    layout->addWidget(m_removeButton, 0, 1);

    // List view
    m_view = new QListView;
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setModel(m_model);

    connect(m_view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateCurrent(QModelIndex,QModelIndex)));
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(modelsInserted(QModelIndex,int,int)));

    layout->addWidget(m_view, 1, 0, 1, 3);
    setLayout(layout);
}

void NonlinearPropertyFactoryGroupBox::clearSelection()
{
    m_view->selectionModel()->reset();
}

void NonlinearPropertyFactoryGroupBox::selectRow(int row)
{
    //m_view->selectionModel()->setCurrentIndex(m_model->index(row,0));
    m_view->setCurrentIndex(m_model->index(row,0));
}

void NonlinearPropertyFactoryGroupBox::addRow()
{
    // Always add one layer at the end of the list
    m_model->insertRows(m_model->rowCount(), 1);
}

void NonlinearPropertyFactoryGroupBox::removeRow()
{
    const int row = m_view->currentIndex().row();
    // Select the previous row
    m_view->setCurrentIndex(m_model->index(row - 1, 0));

    m_model->removeRows(row, 1);
}

void NonlinearPropertyFactoryGroupBox::updateCurrent(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    const bool readOnly = !(m_model->flags(current) & Qt::ItemIsEditable);

    m_removeButton->setDisabled(readOnly);

    emit nlPropertyChanged(m_model->modelAt(current.row()), readOnly);
}

void NonlinearPropertyFactoryGroupBox::modelsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end);

    const QModelIndex &index = m_model->index(start, 0, parent);

    // Select the current model
    m_view->setCurrentIndex(index);
    m_view->edit(index);
}
