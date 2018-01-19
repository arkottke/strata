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

#include "VelocityLayer.h"

#include <QDebug>

#define _USE_MATH_DEFINES
#include <cmath> 
#include <cfloat>

#include <gsl/gsl_cdf.h>

VelocityLayer::VelocityLayer(QObject* parent)
    : AbstractDistribution(parent)
{
    _isVaried = true;
    _depth = 0;
}

VelocityLayer::~VelocityLayer()
{
}

double VelocityLayer::depth() const
{
    return _depth;
}
void VelocityLayer::setDepth(double depth)
{
    _depth = depth;
}

double VelocityLayer::shearVel() const
{
    return _varied;
}

double VelocityLayer::shearMod() const
{
    return density() * shearVel() * shearVel();
}

bool VelocityLayer::isVaried() const
{
    return _isVaried;
}

void VelocityLayer::setIsVaried(bool isVaried)
{
    _isVaried = isVaried;
}

QString VelocityLayer::toString() const
{
    return QString("%1").arg(_avg);
}

void VelocityLayer::vary(double randVar)
{
    if (_isVaried) {
        // Randomize the velocity
        switch (_type) {
        case Normal:
            setVaried(_avg + gsl_cdf_gaussian_Pinv(randVar, _stdev));
            break;
        case LogNormal:
            setVaried(_avg * exp(gsl_cdf_gaussian_Pinv(randVar, _stdev)));
            break;
        case Uniform:
            _varied = gsl_cdf_flat_Pinv(randVar, _min, _max);
            break;
        }
    } else {
        _varied = _avg;
    }
}

void VelocityLayer::fromJson(const QJsonObject &json)
{
    AbstractDistribution::fromJson(json);
    _isVaried = json["isVaried"].toBool();
    _depth = json["depth"].toDouble();
}

QJsonObject VelocityLayer::toJson() const
{
    QJsonObject json = AbstractDistribution::toJson();
    json["isVaried"] = _isVaried;
    json["depth"] = _depth;
    return json;
}

QDataStream & operator<< (QDataStream & out, const VelocityLayer* vl)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractDistribution*>(vl);

    out << vl->_isVaried << vl->_depth;

    return out;
}

QDataStream & operator>> (QDataStream & in, VelocityLayer* vl)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractDistribution*>(vl);

    in >> vl->_isVaried >> vl->_depth;

    return in;
}
