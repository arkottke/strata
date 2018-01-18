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

#include <cmath>

SubLayer::SubLayer(double thickness, double depth, double vTotalStress, double waterTableDepth, SoilLayer * soilLayer)
    : _thickness(thickness), _depth(depth), _waterTableDepth(waterTableDepth), _soilLayer(soilLayer)
{
    _vTotalStress = vTotalStress + untWt() * _thickness / 2;
    reset();
}

void SubLayer::reset()
{
    _damping = _soilLayer->soilType()->damping();

    _effStrain = -1;
    _shearMod = initialShearMod();
    _shearVel = initialShearVel();

    _dampingError = -1;
    _shearModError = -1;

    _oldShearMod = -1;
    _oldDamping = -1;
}

QString SubLayer::soilTypeName() const
{
    return const_cast<const SoilLayer*>(_soilLayer)->soilType()->name();
}

double SubLayer::untWt() const
{
    return _soilLayer->untWt();
}

double SubLayer::density() const
{
    return _soilLayer->density();
}

double SubLayer::vTotalStress(double layerFraction) const
{
    Q_ASSERT(0 <= layerFraction && layerFraction <=1);

    return _vTotalStress + (untWt() * _thickness * (layerFraction - 0.5));
}

double SubLayer::vEffectiveStress(double layerFraction) const
{
    const double waterUntWt = Units::instance()->waterUntWt();
    
    const double poreWaterPressure = waterUntWt * qMax(0., 
            (_depth + _thickness * (layerFraction)) - _waterTableDepth);

    return vTotalStress(layerFraction)- poreWaterPressure;
}

double SubLayer::thickness() const
{
    return _thickness;
}

void SubLayer::setThickness(double thickness)
{
    _thickness = thickness;
}

void SubLayer::setDepth(double depthToTop)
{
    _depth = depthToTop;
}

double SubLayer::depth() const
{
    return _depth;
}

double SubLayer::depthToMid() const
{
    return _depth + _thickness / 2;
}

double SubLayer::depthToBase() const
{
    return _depth + _thickness;
}

SoilLayer* SubLayer::soilLayer()
{
    return _soilLayer;
}

void SubLayer::setSoilLayer(SoilLayer* soilLayer)
{
    _soilLayer = soilLayer;
}

double SubLayer::effStrain() const
{
    return _effStrain;
}

double SubLayer::maxStrain() const
{
    return _maxStrain;
}

double SubLayer::stressRatio() const
{
    return shearStress() / vEffectiveStress();
}

//! Interpolation using the curves
void SubLayer::interp(double strain, double* modulus, double* damping) const
{
    *modulus = initialShearMod() * _soilLayer->soilType()->modulusModel()->interp(strain);
    *damping = _soilLayer->soilType()->dampingModel()->interp(strain);
}


void SubLayer::setInitialStrain(double strain)
{
    _normShearMod = _soilLayer->soilType()->modulusModel()->interp(strain);
    _shearMod = initialShearMod() * _normShearMod;
    _damping = _soilLayer->soilType()->dampingModel()->interp(strain);
}

void SubLayer::setStrain(double effStrain, double maxStrain, bool updateProperties)
{
    _effStrain = effStrain;
    _maxStrain = maxStrain;

    // Compute the new values
    if (updateProperties) {
        // Save the strain and old properties
        _oldShearMod = _shearMod;
        _oldDamping = _damping;

        interp(effStrain, &_shearMod, &_damping);
        _normShearMod = _shearMod / initialShearMod();

        // Update the shear-wave velocity
        _shearVel = sqrt(_shearMod / _soilLayer->density());

        // Compute the error between old and new values of the damping and shear modulus
        _shearModError = 100 * fabs(_shearMod - _oldShearMod) / _shearMod;
        _dampingError  = 100 * fabs(_damping - _oldDamping) / _damping;
    } else {
        _shearModError = 0;
        _dampingError = 0;
    }
}

double SubLayer::shearVel() const
{
    return _shearVel;
}

double SubLayer::shearMod() const
{
    return _shearMod;
}

double SubLayer::oldShearMod() const
{
    return _oldShearMod;
}

double SubLayer::shearModError() const
{
    return _shearModError;
}

double SubLayer::normShearMod() const
{
    return _normShearMod;
}

double SubLayer::shearStress() const
{
    return _shearMod * _maxStrain / 100;
}

double SubLayer::damping() const
{
    return _damping;
}

double SubLayer::oldDamping() const
{
    return _oldDamping;
}

double SubLayer::dampingError() const
{
    return _dampingError;
}

double SubLayer::initialShearVel() const
{
    return _soilLayer->shearVel();
}

double SubLayer::initialShearMod() const
{
    return _soilLayer->shearMod();
}

double SubLayer::error() const
{
    return qMax(_shearModError, _dampingError);
}

QString SubLayer::printProperties() const
{
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
