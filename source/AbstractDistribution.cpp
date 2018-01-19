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

#include "AbstractDistribution.h"

#include <QDebug>
#include <QJsonObject>

#include <cfloat>
#define _USE_MATH_DEFINES
#include <cmath>

AbstractDistribution::AbstractDistribution( QObject * parent)
    : QObject(parent)
{
    _type = LogNormal;
    _avg = 0;
    _stdev = 0;
    _hasMin = false;
    _min = 0;
    _hasMax = false;
    _max = 0;
}

QStringList AbstractDistribution::typeList()
{
    QStringList list;

    list << QObject::tr("Uniform") << QObject::tr("Normal") << QObject::tr("Log Normal");

    return list;
}

AbstractDistribution::Type AbstractDistribution::type() const
{
    return _type;
}

void AbstractDistribution::setType(int type)
{
    setType((Type)type);
}

void AbstractDistribution::setType(AbstractDistribution::Type type)
{
    if (_type != type) {
        _type = type;

        emit requiresLimits(_type == Uniform);

        emit typeChanged(_type);
        emit stdevRequiredChanged(stdevRequired());
        emit wasModified();
    }
}

void AbstractDistribution::reset()
{
    _varied = _avg;
}

bool AbstractDistribution::stdevRequired()
{
    return _type != Uniform;
}

double AbstractDistribution::avg() const
{
    return _avg;
}

void AbstractDistribution::setAvg(double avg)
{
    if (fabs(_avg - avg) > DBL_EPSILON) {
        _avg = avg;
        _varied = avg;

        emit avgChanged(_avg);
        emit wasModified();
    }
}

double AbstractDistribution::stdev() const
{
    return _stdev;
}

void AbstractDistribution::setStdev(double stdev)
{
    if (fabs(_stdev - stdev) > DBL_EPSILON) {
        _stdev = stdev;

        emit stdevChanged(_stdev);
        emit wasModified();
    }
}

bool AbstractDistribution::hasMin() const
{
    return _hasMin;
}

void AbstractDistribution::setHasMin(bool hasMin)
{
    if (_hasMin != hasMin) {
        _hasMin = hasMin;

        emit hasMinChanged(_hasMin);
        emit wasModified();
    }
}

double AbstractDistribution::min() const
{
    return _min;
}

void AbstractDistribution::setMin(double min)
{
    if (fabs(_min - min) > DBL_EPSILON) {
        _min = min;

        emit minChanged(_min);
        emit wasModified();
    }
}

bool AbstractDistribution::hasMax() const
{
    return _hasMax;
}

void AbstractDistribution::setHasMax(bool hasMax)
{
    if (_hasMax != hasMax) {
        _hasMax = hasMax;

        emit hasMaxChanged(_hasMax);
        emit wasModified();
    }
}

double AbstractDistribution::max() const
{
    return _max;
}

void AbstractDistribution::setMax(double max)
{
    if (fabs(_max - max) > DBL_EPSILON) {
        _max = max;

        emit maxChanged(_max);
        emit wasModified();
    }
}

void AbstractDistribution::setVaried(double varied)
{
    if (hasMax())
        varied = qMin(_max, varied);

    if (hasMin())
        varied = qMax(_min, varied);

    _varied = varied;
}

void AbstractDistribution::fromJson(const QJsonObject &json)
{
    _type = (AbstractDistribution::Type)json["type"].toInt();
    _avg = json["avg"].toDouble();
    _stdev = json["stdev"].toDouble();
    _max = json["max"].toDouble();
    _min = json["min"].toDouble();
    _hasMax = json["hasMax"].toBool();
    _hasMin = json["hasMin"].toBool();
}

QJsonObject AbstractDistribution::toJson() const
{
    QJsonObject json;

    json["type"] = (int)_type;
    json["avg"] = _avg;
    json["stdev"] = _stdev;
    json["max"] = _max;
    json["min"] = _min;
    json["hasMax"] = _hasMax;
    json["hasMin"] = _hasMin;

    return json;
}

QDataStream & operator<< (QDataStream & out, const AbstractDistribution* ad)
{
    out << (quint8)1;

    out << (int)ad->_type
        << ad->_avg
        << ad->_stdev
        << ad->_hasMax
        << ad->_max
        << ad->_hasMin
        << ad->_min;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractDistribution* ad)
{
    quint8 ver;
    in >> ver;

    int type;
    in >> type
       >> ad->_avg
       >> ad->_stdev
       >> ad->_hasMax
       >> ad->_max
       >> ad->_hasMin
       >> ad->_min;

    ad->_type = (AbstractDistribution::Type)type;

    return in;
}
