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

#include "VelocityLayer.h"

#include <cmath> 

#include <gsl/gsl_cdf.h>

#include <QMap>
#include <QString>
#include <QVariant>
#include <QDebug>

VelocityLayer::VelocityLayer( QObject * parent )
    : QObject(parent)
{
    m_depth = 0;
    m_isVaried = true;
    m_avg = 0;
	m_min = 0;
    m_hasMin = false;
	m_max = 0;
    m_hasMax = false;
    m_stdev = 0;
	m_distribution = LogNormal;
}

VelocityLayer::VelocityLayer( const VelocityLayer & other)
    :QObject(other.parent())
{
    m_depth = other.depth();
    m_isVaried = other.isVaried();
    setAvg(other.avg());
	m_min = other.min();
    m_hasMin = other.hasMin();
	m_max = other.max();
    m_hasMax = other.hasMax();
    m_stdev = other.stdev();
	m_distribution = other.distribution();
}

VelocityLayer::~VelocityLayer()
{
}

QStringList VelocityLayer::distributionList()
{
    QStringList list;

    list << "Normal" << "Log Normal" << "Uniform";

    return list;
}

double VelocityLayer::depth() const
{
    return m_depth;
}
void VelocityLayer::setDepth(double depth)
{
    m_depth = depth;
}

double VelocityLayer::shearVel() const
{
	return m_shearVel;
}

void VelocityLayer::setShearVel(double shearVel)
{
    // If the variation of the layer is disabled do not set the randomized
    // shear-wave velocity of the layer
	if (!m_isVaried)
        return;

    m_shearVel = shearVel;
    // Check that the shear-wave velocity is within the specified bounds
    if (m_hasMin && m_shearVel < m_min)
        m_shearVel = m_min;

    if ( m_hasMax && m_shearVel > m_max)
        m_shearVel = m_max;
}

double VelocityLayer::shearMod() const
{
    return density() * m_shearVel * m_shearVel;
}

double VelocityLayer::avg() const
{
	return m_avg;
}

void VelocityLayer::setAvg(double avg)
{
	m_avg = avg;
    m_shearVel = avg;
}

VelocityLayer::Distribution VelocityLayer::distribution() const
{
	return m_distribution;
}
		
void VelocityLayer::setDistribution(VelocityLayer::Distribution dist)
{
    m_distribution = dist;
}

double VelocityLayer::stdev() const
{
	return m_stdev;
}

void VelocityLayer::setStdev(double stdev)
{
	m_stdev = stdev;
}

bool VelocityLayer::hasMax() const
{
    return m_hasMax;
}

void VelocityLayer::setHasMax(bool hasMax) 
{
    m_hasMax = hasMax;
}

double VelocityLayer::max() const
{
	return m_max;
}

void VelocityLayer::setMax(double max)
{
	m_max = max;
}

bool VelocityLayer::hasMin() const
{
    return m_hasMin;
}

void VelocityLayer::setHasMin(bool hasMin) 
{
    m_hasMin = hasMin;
}

double VelocityLayer::min() const
{
	return m_min;
}

void VelocityLayer::setMin(double min)
{
	m_min = min;
}

double VelocityLayer::isVaried() const
{
	return m_isVaried;
}

void VelocityLayer::setIsVaried(bool isVaried)
{
	m_isVaried = isVaried;
}

void VelocityLayer::reset()
{
    m_shearVel = m_avg;
}

QString VelocityLayer::toString() const
{
    return QString("%1").arg(m_avg);
}

void VelocityLayer::vary( double randVar  )
{
	if (m_isVaried) {
		// Randomize the velocity
        switch (m_distribution)
        {
            case Normal:
                return setShearVel( m_avg + gsl_cdf_gaussian_Pinv( randVar, m_stdev));
                break;
            case LogNormal:
                return setShearVel( m_avg * exp(gsl_cdf_gaussian_Pinv( randVar, m_stdev)));
            case Uniform:
                return setShearVel( gsl_cdf_flat_Pinv( randVar, m_min, m_max));
        }	
	} else
		// Don't randomize the velocity
        setShearVel(m_avg);
}
