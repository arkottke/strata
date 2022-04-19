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

#ifndef ABSTRACT_NONLINEAR_PROPERTY_FACTORY_H
#define ABSTRACT_NONLINEAR_PROPERTY_FACTORY_H

#include "NonlinearProperty.h"

#include <QAbstractListModel>
#include <QDataStream>
#include <QJsonObject>

class AbstractNonlinearPropertyFactory : public QAbstractListModel {
  Q_OBJECT

  friend auto operator<<(QDataStream &out,
                         const AbstractNonlinearPropertyFactory &anpf)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in,
                         AbstractNonlinearPropertyFactory &anpf)
      -> QDataStream &;

public:
  AbstractNonlinearPropertyFactory(QObject *parent = nullptr);
  ~AbstractNonlinearPropertyFactory();

  auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;

  auto data(const QModelIndex &index, int role = Qt::DisplayRole) const
      -> QVariant;
  auto setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) -> bool;

  auto flags(const QModelIndex &index) const -> Qt::ItemFlags;
  auto headerData(int section, Qt::Orientation orientation,
                  int role = Qt::DisplayRole) const -> QVariant;

  auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;
  auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;

  //! Return the model at the specified row
  auto modelAt(int row) const -> NonlinearProperty *;

  //! Return a duplicate of the model at the specified row
  auto duplicateAt(int row) const -> NonlinearProperty *;

  //! Tries the QVariant as an integer and a QString to match a possible model
  auto duplicateAt(QVariant value) const -> NonlinearProperty *;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

protected:
  //! Type of models the factor produces
  NonlinearProperty::Type _type;

  QList<NonlinearProperty *> _models;
};

#endif // ABSTRACTNONLINEARPROPERTYFACTORY_H
