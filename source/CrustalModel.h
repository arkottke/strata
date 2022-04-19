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

#ifndef CRUSTALMODEL_H
#define CRUSTALMODEL_H

#include "MyAbstractTableModel.h"

#include <QDataStream>
#include <QJsonObject>
#include <QVector>

class CrustalModel : public MyAbstractTableModel {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const CrustalModel *cm)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, CrustalModel *cm) -> QDataStream &;

public:
  explicit CrustalModel(QObject *parent = 0);

  enum Columns { ThicknessColumn, VelocityColumn, DensityColumn };

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
  auto calculate(const QVector<double> &freq) const -> QVector<double>;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

signals:
  void wasModified();

private:
  /*! Compute the average value of a property to a max depth.
   * \param thickness vector of the layer thicknesses
   * \param property vector of the property of interest
   * \param maxDepth depth to which the average is computed
   * \return average value
   */
  static auto averageValue(const QVector<double> &thickness,
                           const QVector<double> &property, double maxDepth)
      -> double;

  //! Thickness of the crustal layers
  QVector<double> _thickness;

  //! Shear-wave velocity of the crustal layers in km/sec
  QVector<double> _velocity;

  //! Density of the crustal layers in grams/cm^3
  QVector<double> _density;
};

#endif // CRUSTALMODEL_H
