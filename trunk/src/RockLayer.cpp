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

#include "RockLayer.h"
#include <QDebug>

RockLayer::RockLayer()
{
    reset();
}

RockLayer::~RockLayer()
{
}

void RockLayer::reset()
{
    setAvgDamping(1.0);
    m_untWt = 22;
}

void RockLayer::setUnits(const Units * units)
{
    m_units = units;
}

double RockLayer::untWt() const
{
    return m_untWt;
}

void RockLayer::setUntWt(double untWt)
{
    m_untWt = untWt;
}

double RockLayer::density() const
{
    return m_untWt / m_units->gravity();
}

QString RockLayer::toString() const
{
    return QString("Bedrock");
}

double RockLayer::damping() const
{
    return m_damping;
}
       
void RockLayer::setDamping( double damping )
{
    m_damping = damping;
}

double RockLayer::avgDamping() const
{
    return m_avgDamping;
}

void RockLayer::setAvgDamping( double damping )
{
    m_damping = damping;
    m_avgDamping = damping;
}


QMap<QString, QVariant> RockLayer::toMap() const
{
	QMap<QString, QVariant> map;
    // Members 
	map.insert("avgDamping", m_avgDamping);
	map.insert("untWt", m_untWt);
    // Members inherited from VelocityLayer	
	map.insert("isVaried", m_isVaried);
	map.insert("distribution", (int)m_distribution);
	map.insert("avg", m_avg);
	map.insert("stdev", m_stdev);
	map.insert("max", m_max);
	map.insert("hasMax", m_hasMax);
	map.insert("min", m_min);
	map.insert("hasMin", m_hasMin);
    map.insert("depth", m_depth);
    
	return map;
}

void RockLayer::fromMap( const QMap<QString, QVariant> & map )
{
    // Members
	setAvgDamping(map.value("avgDamping").toDouble());
	m_untWt	        = map.value("untWt").toDouble();
    // Members inherited from VelocityLayer
	m_isVaried	    = map.value("isVaried").toBool();
	m_distribution	= (VelocityLayer::Distribution)map.value("distribution").toInt();
	m_avg           = map.value("avg").toDouble();
	m_stdev	        = map.value("stdev").toDouble();
	m_max	        = map.value("max").toDouble();
	m_hasMax        = map.value("hasMax").toBool();
	m_min	        = map.value("min").toDouble();
	m_hasMin        = map.value("hasMin").toBool();
	m_depth	        = map.value("depth").toDouble();

    // If the layer is not randomized provide the average shear-wave velocity
    m_shearVel = m_avg;
}	
