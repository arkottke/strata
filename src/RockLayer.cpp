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
#include "Units.h"

#include <QDebug>

RockLayer::RockLayer(QObject * parent)
    : VelocityLayer(parent)
{
    m_untWt = 22.0;
    setAvgDamping(1.0);
}

double RockLayer::untWt() const
{
    return m_untWt;
}

void RockLayer::setUntWt(double untWt)
{
    m_untWt = untWt;

    emit wasModified();
    emit untWtChanged(m_untWt);
}

double RockLayer::density() const
{
    return m_untWt / Units::instance()->gravity();
}

QString RockLayer::toString() const
{
    return QString("Bedrock");
}

double RockLayer::damping() const
{
    return m_damping;
}
       
void RockLayer::setDamping(double damping)
{
    m_damping = damping;
}

double RockLayer::avgDamping() const
{
    return m_avgDamping;
}

void RockLayer::setAvgDamping(double damping )
{

    m_damping = damping;
    m_avgDamping = damping;

    emit wasModified();
    emit avgDampingChanged(m_avgDamping);
}

QDataStream & operator<< (QDataStream & out, const RockLayer* rl)
{
    out << (quint8)1;

    out << qobject_cast<const VelocityLayer*>(rl);
    out << rl->m_untWt << rl->m_avgDamping;

    return out;
}

QDataStream & operator>> (QDataStream & in, RockLayer* rl)
{
    quint8 ver;
    in >> ver;

    double avgDamping;

    in >> qobject_cast<VelocityLayer*>(rl);
    in >> rl->m_untWt >> avgDamping;

    rl->setAvgDamping(avgDamping);

    return in;
}
