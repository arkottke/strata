///////////////////////////////////////////////////////////////////////////////
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

#include "ResponseSpectrum.h"

#include <Serialize.h>

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

ResponseSpectrum::ResponseSpectrum(QObject *parent)
    : MyAbstractTableModel(parent), _modified(false), _damping(0) {
  connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SIGNAL(wasModified()));
}

auto ResponseSpectrum::modified() const -> bool { return _modified; }

void ResponseSpectrum::setModified(bool modified) {
  _modified = modified;
  emit wasModified();
}

auto ResponseSpectrum::damping() const -> double { return _damping; }

void ResponseSpectrum::setDamping(double damping) {
  if (_damping != damping) {
    emit wasModified();
  }

  _damping = damping;
}

auto ResponseSpectrum::period() const -> const QVector<double> & {
  return _period;
}

void ResponseSpectrum::setPeriod(const QVector<double> &period) {
  beginResetModel();
  _period = period;
  _sa.clear();
  endResetModel();
}

auto ResponseSpectrum::sa() const -> const QVector<double> & { return _sa; }

void ResponseSpectrum::setSa(const QVector<double> &sa) {
  beginResetModel();
  _sa = sa;
  endResetModel();
}

void ResponseSpectrum::scaleBy(double scale) {
  for (int i = 0; i < _sa.size(); ++i) {
    _sa[i] *= scale;
  }

  emit dataChanged(index(0, SpecAccelColumn),
                   index(rowCount(), SpecAccelColumn));
  emit wasModified();
}

auto ResponseSpectrum::rowCount(const QModelIndex &parent) const -> int {
  Q_UNUSED(parent);

  return qMin(_period.size(), _sa.size());
}

auto ResponseSpectrum::columnCount(const QModelIndex &parent) const -> int {
  Q_UNUSED(parent);

  return 2;
}

auto ResponseSpectrum::headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant {
  if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole)
    return QVariant();

  switch (orientation) {
  case Qt::Horizontal:
    switch (section) {
    case PeriodColumn:
      return tr("Period (s)");
    case SpecAccelColumn:
      return tr("Spec. Accel. (g)");
    }
  case Qt::Vertical:
    return section + 1;
  default:
    return QVariant();
  }
}

auto ResponseSpectrum::data(const QModelIndex &index, int role) const
    -> QVariant {
  if (index.parent() != QModelIndex())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
    switch (index.column()) {
    case PeriodColumn:
      return QString::number(_period.at(index.row()));
    case SpecAccelColumn:
      return QString::number(_sa.at(index.row()));
    default:
      return QVariant();
    }
  }

  return QVariant();
}

auto ResponseSpectrum::setData(const QModelIndex &index, const QVariant &value,
                               int role) -> bool {
  if (index.parent() != QModelIndex())
    return false;

  if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
    switch (index.column()) {
    case PeriodColumn:
      _period[index.row()] = value.toDouble();
      break;
    case SpecAccelColumn:
      _sa[index.row()] = value.toDouble();
      break;
    default:
      return false;
    }
  } else
    return false;

  dataChanged(index, index);
  setModified(true);

  return true;
}

auto ResponseSpectrum::flags(const QModelIndex &index) const -> Qt::ItemFlags {
  return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

auto ResponseSpectrum::insertRows(int row, int count, const QModelIndex &parent)
    -> bool {
  emit beginInsertRows(parent, row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    _period.insert(row, 0);
    _sa.insert(row, 0);
  }

  emit endInsertRows();

  return true;
}

auto ResponseSpectrum::removeRows(int row, int count, const QModelIndex &parent)
    -> bool {
  emit beginRemoveRows(parent, row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    _period.remove(row);
    _sa.remove(row);
  }

  emit endRemoveRows();
  return true;
}

void ResponseSpectrum::fromJson(const QJsonObject &json) {
  _modified = json["modified"].toBool();
  _damping = json["damping"].toDouble();

  Serialize::toDoubleVector(json["period"], _period);
  Serialize::toDoubleVector(json["sa"], _sa);
}

auto ResponseSpectrum::toJson() const -> QJsonObject {
  QJsonObject json;
  json["modified"] = _modified;
  json["damping"] = _damping;
  json["period"] = Serialize::toJsonArray(_period);
  json["sa"] = Serialize::toJsonArray(_sa);

  return json;
}

auto operator<<(QDataStream &out, const ResponseSpectrum *rs) -> QDataStream & {
  out << (quint8)1;

  out << rs->_modified << rs->_damping << rs->_period << rs->_sa;

  return out;
}

auto operator>>(QDataStream &in, ResponseSpectrum *rs) -> QDataStream & {
  quint8 ver;
  in >> ver;

  in >> rs->_modified >> rs->_damping >> rs->_period >> rs->_sa;

  return in;
}
