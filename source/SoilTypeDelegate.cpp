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

#include "SoilTypeDelegate.h"

#include "SoilTypeCatalog.h"

#include <QComboBox>

SoilTypeDelegate::SoilTypeDelegate(QObject *parent) : QItemDelegate(parent) {}

void SoilTypeDelegate::setCatalog(SoilTypeCatalog *catalog) {
  _catalog = catalog;
}

auto SoilTypeDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
    -> QWidget * {
  Q_UNUSED(option);
  Q_UNUSED(index);

  auto *editor = new QComboBox(parent);
  return editor;
}

void SoilTypeDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const {
  auto *comboBox = static_cast<QComboBox *>(editor);

  comboBox->setModel(_catalog);
  comboBox->setCurrentIndex(
      comboBox->findText(index.model()->data(index, Qt::EditRole).toString()));
}

void SoilTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const {
  auto *comboBox = static_cast<QComboBox *>(editor);
  model->setData(index, comboBox->currentIndex());
}
