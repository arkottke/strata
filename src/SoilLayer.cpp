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

#include "SoilType.h"
#include "SoilTypeCatalog.h"

SoilLayer::SoilLayer(QObject* parent)
    : VelocityLayer(parent)
{
    m_thickness = 0;
    m_soilType = 0;
}

SoilLayer::SoilLayer(const SoilLayer* other)
{
    // Abstract Distribution
    m_avg = other->avg();
    m_varied = other->shearVel();
    m_stdev = other->stdev();
    m_hasMin = other->hasMin();
    m_min = other->min();
    m_hasMax = other->hasMax();
    m_max = other->max();

    // VelocityLayer Information
    m_isVaried = other->isVaried();
    m_depth = other->depth();

    // SoilLayer Information
    m_soilType = other->soilType();
    m_thickness = other->thickness();
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

    emit wasModified();
}
        
double SoilLayer::depthToBase() const
{
    return m_depth + m_thickness;
}

QString SoilLayer::toString() const
{
    return QString("%1 %2").arg(m_soilType->name()).arg(m_avg);
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

QDataStream & operator<< (QDataStream & out, const SoilLayer* sl)
{
    out << (quint8)1;

    out << qobject_cast<const VelocityLayer*>(sl);
    out << sl->m_thickness;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilLayer* sl)
{
    quint8 ver;
    in >> ver;

    in >>  qobject_cast<VelocityLayer*>(sl);
    in >>  sl->m_thickness;

    return in;
}
