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
    QString soilTypeName() const;

    //! Unit weight of the layer
    double untWt() const;

    //! Density of the layer
    double density() const;

    double vTotalStress(double layerFraction=0.5) const;
    double vEffectiveStress(double layerFraction=0.5) const;

    double thickness() const;
    void setThickness(double thickness);

    void setDepth(double depthToTop);

    //! Depth to the top of the layer
    double depth() const;

    //! Depth to the middle of the layer
    double depthToMid() const;

    //! Depth to the base of the layer
    double depthToBase() const;

    SoilLayer* soilLayer();
    void setSoilLayer(SoilLayer* soilLayer);

    double effStrain() const;
    double maxStrain() const;

    //! The ratio of max shear stress to vertical total stress
    double stressRatio() const;

    //! Interpolation using the curves
    void interp(double strain, double* modulus, double* damping) const;

    //! Compute the properties from an initial estimate of strain
    void setInitialStrain(double strain);

    //! Update the shear modulus and damping based a the strain
    /*!
         * \param effStrain effective strain -- maximum strain reduced by effective strain ratio
         * \param maxStrain maximum strain
         */
    void setStrain(double effStrain, double maxStrain, bool updateProperties=true);

    //! The shear-wave velocity -- corrected for strain
    double shearVel() const;

    //! The shear modulus -- corrected for strain
    double shearMod() const;

    //! The shear modulus of the previous iteration
    double oldShearMod() const;

    //! The error of the shear modulus
    double shearModError() const;

    //! Normalized shear modulus -- corrected for strain
    double normShearMod() const;

    //! Shear stres in the layer
    double shearStress() const;

    //! The shear-wave velocity -- NOT corrected for strain
    double initialShearVel() const;

    //! The shear modulus -- NOT corrected for strain
    double initialShearMod() const;

    //! The damping -- corrected for strain
    double damping() const;

    //! The damping of the previous iteration
    double oldDamping() const;

    //! The error in the damping
    double dampingError() const;

    //! Maximum error of the shear modulus and damping
    double error() const;

    //! Print out the properties of the current iteration
    QString printProperties() const;

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
