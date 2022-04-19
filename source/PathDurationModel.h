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

#ifndef PATHDURATIONMODEL_H
#define PATHDURATIONMODEL_H

#include "MyAbstractTableModel.h"

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>
#include <QVector>

class PathDurationModel : public MyAbstractTableModel {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const PathDurationModel *pdm)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, PathDurationModel *pdm)
      -> QDataStream &;

public:
  explicit PathDurationModel(QObject *parent = 0);

  enum Columns {
    DistanceColumn,
    RateColumn,
  };

  enum Source { Default, Specified };

  static auto sourceList() -> QStringList;

  void setRegion(AbstractRvtMotion::Region region);
  void setSource(Source source);
  auto source() const -> Source;

  //!@{ Methods for QAbstractTableModel
  virtual auto rowCount(const QModelIndex &parent) const -> int;

  virtual auto columnCount(const QModelIndex &parent) const -> int;

  virtual auto headerData(int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole) const -> QVariant;

  virtual auto data(const QModelIndex &index, int role = Qt::DisplayRole) const
      -> QVariant;
  virtual auto setData(const QModelIndex &index, const QVariant &value,
                       int role = Qt::EditRole) -> bool;

  virtual auto flags(const QModelIndex &index) const -> Qt::ItemFlags;

  virtual auto insertRows(int row, int count,
                          const QModelIndex &parent = QModelIndex()) -> bool;
  virtual auto removeRows(int row, int count,
                          const QModelIndex &parent = QModelIndex()) -> bool;
  //!@}

  /*! Compute the crustal amplification
   */
  auto duration(double distance) const -> double;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

signals:
  void wasModified();
  void sourceChanged(int source);

public slots:
  void setRegion(int region);
  void setSource(int source);

private:
  //! Model source
  Source _source;
  //! Distance (km)
  QVector<double> _distance;
  //! Duration rate (sec)
  QVector<double> _rate;
};

#endif // PATHDURATIONMODEL_H
