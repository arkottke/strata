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

#ifndef SOIL_LAYER_H_
#define SOIL_LAYER_H_

#include "VelocityLayer.h"

#include <QDataStream>
#include <QJsonObject>
#include <QPointer>

class SoilTypeCatalog;
class SoilType;

//! Describes the velocity variation and nonlinear response of soil

class SoilLayer : public VelocityLayer {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const SoilLayer *sl)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, SoilLayer *sl) -> QDataStream &;
  friend class SoilProfile;

public:
  explicit SoilLayer(QObject *parent = nullptr);

  explicit SoilLayer(const SoilLayer *soilLayer);

  auto soilType() const -> SoilType *;
  void setSoilType(SoilType *soilType);

  auto thickness() const -> double;

  auto depthToBase() const -> double;

  auto toString() const -> QString;

  auto untWt() const -> double;
  auto density() const -> double;

  auto strainLimit() const -> double;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

protected:
  void setThickness(double thickness);

private:
  //! Soil type of the layer
  QPointer<SoilType> _soilType;

  //! Total thickness of the layer
  double _thickness;
};
#endif
