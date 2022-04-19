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

#include "MyAbstractTableModel.h"

#include <QBrush>
#include <QColor>

MyAbstractTableModel::MyAbstractTableModel(QObject *parent)
    : QAbstractTableModel(parent), _readOnly(false) {}

auto MyAbstractTableModel::data(const QModelIndex &index, int role) const
    -> QVariant {
  // Color the background light gray for cells that are not editable
  if (role == Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable ||
                                      flags(index) & Qt::ItemIsUserCheckable))
    return QVariant(QBrush(QColor(200, 200, 200)));

  return QVariant();
}

auto MyAbstractTableModel::readOnly() const -> bool { return _readOnly; }

void MyAbstractTableModel::setReadOnly(bool readOnly) {
  if (_readOnly != readOnly) {
    beginResetModel();
    _readOnly = readOnly;
    endResetModel();
  }
}
