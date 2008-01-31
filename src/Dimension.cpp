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

#include "Dimension.h"
#include <QObject>
#include <QDebug>
#include <cmath>

Dimension::Dimension()
{
}

QStringList Dimension::spacingList()
{
    QStringList list;

    list << QObject::tr("Linear") << QObject::tr("Log");

    return list;
}

double Dimension::min() const
{
    return m_min;
}

void Dimension::setMin(double min)
{
    m_min = min;
}

double Dimension::max() const
{
    return m_max;
}

void Dimension::setMax(double max)
{
    m_max = max;
}

int Dimension::npts() const
{
    return m_npts;
}

void Dimension::setNpts(int npts)
{
    m_npts = npts;
}

Dimension::Spacing Dimension::spacing() const
{
    return m_spacing;
}

void Dimension::setSpacing(Dimension::Spacing spacing)
{
    m_spacing = spacing;
}

QVector<double> & Dimension::data()
{
    if (m_data.isEmpty())
        init();
    
    return m_data;
}

void Dimension::init( bool hasMin, double minValue, bool hasMax, double maxValue) 
{
    if (hasMin)
        minValue = ( minValue < m_min ) ? m_min : minValue;
    else
        minValue = m_min;

    if (hasMax)
        maxValue = ( m_max < maxValue ) ? m_max : maxValue;
    else 
        maxValue = m_max;

    if ( m_spacing == Linear )
        m_data = linSpace( minValue, maxValue, m_npts);
    else if ( m_spacing == Log )
        m_data = logSpace( minValue, maxValue, m_npts);
}

QVector<double> Dimension::linSpace( double min, double max, int size )
{
	QVector<double> vec(size);

	double delta = (max-min)/(size-1);

	for (int i = 0; i < size; ++i)
		vec[i] = min + i * delta;

	return vec;
}

QVector<double> Dimension::logSpace( double min, double max, int size )
{
	QVector<double> vec(size);

	double logMin = log10( min );
	double logMax = log10( max );

	double delta = pow(10, (logMax-logMin)/(size-1));

    vec[0] = min;
	for (int i = 1; i < size; ++i)
		vec[i] = delta * vec[i-1];

	return vec;
}
        
QMap<QString, QVariant> Dimension::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;

    map.insert("min", m_min);
    map.insert("max", m_max);
    map.insert("npts", m_npts);
    map.insert("spacing", m_spacing);

    if (saveData) {
        QList<QVariant> list;

        for (int i = 0; i < m_data.size(); ++i)
            list << m_data.at(i);

        map.insert("data", list);
    }

    return map;
}


void Dimension::fromMap(const QMap<QString, QVariant> & map)
{
	m_min = map.value("min").toDouble();
	m_max = map.value("max").toDouble();
	m_npts = map.value("npts").toInt();
	m_spacing = (Spacing)map.value("spacing").toInt();

    if (map.contains("data")) {
        QList<QVariant> list = map.value("data").toList();

        m_data.resize(list.size()); 

        for (int i = 0; i < m_data.size(); ++i)
            m_data[i] = list.at(i).toDouble();
    } 
}
