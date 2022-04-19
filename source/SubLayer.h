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

#ifndef SUB_LAYER_H_
#define SUB_LAYER_H_

#include "SoilLayer.h"

//! A numerical layer through which waves are propagated through

class SubLayer
{
public:
    //! Constructor
    /*!
         * \param thickness thickness of the layer
         * \param depth depth to the top of the layer
         * \param vTotalStress total vertical stress at the top of the layer
         * \param waterTableDepth depth to the water table
         * \param soilLayer soilLayer which describes the dynamic properties of the subLayer
         */
    SubLayer( double thickness, double depth, double vTotalStress, double waterTableDepth, SoilLayer * soilLayer );

    //! Reset the properties of the SubLayer to the initial conditions
    void reset();

    //! Name of the SoilType associated with the subLayer
    auto soilTypeName() const -> QString;

    //! Unit weight of the layer
    auto untWt() const -> double;

    //! Density of the layer
    auto density() const -> double;

    auto vTotalStress(double layerFraction=0.5) const -> double;
    auto vEffectiveStress(double layerFraction=0.5) const -> double;

    auto thickness() const -> double;
    void setThickness(double thickness);

    void setDepth(double depthToTop);

    //! Depth to the top of the layer
    auto depth() const -> double;

    //! Depth to the middle of the layer
    auto depthToMid() const -> double;

    //! Depth to the base of the layer
    auto depthToBase() const -> double;

    auto soilLayer() -> SoilLayer*;
    void setSoilLayer(SoilLayer* soilLayer);

    auto effStrain() const -> double;
    auto maxStrain() const -> double;

    //! The ratio of max shear stress to vertical total stress
    auto stressRatio() const -> double;

    //! Interpolation using the curves
    bool interp(double strain, double* modulus, double* damping) const;

    //! Compute the properties from an initial estimate of strain
    void setInitialStrain(double strain);

    //! Update the shear modulus and damping based a the strain
    /*!
         * \param effStrain effective strain -- maximum strain reduced by effective strain ratio
         * \param maxStrain maximum strain
         */
    bool setStrain(double effStrain, double maxStrain, bool updateProperties=true);

    //! The shear-wave velocity -- corrected for strain
    auto shearVel() const -> double;

    //! The shear modulus -- corrected for strain
    auto shearMod() const -> double;

    //! The shear modulus of the previous iteration
    auto oldShearMod() const -> double;

    //! The error of the shear modulus
    auto shearModError() const -> double;

    //! Normalized shear modulus -- corrected for strain
    auto normShearMod() const -> double;

    //! Shear stres in the layer
    auto shearStress() const -> double;

    //! The shear-wave velocity -- NOT corrected for strain
    auto initialShearVel() const -> double;

    //! The shear modulus -- NOT corrected for strain
    auto initialShearMod() const -> double;

    //! The damping -- corrected for strain
    auto damping() const -> double;

    //! The damping of the previous iteration
    auto oldDamping() const -> double;

    //! The error in the damping
    auto dampingError() const -> double;

    //! Maximum error of the shear modulus and damping
    auto error() const -> double;

    //! Print out the properties of the current iteration
    auto printProperties() const -> QString;

private:
    //! Thickness of the layer
    double _thickness;

    //! Depth to the top of the layer
    double _depth;

    //! Depth to the water table
    double _waterTableDepth;

    //! Total vertical stress at the center of the layer
    double _vTotalStress;

    //! SoilType associated with the SubLayer
    SoilLayer* _soilLayer;

    //! Effective shear strain within the layer
    double _effStrain;

    //! Maximum strain within the layer
    double _maxStrain;

    //! Shear modulus of the layer
    double _shearMod;

    //! Normalized shear modulus
    double _normShearMod;

    //! Shear velocity of the layer
    double _shearVel;

    //! Damping of the layer
    double _damping;

    //! Minimum damping of the layer
    double _minDamping;

    //! Previous shear modulus value
    double _oldShearMod;

    //! Previous damping value
    double _oldDamping;

    //! Error in the shear modulus
    double _shearModError;

    //! Error in the damping
    double _dampingError;
};
#endif
