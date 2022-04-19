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

#include "RatiosOutputCatalog.h"

#include "AbstractOutput.h"
#include "AbstractRatioOutput.h"
#include "AccelTransferFunctionOutput.h"
#include "Algorithms.h"
#include "SpectralRatioOutput.h"
#include "StrainTransferFunctionOutput.h"

#include <QDebug>

RatiosOutputCatalog::RatiosOutputCatalog(OutputCatalog *outputCatalog)
    : AbstractMutableOutputCatalog(outputCatalog) {
  _lookup["Acceleration Transfer Function"] = "AccelTransferFunctionOutput";
  _lookup["Spectral Ratio"] = "SpectralRatioOutput";
  _lookup["Strain Transfer Function"] = "StrainTransferFunctionOutput";
}

auto RatiosOutputCatalog::needsInputConditions() const -> bool { return true; }

auto RatiosOutputCatalog::needsOutputConditions() const -> bool { return true; }

auto RatiosOutputCatalog::rowCount(const QModelIndex &parent) const -> int {
  Q_UNUSED(parent);

  return _outputs.size();
}

auto RatiosOutputCatalog::columnCount(const QModelIndex &parent) const -> int {
  Q_UNUSED(parent);
  return 5;
}

auto RatiosOutputCatalog::data(const QModelIndex &index, int role) const
    -> QVariant {
  if (index.parent() != QModelIndex())
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
    case NameColumn:
      return _outputs.at(index.row())->name();
    case OutDepthColumn:
      if (role == Qt::DisplayRole) {
        return locationToString(_outputs.at(index.row())->outDepth());
      } else {
        return _outputs.at(index.row())->outDepth();
      }
    case OutTypeColumn:
      if (role == Qt::DisplayRole) {
        return AbstractMotion::typeList().at(
            _outputs.at(index.row())->outType());
      } else {
        return _outputs.at(index.row())->outType();
      }
    case InDepthColumn:
      if (role == Qt::DisplayRole) {
        return locationToString(_outputs.at(index.row())->inDepth());
      } else {
        return _outputs.at(index.row())->inDepth();
      }
    case InTypeColumn:
      if (role == Qt::DisplayRole) {
        return AbstractMotion::typeList().at(
            _outputs.at(index.row())->inType());
      } else {
        return _outputs.at(index.row())->inType();
      }
    default:
      return QVariant();
    }
  }

  return AbstractOutputCatalog::data(index, role);
}

auto RatiosOutputCatalog::setData(const QModelIndex &index,
                                  const QVariant &value, int role) -> bool {
  if (index.parent() != QModelIndex() || _readOnly)
    return false;

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
    case NameColumn:
      return false;
    case OutDepthColumn:
    case InDepthColumn: {
      bool ok;
      const double d = value.toDouble(&ok);

      if (ok) {
        if (index.column() == OutDepthColumn)
          _outputs[index.row()]->setOutDepth(d);
        else if (index.column() == InDepthColumn)
          _outputs[index.row()]->setInDepth(d);
      } else {
        return false;
      }
      break;
    }
    case OutTypeColumn:
    case InTypeColumn: {
      bool ok;
      AbstractMotion::Type type = AbstractMotion::variantToType(value, &ok);

      if (ok) {
        if (index.column() == OutTypeColumn)
          _outputs[index.row()]->setOutType(type);
        else if (index.column() == InTypeColumn)
          _outputs[index.row()]->setInType(type);
      } else
        return false;

      break;
    }
    default:
      return false;
    }
  }

  emit dataChanged(index, index);
  emit wasModified();

  return true;
}

auto RatiosOutputCatalog::headerData(int section, Qt::Orientation orientation,
                                     int role) const -> QVariant {
  if (role != Qt::DisplayRole)
    return QVariant();

  switch (orientation) {
  case Qt::Horizontal:
    switch (section) {
    case NameColumn:
      return tr("Name");
    case OutDepthColumn:
      return tr("Location 1");
    case OutTypeColumn:
      return tr("Type 1");
    case InDepthColumn:
      return tr("Location 2");
    case InTypeColumn:
      return tr("Type 2");
    }
  case Qt::Vertical:
    return section + 1;
  }

  return QVariant();
}

auto RatiosOutputCatalog::flags(const QModelIndex &index) const
    -> Qt::ItemFlags {
  Q_UNUSED(index);

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  switch (index.column()) {
  case OutDepthColumn:
  case OutTypeColumn:
  case InDepthColumn:
  case InTypeColumn:
    flags |= Qt::ItemIsEditable;
  case NameColumn:
    break;
  }

  return flags;
}

auto RatiosOutputCatalog::removeRows(int row, int count,
                                     const QModelIndex &parent) -> bool {
  if (!count)
    return false;
  emit beginRemoveRows(parent, row, row + count - 1);

  for (int i = 0; i < count; ++i) {
    auto *aro = _outputs.takeAt(row);

    if (aro->needsFreq()) {
      // Check if remaining outputs needs frequencies
      bool needsFreq = false;
      for (auto *_aro : _outputs) {
        if (_aro->needsFreq()) {
          needsFreq = true;
          break;
        }
      }

      if (!needsFreq)
        emit frequencyIsNeededChanged(needsFreq);

    } else if (aro->needsPeriod()) {
      // Check if remaining outputs needs period
      bool needsPeriod = false;
      for (AbstractRatioOutput *_aro : _outputs) {
        if (_aro->needsPeriod()) {
          needsPeriod = true;
          break;
        }
      }

      if (!needsPeriod)
        emit periodIsNeededChanged(needsPeriod);
    }

    aro->deleteLater();
  }

  emit endRemoveRows();
  emit wasModified();

  return true;
}

void RatiosOutputCatalog::addRow(const QString &name) {
  if (_lookup.contains(name)) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    _outputs << factory(_lookup.value(name), _outputCatalog);

    connect(_outputs.last(), SIGNAL(wasModified()), this,
            SIGNAL(wasModified()));

    endInsertRows();

    emit wasModified();
  }
}

auto RatiosOutputCatalog::outputs() const -> QList<AbstractOutput *> {
  QList<AbstractOutput *> list;

  for (AbstractRatioOutput *aro : _outputs)
    list << static_cast<AbstractOutput *>(aro);

  return list;
}

auto RatiosOutputCatalog::factory(const QString &className,
                                  OutputCatalog *parent) const
    -> AbstractRatioOutput * {
  AbstractRatioOutput *aro = 0;

  if (className == "AccelTransferFunctionOutput") {
    aro = new AccelTransferFunctionOutput(parent);
    emit frequencyIsNeededChanged(true);
  } else if (className == "SpectralRatioOutput") {
    aro = new SpectralRatioOutput(parent);
    emit periodIsNeededChanged(true);
  } else if (className == "StrainTransferFunctionOutput") {
    aro = new StrainTransferFunctionOutput(parent);
    emit frequencyIsNeededChanged(true);
  }
  Q_ASSERT(aro);

  return aro;
}

void RatiosOutputCatalog::fromJson(const QJsonArray &json) {
  beginResetModel();
  _outputs.clear();

  for (const QJsonValue &qjv : json) {
    QJsonObject qjo = qjv.toObject();
    AbstractRatioOutput *aro =
        factory(qjo["className"].toString(), _outputCatalog);
    aro->fromJson(qjo);
    _outputs << aro;
  }

  endResetModel();
}

auto RatiosOutputCatalog::toJson() const -> QJsonArray {
  QJsonArray json;
  for (auto *aro : _outputs) {
    json << aro->toJson();
  }

  return json;
}

auto operator<<(QDataStream &out, const RatiosOutputCatalog *roc)
    -> QDataStream & {
  out << (quint8)1;

  out << roc->_outputs.size();

  for (auto *aro : roc->_outputs)
    out << QString(aro->metaObject()->className()) << aro;

  return out;
}

auto operator>>(QDataStream &in, RatiosOutputCatalog *roc) -> QDataStream & {
  quint8 ver;
  in >> ver;

  int size;
  in >> size;

  roc->beginResetModel();
  QString name;
  while (roc->_outputs.size() < size) {
    in >> name;
    roc->_outputs << roc->factory(name, roc->_outputCatalog);
    in >> roc->_outputs.last();
  }
  roc->endResetModel();

  return in;
}
