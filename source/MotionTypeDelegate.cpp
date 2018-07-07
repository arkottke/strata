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

#include "MotionTypeDelegate.h"

#include "AbstractMotion.h"

#include "QComboBox"

MotionTypeDelegate::MotionTypeDelegate(QObject *parent) :
        QItemDelegate(parent)
{
}

QWidget* MotionTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                          const QModelIndex & /*index*/) const
{
    QComboBox *editor = new QComboBox(parent);
    return editor;
}

void MotionTypeDelegate::setEditorData(QWidget *editor, const QModelIndex & index) const
{
    QComboBox * comboBox = static_cast<QComboBox*>(editor);
    comboBox->addItems(AbstractMotion::typeList());
    comboBox->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt());
}

void MotionTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QComboBox * comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentIndex());
    return;
}
