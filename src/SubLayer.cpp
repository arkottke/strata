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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "SubLayer.h"
#include <cmath>
#include <QDebug>

SubLayer::SubLayer( double thickness, double depth, double vTotalStress, SoilLayer * soilLayer )
    : m_thickness(thickness), m_depth(depth), m_soilLayer(soilLayer)
{
    m_vTotalStress = vTotalStress + untWt() * m_thickness/2;
    reset();
}

void SubLayer::reset()
{
    m_damping = m_soilLayer->soilType()->initialDamping();

    m_effStrain = -1;
    m_shearMod = initialShearMod();
    m_shearVel = initialShearVel();

	m_dampingError = -1;
	m_shearModError = -1;
 
    m_oldShearMod = -1;
    m_oldDamping = -1;
}

QString SubLayer::soilTypeName() const
{
    return const_cast<const SoilLayer *>(m_soilLayer)->soilType()->name();
}

double SubLayer::untWt() const
{
    return m_soilLayer->untWt();
}

double SubLayer::density() const
{
    return m_soilLayer->density();
}

double SubLayer::vTotalStress() const
{
    return m_vTotalStress;
}

double SubLayer::thickness() const
{
	return m_thickness;
}

void SubLayer::setThickness(double thickness)
{
	m_thickness = thickness;
}

void SubLayer::setDepth(double depthToTop)
{
    m_depth = depthToTop;
}

double SubLayer::depth() const
{
    return m_depth;
}

double SubLayer::depthToMid() const
{
    return m_depth + m_thickness / 2;
}

double SubLayer::depthToBase() const
{
    return m_depth + m_thickness;
}

SoilLayer * SubLayer::soilLayer()
{
    return m_soilLayer;
}

void SubLayer::setSoilLayer(SoilLayer * soilLayer)
{
    m_soilLayer = soilLayer;
}
        
double SubLayer::effStrain() const
{
    return m_effStrain;
}

double SubLayer::maxStrain() const
{
    return m_maxStrain;
}

double SubLayer::stressRatio() const
{
    return shearStress() / vTotalStress();
}

void SubLayer::setStrain( double effStrain, double maxStrain )
{
	m_effStrain = effStrain;
    m_maxStrain = maxStrain;
	// Save the strain and old properties
	m_oldShearMod = m_shearMod;
	m_oldDamping = m_damping;

	// Compute the new values
    m_normShearMod = m_soilLayer->soilType()->normShearMod().interp(effStrain);
	m_shearMod = initialShearMod() * m_normShearMod; 
	m_damping = m_soilLayer->soilType()->damping().interp(effStrain);

	// Check that the values are non-negative
	if ( m_shearMod < 0 || m_damping < 0 )
		qCritical("Negative nonlinear properties!");

	// Update the shear-wave velocity
	m_shearVel = sqrt( m_shearMod / m_soilLayer->untWt() );
	
	// Compute the error between old and new values of the damping and shear modulus
	m_shearModError = 100 * fabs( m_shearMod - m_oldShearMod ) / m_shearMod;
	m_dampingError  = 100 * fabs( m_damping - m_oldDamping ) / m_damping;
}

double SubLayer::shearVel() const
{
	return m_shearVel;	
}

double SubLayer::shearMod() const
{
	return m_shearMod;
}

double SubLayer::oldShearMod() const
{
    return m_oldShearMod;
}

double SubLayer::shearModError() const
{
    return m_shearModError;
}

double SubLayer::normShearMod() const
{
    return m_normShearMod;
}

double SubLayer::shearStress() const
{
    return m_shearMod * m_maxStrain / 100;
}

double SubLayer::damping() const
{
    return m_damping;
}

double SubLayer::oldDamping() const
{
    return m_oldDamping;
}

double SubLayer::dampingError() const
{
    return m_dampingError;
}

double SubLayer::initialShearVel() const
{
    return m_soilLayer->shearVel();
}

double SubLayer::initialShearMod() const
{
    return m_soilLayer->shearMod();
}

double SubLayer::error() const
{
	// Return the larger of the two errors
	if ( m_shearModError > m_dampingError )
		return m_shearModError;
	else
		return m_dampingError; 
}

QString SubLayer::printProperties() const
{
    return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
        .arg(m_soilLayer->soilType()->toString(), 30)
        .arg(m_depth, 5, 'g', 1)
        .arg(m_effStrain, 6, 'g', 5)
        .arg(m_damping, 4, 'g', 3)
        .arg(m_oldDamping, 4, 'g', 3)
        .arg(m_dampingError, 4, 'g', 1)
        .arg(m_shearMod, 6, 'g', 3)
        .arg(m_oldShearMod, 6, 'g', 3)
        .arg(m_shearModError, 4, 'g', 1)
        .arg(m_normShearMod, 4, 'g', 3);
}
