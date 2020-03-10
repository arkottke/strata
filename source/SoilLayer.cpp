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

#include "SoilLayer.h"

#include "SoilType.h"
#include "SoilTypeCatalog.h"

SoilLayer::SoilLayer(QObject* parent)
    : VelocityLayer(parent)
{
    _thickness = 0;
    _soilType = nullptr;
}

SoilLayer::SoilLayer(const SoilLayer* other)
{
    // Abstract Distribution
    _avg = other->avg();
    _varied = other->shearVel();
    _stdev = other->stdev();
    _hasMin = other->hasMin();
    _min = other->min();
    _hasMax = other->hasMax();
    _max = other->max();

    // VelocityLayer Information
    _isVaried = other->isVaried();
    _depth = other->depth();

    // SoilLayer Information
    _soilType = other->soilType();
    _thickness = other->thickness();
}

auto SoilLayer::soilType() const -> SoilType *
{
    return _soilType;
}

void SoilLayer::setSoilType(SoilType * soilType)
{
    _soilType = soilType;
}

auto SoilLayer::thickness() const -> double
{
    return _thickness;
}

void SoilLayer::setThickness(double thickness)
{
    _thickness = thickness;
    emit wasModified();
}
        
auto SoilLayer::depthToBase() const -> double
{
    return _depth + _thickness;
}

auto SoilLayer::toString() const -> QString
{
    return QString("%1 %2").arg(_soilType->name()).arg(_avg);
}

auto SoilLayer::untWt() const -> double
{
    if (_soilType)
        return _soilType->untWt();
    else
        return -1;
}

auto SoilLayer::density() const -> double
{
    if (_soilType)
        return _soilType->density();
    else
        return -1;
}

void SoilLayer::fromJson(const QJsonObject &json)
{
    VelocityLayer::fromJson(json);
    _thickness = json["thickness"].toDouble();
}


auto SoilLayer::toJson() const -> QJsonObject
{
    QJsonObject json = VelocityLayer::toJson();
    json["thickness"] = _thickness;

    return json;
}


auto operator<< (QDataStream & out, const SoilLayer* sl) -> QDataStream &
{
    out << (quint8)1;

    out << qobject_cast<const VelocityLayer*>(sl);
    out << sl->_thickness;

    return out;
}

auto operator>> (QDataStream & in, SoilLayer* sl) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >>  qobject_cast<VelocityLayer*>(sl);
    in >>  sl->_thickness;

    return in;
}
