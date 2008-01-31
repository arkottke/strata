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

#include "SoilLayer.h"

SoilLayer::SoilLayer()
{
    m_thickness = 0;
    m_soilType = 0;
}

SoilLayer::SoilLayer( const SoilLayer & other)
    : VelocityLayer( other )
{
    m_soilType = other.soilType();
    m_thickness = other.thickness();
}

SoilType * SoilLayer::soilType() const
{
    return m_soilType;
}

void SoilLayer::setSoilType(SoilType * soilType)
{
    m_soilType = soilType;
}

double SoilLayer::thickness() const
{
    return m_thickness;
}

void SoilLayer::setThickness(double thickness)
{
    m_thickness = thickness;
}
        
double SoilLayer::depthToBase() const
{
    return m_depth + m_thickness;
}

QString SoilLayer::toString() const
{
    return QString("%1 %2").arg(m_soilType->toString()).arg(m_shearVel);
}

double SoilLayer::untWt() const
{
    if (m_soilType)
        return m_soilType->untWt();
    else
        return -1;
}

double SoilLayer::density() const
{
    if (m_soilType)
        return m_soilType->density();
    else
        return -1;
}

QMap<QString, QVariant> SoilLayer::toMap() const
{
    QMap<QString, QVariant> map;

    // Members 
	map.insert("thickness", QVariant(m_thickness));
    // Members inherited from VelocityLayer	
	map.insert("isVaried", QVariant(m_isVaried));
	map.insert("distribution", QVariant((int)m_distribution));
	map.insert("avg", QVariant(m_avg));
	map.insert("stdev", QVariant(m_stdev));
	map.insert("max", QVariant(m_max));
	map.insert("hasMax", QVariant(m_hasMax));
	map.insert("min", QVariant(m_min));
	map.insert("hasMin", QVariant(m_hasMin));
    map.insert("depth", QVariant(m_depth));

    return map;
}

void SoilLayer::fromMap(const QMap<QString, QVariant>& map)
{
    // Members
	m_thickness     = map.value("thickness").toDouble();
    // Members inherited from VelocityLayer
	m_isVaried	    = map.value("isVaried").toBool();
	m_distribution	= (VelocityLayer::Distribution)map.value("distribution").toInt();
	m_avg	        = map.value("avg").toDouble();
	m_stdev	        = map.value("stdev").toDouble();
	m_max	        = map.value("max").toDouble();
	m_hasMax        = map.value("hasMax").toBool();
	m_min	        = map.value("min").toDouble();
	m_hasMin        = map.value("hasMin").toBool();
	m_depth	        = map.value("depth").toDouble();

    // If the layer is not randomized provide the average shear-wave velocity
    m_shearVel = m_avg;
}
