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

#include "CustomNonlinearProperty.h"

CustomNonlinearProperty::CustomNonlinearProperty(Type type, bool retain,
                                                 QObject *parent)
    : NonlinearProperty(parent), _retain(retain) {
  _name = "Custom";
  _type = type;
}

void CustomNonlinearProperty::setName(const QString &name) { _name = name; }

auto CustomNonlinearProperty::retain() const -> bool { return _retain; }

auto CustomNonlinearProperty::setData(const QModelIndex &index,
                                      const QVariant &value, int role) -> bool {
  if (index.parent() != QModelIndex() && role != Qt::EditRole) {
    return false;
  }

  bool b;
  const double d = value.toDouble(&b);

  if (b) {
    switch (index.column()) {
    case StrainColumn:
      _strain[index.row()] = d;
      break;
    case PropertyColumn:
      _average[index.row()] = d;
      _varied[index.row()] = d;
      gsl_interp_accel_reset(_acc);
      break;
    }
    dataChanged(index, index);
    return true;
  } else {
    return false;
  }
}

auto CustomNonlinearProperty::flags(const QModelIndex &index) const
    -> Qt::ItemFlags {
  return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

auto CustomNonlinearProperty::insertRows(int row, int count,
                                         const QModelIndex &parent) -> bool {
  if (!count)
    return false;

  emit beginInsertRows(parent, row, row + count - 1);

  _strain.insert(row, count, 0);
  _average.insert(row, count, 0);
  _varied.insert(row, count, 0);

  emit endInsertRows();
  return true;
}

auto CustomNonlinearProperty::removeRows(int row, int count,
                                         const QModelIndex &parent) -> bool {
  if (!count)
    return false;

  emit beginRemoveRows(parent, row, row + count - 1);

  _strain.remove(row, count);
  _average.remove(row, count);
  _varied.remove(row, count);

  emit endRemoveRows();
  return true;
}
