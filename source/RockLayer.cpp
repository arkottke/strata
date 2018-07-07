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

#include "RockLayer.h"
#include "Units.h"

#include <QDebug>

RockLayer::RockLayer(QObject * parent)
    : VelocityLayer(parent)
{
    _untWt = 22.0;
    setAvgDamping(1.0);
}

double RockLayer::untWt() const
{
    return _untWt;
}

void RockLayer::setUntWt(double untWt)
{
    _untWt = untWt;

    emit wasModified();
    emit untWtChanged(_untWt);
}

double RockLayer::density() const
{
    return _untWt / Units::instance()->gravity();
}

QString RockLayer::toString() const
{
    return QString("Bedrock");
}

double RockLayer::damping() const
{
    return _damping;
}
       
void RockLayer::setDamping(double damping)
{
    _damping = damping;
}

double RockLayer::avgDamping() const
{
    return _avgDamping;
}

void RockLayer::setAvgDamping(double damping )
{

    _damping = damping;
    _avgDamping = damping;

    emit wasModified();
    emit avgDampingChanged(_avgDamping);
}

void RockLayer::fromJson(const QJsonObject &json)
{
    VelocityLayer::fromJson(json);
    _untWt = json["untWt"].toDouble();
    double avgDamping = json["avgDamping"].toDouble();
    setAvgDamping(avgDamping);
}

QJsonObject RockLayer::toJson() const
{
    QJsonObject json = VelocityLayer::toJson();
    json["untWt"] = _untWt;
    json["avgDamping"] = _avgDamping;
    return json;
}


QDataStream & operator<< (QDataStream & out, const RockLayer* rl)
{
    out << (quint8)1;

    out << qobject_cast<const VelocityLayer*>(rl);
    out << rl->_untWt << rl->_avgDamping;

    return out;
}

QDataStream & operator>> (QDataStream & in, RockLayer* rl)
{
    quint8 ver;
    in >> ver;

    double avgDamping;

    in >> qobject_cast<VelocityLayer*>(rl);
    in >> rl->_untWt >> avgDamping;

    rl->setAvgDamping(avgDamping);

    return in;
}
