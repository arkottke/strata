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

#ifndef CONFINING_STRESS_TABLE_MODEL_H_
#define CONFINING_STRESS_TABLE_MODEL_H_

#include "MyAbstractTableModel.h"

#include <QList>

struct Layer {
  double untWt;       //!< Unit weight
  double thick;       //!< Thickness
  double atRestCoeff; //!< At rest coefficient
  double mEffStress;  //!< Mean effective stress
};

//! Table model used to store the information used in the confining stress
//! calculation.

class ConfiningStressTableModel : public MyAbstractTableModel {
  Q_OBJECT

public:
  ConfiningStressTableModel(QObject *parent = nullptr);

  auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;
  auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int;

  auto data(const QModelIndex &index, int role = Qt::DisplayRole) const
      -> QVariant;
  auto setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) -> bool;

  auto headerData(int section, Qt::Orientation orientation,
                  int role = Qt::DisplayRole) const -> QVariant;
  auto flags(const QModelIndex &index) const -> Qt::ItemFlags;

  auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;
  auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
      -> bool;

  auto waterTableDepth() -> double;

public slots:
  void setWaterTableDepth(double depth);

protected slots:
  void computeStress(int layer = 0);
  void updateHeader();

protected:
  //! Layers
  QList<Layer *> _layers;

  //! Depth to the water table
  double _waterTableDepth;
};
#endif
