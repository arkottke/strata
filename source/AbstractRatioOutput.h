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

#ifndef ABSTRACT_RATIO_OUTPUT_H
#define ABSTRACT_RATIO_OUTPUT_H

#include "AbstractMotion.h"
#include "AbstractOutput.h"

#include <QDataStream>
#include <QJsonObject>

class OutputStatistics;

class AbstractRatioOutput : public AbstractOutput {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const AbstractRatioOutput *aro)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, AbstractRatioOutput *aro)
      -> QDataStream &;

public:
  explicit AbstractRatioOutput(OutputCatalog *catalog);

  virtual auto fullName() const -> QString;

  auto inDepth() const -> double;
  auto inType() const -> AbstractMotion::Type;
  void setInType(AbstractMotion::Type inType);

  auto outDepth() const -> double;
  auto outType() const -> AbstractMotion::Type;
  void setOutType(AbstractMotion::Type outType);

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

public slots:
  void setInDepth(double inDepth);
  void setInType(int inType);

  void setOutDepth(double outDepth);
  void setOutType(int outType);

signals:
  void inDepthChanged(double inDepth);
  void inTypeChanged(int inType);

  void outDepthChanged(double outDepth);
  void outTypeChanged(int outType);

protected:
  virtual auto fileName(int motion = 0) const -> QString;
  virtual auto prefix() const -> const QString;

  //! Input depth
  double _inDepth;

  //! Input type
  AbstractMotion::Type _inType;

  //! Output depth
  double _outDepth;

  //! Output type
  AbstractMotion::Type _outType;
};

#endif // ABSTRACT_RATIO_OUTPUT_H
