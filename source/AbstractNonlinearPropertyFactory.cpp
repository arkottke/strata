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

#include "AbstractNonlinearPropertyFactory.h"

#include "CustomNonlinearProperty.h"
#include "DarendeliNonlinearProperty.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QJsonArray>

AbstractNonlinearPropertyFactory::AbstractNonlinearPropertyFactory(
    QObject *parent)
    : QAbstractListModel(parent) {}

AbstractNonlinearPropertyFactory::~AbstractNonlinearPropertyFactory() {}

auto AbstractNonlinearPropertyFactory::rowCount(const QModelIndex &parent) const
    -> int {
  Q_UNUSED(parent);

  return _models.size();
}

auto AbstractNonlinearPropertyFactory::data(const QModelIndex &index,
                                            int role) const -> QVariant {
  if (index.parent() != QModelIndex())
    return QVariant();

  // Color the background light gray for cells that are not editable
  if (role == Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
    return QBrush(QColor(200, 200, 200));

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    return _models.at(index.row())->name();
  }

  return QVariant();
}

auto AbstractNonlinearPropertyFactory::setData(const QModelIndex &index,
                                               const QVariant &value, int role)
    -> bool {
  if (index.parent() != QModelIndex() && role != Qt::EditRole)
    return false;

  auto *cnp = qobject_cast<CustomNonlinearProperty *>(_models.at(index.row()));

  if (cnp) {
    cnp->setName(value.toString());
    dataChanged(index, index);
    return true;
  } else {
    return false;
  }
}

auto AbstractNonlinearPropertyFactory::flags(const QModelIndex &index) const
    -> Qt::ItemFlags {
  if (index.row() < 2 ||
      !qobject_cast<const CustomNonlinearProperty *>(_models.at(index.row()))) {
    return QAbstractItemModel::flags(index);
  } else {
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  }
}

auto AbstractNonlinearPropertyFactory::headerData(int section,
                                                  Qt::Orientation orientation,
                                                  int role) const -> QVariant {
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (orientation) {
  case Qt::Horizontal:
    return tr("Model Name");
  case Qt::Vertical:
    return section + 1;
  }

  return QVariant();
}

auto AbstractNonlinearPropertyFactory::insertRows(int row, int count,
                                                  const QModelIndex &parent)
    -> bool {
  if (!count)
    return false;

  emit beginInsertRows(parent, row, row + count - 1);

  for (int i = 0; i < count; ++i)
    _models.insert(row, new CustomNonlinearProperty(_type, true));

  emit endInsertRows();
  return true;
}

auto AbstractNonlinearPropertyFactory::removeRows(int row, int count,
                                                  const QModelIndex &parent)
    -> bool {
  if (!count)
    return false;

  for (int i = 0; i < count; ++i) {
    if (flags(index(row)) & Qt::ItemIsEditable) {
      emit beginRemoveRows(parent, row, row);
      _models.takeAt(row)->deleteLater();
      emit endRemoveRows();
    } else {
      return false;
    }
  }
  return true;
}

auto AbstractNonlinearPropertyFactory::modelAt(int row) const
    -> NonlinearProperty * {
  return _models.at(row);
}

auto AbstractNonlinearPropertyFactory::duplicateAt(int row) const
    -> NonlinearProperty * {
  if (row == 0) {
    return new CustomNonlinearProperty(_type, false);
  } else if (row == 1) {
    return new DarendeliNonlinearProperty(_type);
  } else {
    return _models.at(row)->duplicate();
  }
}

auto AbstractNonlinearPropertyFactory::duplicateAt(QVariant value) const
    -> NonlinearProperty * {
  int i = -1;
  if (value.type() == QVariant::Int) {
    i = value.toInt();
  } else if (value.type() == QVariant::String) {
    // Strings might come from the clipboard and actually be integers
    QString s = value.toString();

    bool ok;
    i = s.toInt(&ok);

    if (!ok) {
      for (int j = 0; j < _models.size(); ++j) {
        if (_models.at(j)->name() == s) {
          i = j;
          break;
        }
      }
    }
  }

  if (0 <= i && i < rowCount()) {
    return duplicateAt(i);
  } else {
    return 0;
  }
}

void AbstractNonlinearPropertyFactory::fromJson(const QJsonObject &json) {
  for (const QJsonValue &v : json["models"].toArray()) {
    auto *np = new CustomNonlinearProperty(_type, true);
    np->fromJson(v.toObject());
    _models << np;
  }
}

auto AbstractNonlinearPropertyFactory::toJson() const -> QJsonObject {
  QJsonArray models;
  for (auto *np : _models) {
    const CustomNonlinearProperty *cnp =
        qobject_cast<CustomNonlinearProperty *>(np);
    if (cnp && cnp->retain()) {
      models.append(np->toJson());
    }
  }

  QJsonObject json;
  json["models"] = models;
  return json;
}

auto operator<<(QDataStream &out, const AbstractNonlinearPropertyFactory &anpf)
    -> QDataStream & {
  out << (quint8)1;

  // Create a list of models that need to be saved
  QList<NonlinearProperty *> models;
  for (auto *np : anpf._models) {
    const auto *cnp = qobject_cast<const CustomNonlinearProperty *>(np);

    if (cnp && cnp->retain()) {
      models << np;
    }
  }

  // Save the data
  out << models;

  return out;
}

auto operator>>(QDataStream &in, AbstractNonlinearPropertyFactory &anpf)
    -> QDataStream & {
  quint8 ver;
  in >> ver;

  int size;
  in >> size;

  for (int i = 0; i < size; ++i) {
    auto *cnp = new CustomNonlinearProperty(anpf._type, true);
    in >> cnp;
    anpf._models << cnp;
  }

  return in;
}
