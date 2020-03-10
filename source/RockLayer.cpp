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

auto RockLayer::untWt() const -> double
{
    return _untWt;
}

void RockLayer::setUntWt(double untWt)
{
    _untWt = untWt;

    emit wasModified();
    emit untWtChanged(_untWt);
}

auto RockLayer::density() const -> double
{
    return _untWt / Units::instance()->gravity();
}

auto RockLayer::toString() const -> QString
{
    return QString("Bedrock");
}

auto RockLayer::damping() const -> double
{
    return _damping;
}
       
void RockLayer::setDamping(double damping)
{
    _damping = damping;
}

auto RockLayer::avgDamping() const -> double
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

auto RockLayer::toJson() const -> QJsonObject
{
    QJsonObject json = VelocityLayer::toJson();
    json["untWt"] = _untWt;
    json["avgDamping"] = _avgDamping;
    return json;
}


auto operator<< (QDataStream & out, const RockLayer* rl) -> QDataStream &
{
    out << (quint8)1;

    out << qobject_cast<const VelocityLayer*>(rl);
    out << rl->_untWt << rl->_avgDamping;

    return out;
}

auto operator>> (QDataStream & in, RockLayer* rl) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    double avgDamping;

    in >> qobject_cast<VelocityLayer*>(rl);
    in >> rl->_untWt >> avgDamping;

    rl->setAvgDamping(avgDamping);

    return in;
}
