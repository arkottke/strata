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

#include "Distribution.h"
#include <QObject>
#include <gsl/gsl_randist.h>

#include <cmath>

Distribution::Distribution( QObject * parent)
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

QStringList Distribution::typeList()
{
    QStringList list;

    list << QObject::tr("Uniform") << QObject::tr("Normal") << QObject::tr("Log Normal");

    return list;
}

Distribution::Type Distribution::type() const
{
    return m_type;
}

void Distribution::setType(int type)
{
    setType((Type)type);
}

void Distribution::setType(Distribution::Type type)
{
    if ( m_type != type ) {
        emit wasModified();
    }

    m_type = type;
}

double Distribution::avg() const
{
    return m_avg;
}

void Distribution::setAvg(double avg)
{
    if ( m_avg != avg ) {
        emit wasModified();
    }

    m_avg = avg;
}
        
double Distribution::stdev() const
{
    return m_stdev;
}

void Distribution::setStdev(double stdev)
{
    if ( m_stdev != stdev ) {
        emit wasModified();
    }

    m_stdev = stdev;
}

bool Distribution::hasMin() const
{
    return m_hasMin;
}

void Distribution::setHasMin(bool hasMin)
{
    if ( m_hasMin != hasMin ) {
        emit wasModified();
    }

    m_hasMin = hasMin;
}

double Distribution::min() const
{
    return m_min;
}

void Distribution::setMin(double min)
{
    if ( m_min != min ) {
        emit wasModified();
    }

    m_min = min;
}

bool Distribution::hasMax() const
{
    return m_hasMax;
}

void Distribution::setHasMax(bool hasMax)
{
    if ( m_hasMax != hasMax ) {
        emit wasModified();
    }

    m_hasMax = hasMax;
}

double Distribution::max() const
{
    return m_max;
}

void Distribution::setMax(double max)
{
    if ( m_max != max ) {
        emit wasModified();
    }

    m_max = max;
}

void Distribution::setRandomNumberGenerator( gsl_rng * rng )
{
    m_rng = rng;
}

double Distribution::rand()
{
    double value = 0;

    switch(m_type)
    {
        case Uniform:
            // Return the variable -- no trunction needed
            return gsl_ran_flat( m_rng, m_min, m_max );
        case Normal:
            // Generate the depth
            value = m_avg + gsl_ran_gaussian( m_rng, m_stdev);
            break;
        case LogNormal:
            value = gsl_ran_lognormal( m_rng, log(m_avg), m_stdev);
            break;
        default:
            return -1;
    }

    // Check if the generated depth is less than the minimum
    if ( m_hasMin && value < m_min ) {
        value = m_min;
    }
    // Or if the generated depth is greater than the maximum
    if ( m_hasMax && value > m_max ) {
        value = m_max;
    }

    return value;
}
    
QMap<QString, QVariant> Distribution::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("type", m_type);
    map.insert("avg", m_avg);
    map.insert("stdev", m_stdev);
    
    map.insert("hasMin", m_hasMin);
    map.insert("min", m_min);
    
    map.insert("hasMax", m_hasMax);
    map.insert("max", m_max);
    
    return map;
}

void Distribution::fromMap( const QMap<QString, QVariant> & map )
{
    m_type = (Type)map.value("type").toInt();
    m_avg = map.value("avg").toDouble();
    m_stdev = map.value("stdev").toDouble();

	m_hasMin = map.value("hasMin").toBool();
	m_min = map.value("min").toDouble();
	
    m_hasMax = map.value("hasMax").toBool();
	m_max = map.value("max").toDouble();
}
