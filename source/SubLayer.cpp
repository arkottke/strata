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

#include "SubLayer.h"

#include "SoilType.h"
#include "Units.h"

#include <QDebug>

#include <algorithm>
#include <cmath>

SubLayer::SubLayer(double thickness, double depth, double vTotalStress,
                   double waterTableDepth, SoilLayer *soilLayer)
    : _thickness(thickness), _depth(depth), _waterTableDepth(waterTableDepth),
      _soilLayer(soilLayer) {
  _vTotalStress = vTotalStress + untWt() * _thickness / 2;
  reset();
}

void SubLayer::reset() {
  _damping = _soilLayer->soilType()->damping();
  _minDamping = _soilLayer->soilType()->minDamping();

  _effStrain = -1;
  _shearMod = initialShearMod();
  _shearVel = initialShearVel();

  _dampingError = -1;
  _shearModError = -1;

  _oldShearMod = -1;
  _oldDamping = -1;
}

auto SubLayer::soilTypeName() const -> QString {
  return const_cast<const SoilLayer *>(_soilLayer)->soilType()->name();
}

auto SubLayer::untWt() const -> double { return _soilLayer->untWt(); }

auto SubLayer::density() const -> double { return _soilLayer->density(); }

auto SubLayer::vTotalStress(double layerFraction) const -> double {
  Q_ASSERT(0 <= layerFraction && layerFraction <= 1);

  return _vTotalStress + (untWt() * _thickness * (layerFraction - 0.5));
}

auto SubLayer::vEffectiveStress(double layerFraction) const -> double {
  const double waterUntWt = Units::instance()->waterUntWt();

  const double poreWaterPressure =
      waterUntWt *
      qMax(0., (_depth + _thickness * (layerFraction)) - _waterTableDepth);

  return vTotalStress(layerFraction) - poreWaterPressure;
}

auto SubLayer::thickness() const -> double { return _thickness; }

void SubLayer::setThickness(double thickness) { _thickness = thickness; }

void SubLayer::setDepth(double depthToTop) { _depth = depthToTop; }

auto SubLayer::depth() const -> double { return _depth; }

auto SubLayer::depthToMid() const -> double { return _depth + _thickness / 2; }

auto SubLayer::depthToBase() const -> double { return _depth + _thickness; }

auto SubLayer::soilLayer() -> SoilLayer * { return _soilLayer; }

void SubLayer::setSoilLayer(SoilLayer *soilLayer) { _soilLayer = soilLayer; }

auto SubLayer::effStrain() const -> double { return _effStrain; }

auto SubLayer::maxStrain() const -> double { return _maxStrain; }

auto SubLayer::stressRatio() const -> double {
  return shearStress() / vEffectiveStress();
}

//! Interpolation using the curves
bool SubLayer::interp(double strain, double *modulus, double *damping) const {
  auto soilType = _soilLayer->soilType();

  if (strain > _soilLayer->strainLimit()) {
    return false;
  }

  *modulus = initialShearMod() * soilType->modulusModel()->interp(strain);
  // Limit the damping by the minimum strain
  *damping = std::max(_minDamping, soilType->dampingModel()->interp(strain));

  return true;
}

void SubLayer::setInitialStrain(double strain) {
  _normShearMod = _soilLayer->soilType()->modulusModel()->interp(strain);
  _shearMod = initialShearMod() * _normShearMod;
  _damping = _soilLayer->soilType()->dampingModel()->interp(strain);
}

bool SubLayer::setStrain(double effStrain, double maxStrain,
                         bool updateProperties) {
  _effStrain = effStrain;
  _maxStrain = maxStrain;

  // Compute the new values
  if (updateProperties) {
    // Save the strain and old properties
    _oldShearMod = _shearMod;
    _oldDamping = _damping;

    bool success = interp(effStrain, &_shearMod, &_damping);
    if (!success) {
      return success;
    }

    _normShearMod = _shearMod / initialShearMod();

    // Update the shear-wave velocity
    _shearVel = sqrt(_shearMod / _soilLayer->density());

    // Compute the error between old and new values of the damping and shear
    // modulus
    _shearModError = 100 * abs(_shearMod - _oldShearMod) / _shearMod;
    _dampingError = 100 * abs(_damping - _oldDamping) / _damping;
  } else {
    _shearModError = 0;
    _dampingError = 0;
  }

  return true;
}

auto SubLayer::shearVel() const -> double { return _shearVel; }

auto SubLayer::shearMod() const -> double { return _shearMod; }

auto SubLayer::oldShearMod() const -> double { return _oldShearMod; }

auto SubLayer::shearModError() const -> double { return _shearModError; }

auto SubLayer::normShearMod() const -> double { return _normShearMod; }

auto SubLayer::shearStress() const -> double {
  return _shearMod * _maxStrain / 100;
}

auto SubLayer::damping() const -> double { return _damping; }

auto SubLayer::oldDamping() const -> double { return _oldDamping; }

auto SubLayer::dampingError() const -> double { return _dampingError; }

auto SubLayer::initialShearVel() const -> double {
  return _soilLayer->shearVel();
}

auto SubLayer::initialShearMod() const -> double {
  return _soilLayer->shearMod();
}

auto SubLayer::error() const -> double {
  return qMax(_shearModError, _dampingError);
}

auto SubLayer::printProperties() const -> QString {
  return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
      .arg(_soilLayer->soilType()->name(), 30)
      .arg(_depth, 5, 'g', 1)
      .arg(_effStrain, 6, 'g', 5)
      .arg(_damping, 4, 'g', 3)
      .arg(_oldDamping, 4, 'g', 3)
      .arg(_dampingError, 4, 'g', 1)
      .arg(_shearMod, 6, 'g', 3)
      .arg(_oldShearMod, 6, 'g', 3)
      .arg(_shearModError, 4, 'g', 1)
      .arg(_normShearMod, 4, 'g', 3);
}
