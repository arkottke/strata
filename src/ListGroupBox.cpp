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

#include "ListGroupBox.h"

#include <QGridLayout>

ListGroupBox::ListGroupBox( QAbstractListModel * model, const QString & title, QWidget * parent )
    : QGroupBox( title, parent), m_model(model)
{
    QGridLayout * layout = new QGridLayout;

    // Add push button
    m_addButton = new QPushButton(QIcon(":/images/list-add.svg"), tr("Add"));
    connect( m_addButton, SIGNAL(clicked()), this, SLOT(addRow()));
    layout->addWidget( m_addButton, 0, 0);

    // Remove push button
    m_removeButton = new QPushButton(QIcon(":/images/list-remove.svg"), tr("Remove"));
    m_removeButton->setEnabled(false);
    connect( m_removeButton, SIGNAL(clicked()), this, SLOT(removeRow()));
    layout->addWidget( m_removeButton, 0, 1);

    // List view
    m_view = new QListView;
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setModel(m_model);

    connect( m_view->selectionModel(), SIGNAL(selectionChanged( QItemSelection, QItemSelection)),
            this, SLOT(selectionChanged()));

    layout->addWidget(m_view, 1, 0, 1, 3);

    setLayout(layout);
}

QListView * ListGroupBox::view()
{
    return m_view;
}

QAbstractListModel * ListGroupBox::model()
{
    return m_model;
}

void ListGroupBox::removeEnabled(bool b)
{
    m_removeButton->setEnabled(b);
}

void ListGroupBox::addRow()
{
	// Always add one layer at the end of the list
    m_model->insertRows( m_model->rowCount(), 1);
    // Signal that the data has changed
    emit dataChanged();
}

void ListGroupBox::removeRow()
{
    m_model->removeRows( m_view->currentIndex().row(), 1);
	// Update the remove buttons
    m_removeButton->setEnabled(false);
    
    // Notify that the row has been removed
    emit rowRemoved();
    // Signal that the data has changed
    emit dataChanged();
}

void ListGroupBox::selectionChanged()
{
    emit rowSelected( m_view->currentIndex() );
}
