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

#include <QDebug>

#include <cmath> 
#include <cfloat>

#include <gsl_cdf.h>

VelocityLayer::VelocityLayer(QObject* parent)
    : AbstractDistribution(parent)
{
    m_isVaried = true;
    m_depth = 0;
}

VelocityLayer::~VelocityLayer()
{
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
    return m_varied;
}

double VelocityLayer::shearMod() const
{
    return density() * shearVel() * shearVel();
}

bool VelocityLayer::isVaried() const
{
    return m_isVaried;
}

void VelocityLayer::setIsVaried(bool isVaried)
{
    m_isVaried = isVaried;
}

QString VelocityLayer::toString() const
{
    return QString("%1").arg(m_avg);
}

void VelocityLayer::vary(double randVar)
{
    if (m_isVaried) {
        // Randomize the velocity
        switch (m_type) {
        case Normal:
            setVaried(m_avg + gsl_cdf_gaussian_Pinv(randVar, m_stdev));
            break;
        case LogNormal:
            setVaried(m_avg * exp(gsl_cdf_gaussian_Pinv(randVar, m_stdev)));
            break;
        case Uniform:
            m_varied = gsl_cdf_flat_Pinv(randVar, m_min, m_max);
            break;
        }
    } else {
        m_varied = m_avg;
    }
}

void VelocityLayer::fromJson(const QJsonObject &json)
{
    AbstractDistribution::fromJson(json);
    m_isVaried = json["isVaried"].toBool();
    m_depth = json["depth"].toDouble();
}

QJsonObject VelocityLayer::toJson() const
{
    QJsonObject json = AbstractDistribution::toJson();
    json["isVaried"] = m_isVaried;
    json["depth"] = m_depth;
    return json;
}

QDataStream & operator<< (QDataStream & out, const VelocityLayer* vl)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractDistribution*>(vl);

    out << vl->m_isVaried << vl->m_depth;

    return out;
}

QDataStream & operator>> (QDataStream & in, VelocityLayer* vl)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractDistribution*>(vl);

    in >> vl->m_isVaried >> vl->m_depth;

    return in;
}
