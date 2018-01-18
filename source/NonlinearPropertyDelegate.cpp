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

#include "NonlinearPropertyDelegate.h"

#include <QComboBox>

#include "AbstractNonlinearPropertyFactory.h"

NonlinearPropertyDelegate::NonlinearPropertyDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void NonlinearPropertyDelegate::setModel(AbstractNonlinearPropertyFactory* factory)
{
    _factory = factory;
}

QWidget* NonlinearPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QComboBox *editor = new QComboBox(parent);
    return editor;
}

void NonlinearPropertyDelegate::setEditorData(QWidget *editor, const QModelIndex & index) const
{
    QComboBox * comboBox = static_cast<QComboBox*>(editor);

    if (_factory) {
        comboBox->setModel(_factory);

        comboBox->setCurrentIndex(
                comboBox->findText(index.model()->data(index, Qt::EditRole).toString()));
    }
}

void NonlinearPropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                const QModelIndex &index) const
{
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentIndex());
}
