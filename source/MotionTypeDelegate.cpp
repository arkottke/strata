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

MotionTypeDelegate::MotionTypeDelegate(QObject *parent)
    : QItemDelegate(parent) {}

auto MotionTypeDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem & /*option*/,
                                      const QModelIndex & /*index*/) const
    -> QWidget * {
  auto *editor = new QComboBox(parent);
  // Populate the items once at editor creation; setEditorData() is called
  // again whenever the model emits dataChanged() while the editor is still
  // open, so adding the items there would duplicate them.
  editor->addItems(AbstractMotion::typeList());
  return editor;
}

void MotionTypeDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const {
  auto *comboBox = static_cast<QComboBox *>(editor);
  comboBox->setCurrentIndex(index.model()->data(index, Qt::EditRole).toInt());
}

void MotionTypeDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const {
  auto *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, comboBox->currentIndex());
  return;
}
