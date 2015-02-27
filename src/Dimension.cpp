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

#include <cfloat>
#include <cmath>

Dimension::Dimension(QObject * parent)
        : QObject(parent)
{
    m_min = 0;
    m_max = 0;
    m_size = 10;
    m_spacing = Linear;

    connect(this, SIGNAL(wasModified()),
            this, SLOT(clear()));
}

QStringList Dimension::spacingList()
{
    return QStringList() << tr("Linear") << tr("Log");
}

double Dimension::min() const
{
    return m_min;
}

void Dimension::setMin(double min)
{
    if (fabs(m_min - min) > DBL_EPSILON ) {
        m_min = min;

        emit wasModified();
        emit minChanged(m_min);
    }
}

double Dimension::max() const
{
    return m_max;
}

void Dimension::setMax(double max)
{
    if (fabs(m_max - max) > DBL_EPSILON ) {
        m_max = max;

        emit wasModified();
        emit maxChanged(m_max);
    }
}

int Dimension::size() const
{
    return m_size;
}

double Dimension::at(int i) const
{
    return m_data.at(i);
}

void Dimension::setSize(int size)
{
    if (m_size != size) {
        m_size = size;

        emit sizeChanged(m_size);
        emit wasModified();
    }
}

Dimension::Spacing Dimension::spacing() const
{
    return m_spacing;
}

void Dimension::setSpacing(Dimension::Spacing spacing)
{
    if ( m_spacing != spacing ) {
        m_spacing = spacing;


        if (m_spacing == Log && fabs(m_min) < DBL_EPSILON)
            setMin(0.01);

        emit spacingChanged(m_spacing);
        emit wasModified();
    }
}

void Dimension::setSpacing(int spacing)
{
    setSpacing((Spacing)spacing);
}

QVector<double> & Dimension::data()
{
    if (m_data.isEmpty())
        init();
    
    return m_data;
}

void Dimension::init()
{
    if ( m_spacing == Linear )
        m_data = linSpace(m_min, m_max, m_size);
    else if ( m_spacing == Log )
        m_data = logSpace(m_min, m_max, m_size);
}

QVector<double> Dimension::linSpace( double min, double max, int size )
{
    QVector<double> vec(size);

    double delta = (max-min)/double(size-1);

    for (int i = 0; i < size; ++i) {
        vec[i] = min + i * delta;
    }

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

void Dimension::clear()
{
    m_data.clear();
}

void Dimension::ptRead(const ptree &pt)
{
    m_min = pt.get<double>("min");
    m_max = pt.get<double>("max");
    m_size = pt.get<int>("size");
    m_spacing = (Dimension::Spacing) pt.get<int>("spacing");
    init();
}

void Dimension::ptWrite(ptree &pt) const
{
    pt.put("min", m_min);
    pt.put("max", m_max);
    pt.put("size", m_size);
    pt.put("spacing", (int) m_spacing);
}

QDataStream & operator<<(QDataStream & out, const Dimension* d)
{
    out << (quint8)1;

    out << d->m_min << d->m_max << d->m_size << (int)d->m_spacing;

    return out;
}

QDataStream & operator>>(QDataStream & in, Dimension* d)
{
    quint8 ver;
    in >> ver;

    int spacing;

    in >> d->m_min >> d->m_max >> d->m_size >> spacing;

    d->m_spacing = (Dimension::Spacing)spacing;
    d->init();

    return in;
}
