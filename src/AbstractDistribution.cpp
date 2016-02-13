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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "AbstractDistribution.h"

#include <QDebug>
#include <QJsonObject>

#include <cfloat>
#include <cmath>

AbstractDistribution::AbstractDistribution( QObject * parent)
    : QObject(parent)
{
    m_type = LogNormal;
    m_avg = 0;
    m_stdev = 0;
    m_hasMin = false;
    m_min = 0;
    m_hasMax = false;
    m_max = 0;
}

QStringList AbstractDistribution::typeList()
{
    QStringList list;

    list << QObject::tr("Uniform") << QObject::tr("Normal") << QObject::tr("Log Normal");

    return list;
}

AbstractDistribution::Type AbstractDistribution::type() const
{
    return m_type;
}

void AbstractDistribution::setType(int type)
{
    setType((Type)type);
}

void AbstractDistribution::setType(AbstractDistribution::Type type)
{
    if (m_type != type) {
        m_type = type;

        emit requiresLimits(m_type == Uniform);

        emit typeChanged(m_type);
        emit stdevRequiredChanged(stdevRequired());
        emit wasModified();
    }
}

void AbstractDistribution::reset()
{
    m_varied = m_avg;
}

bool AbstractDistribution::stdevRequired()
{
    return m_type != Uniform;
}

double AbstractDistribution::avg() const
{
    return m_avg;
}

void AbstractDistribution::setAvg(double avg)
{
    if (fabs(m_avg - avg) > DBL_EPSILON) {
        m_avg = avg;
        m_varied = avg;

        emit avgChanged(m_avg);
        emit wasModified();
    }
}

double AbstractDistribution::stdev() const
{
    return m_stdev;
}

void AbstractDistribution::setStdev(double stdev)
{
    if (fabs(m_stdev - stdev) > DBL_EPSILON) {
        m_stdev = stdev;

        emit stdevChanged(m_stdev);
        emit wasModified();
    }
}

bool AbstractDistribution::hasMin() const
{
    return m_hasMin;
}

void AbstractDistribution::setHasMin(bool hasMin)
{
    if (m_hasMin != hasMin) {
        m_hasMin = hasMin;

        emit hasMinChanged(m_hasMin);
        emit wasModified();
    }
}

double AbstractDistribution::min() const
{
    return m_min;
}

void AbstractDistribution::setMin(double min)
{
    if (fabs(m_min - min) > DBL_EPSILON) {
        m_min = min;

        emit minChanged(m_min);
        emit wasModified();
    }
}

bool AbstractDistribution::hasMax() const
{
    return m_hasMax;
}

void AbstractDistribution::setHasMax(bool hasMax)
{
    if (m_hasMax != hasMax) {
        m_hasMax = hasMax;

        emit hasMaxChanged(m_hasMax);
        emit wasModified();
    }
}

double AbstractDistribution::max() const
{
    return m_max;
}

void AbstractDistribution::setMax(double max)
{
    if (fabs(m_max - max) > DBL_EPSILON) {
        m_max = max;

        emit maxChanged(m_max);
        emit wasModified();
    }
}

void AbstractDistribution::setVaried(double varied)
{
    if (hasMax())
        varied = qMin(m_max, varied);

    if (hasMin())
        varied = qMax(m_min, varied);

    m_varied = varied;
}

void AbstractDistribution::fromJson(const QJsonObject &json)
{
    m_type = (AbstractDistribution::Type)json["type"].toInt();
    m_avg = json["avg"].toDouble();
    m_stdev = json["stdev"].toDouble();
    m_max = json["max"].toDouble();
    m_min = json["min"].toDouble();
    m_hasMax = json["hasMax"].toBool();
    m_hasMin = json["hasMin"].toBool();
}

QJsonObject AbstractDistribution::toJson() const
{
    QJsonObject json;

    json["type"] = (int)m_type;
    json["avg"] = m_avg;
    json["stdev"] = m_stdev;
    json["max"] = m_max;
    json["min"] = m_min;
    json["hasMax"] = m_hasMax;
    json["hasMin"] = m_hasMin;

    return json;
}

QDataStream & operator<< (QDataStream & out, const AbstractDistribution* ad)
{
    out << (quint8)1;

    out << (int)ad->m_type
        << ad->m_avg
        << ad->m_stdev
        << ad->m_hasMax
        << ad->m_max
        << ad->m_hasMin
        << ad->m_min;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractDistribution* ad)
{
    quint8 ver;
    in >> ver;

    int type;
    in >> type
       >> ad->m_avg
       >> ad->m_stdev
       >> ad->m_hasMax
       >> ad->m_max
       >> ad->m_hasMin
       >> ad->m_min;

    ad->m_type = (AbstractDistribution::Type)type;

    return in;
}
