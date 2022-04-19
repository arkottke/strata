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

#ifndef CUSTOM_NONLINEAR_PROPERTY_H
#define CUSTOM_NONLINEAR_PROPERTY_H

#include "NonlinearProperty.h"

class CustomNonlinearProperty : public NonlinearProperty {
  Q_OBJECT

public:
  CustomNonlinearProperty(Type type, bool retain = false,
                          QObject *parent = nullptr);

  void setName(const QString &name);
  auto retain() const -> bool;

  auto setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) -> bool;
  auto flags(const QModelIndex &index) const -> Qt::ItemFlags;
  auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;
  auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;

protected:
  //! If the curve should be saved to disk in the catalog
  bool _retain;
};

#endif // CUSTOM_NONLINEAR_PROPERTY_H
