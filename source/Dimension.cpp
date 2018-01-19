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

#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>

#include "Dimension.h"

#include <QDebug>


Dimension::Dimension(QObject * parent)
        : QObject(parent)
{
    _min = 0;
    _max = 0;
    _size = 10;
    _spacing = Linear;

    connect(this, SIGNAL(wasModified()),
            this, SLOT(clear()));
}

QStringList Dimension::spacingList()
{
    return QStringList() << tr("Linear") << tr("Log");
}

double Dimension::min() const
{
    return _min;
}

void Dimension::setMin(double min)
{
    if (fabs(_min - min) > DBL_EPSILON ) {
        _min = min;

        emit wasModified();
        emit minChanged(_min);
    }
}

double Dimension::max() const
{
    return _max;
}

void Dimension::setMax(double max)
{
    if (fabs(_max - max) > DBL_EPSILON ) {
        _max = max;

        emit wasModified();
        emit maxChanged(_max);
    }
}

int Dimension::size() const
{
    return _size;
}

double Dimension::at(int i) const
{
    return _data.at(i);
}

void Dimension::setSize(int size)
{
    if (_size != size) {
        _size = size;

        emit sizeChanged(_size);
        emit wasModified();
    }
}

Dimension::Spacing Dimension::spacing() const
{
    return _spacing;
}

void Dimension::setSpacing(Dimension::Spacing spacing)
{
    if ( _spacing != spacing ) {
        _spacing = spacing;


        if (_spacing == Log && fabs(_min) < DBL_EPSILON)
            setMin(0.01);

        emit spacingChanged(_spacing);
        emit wasModified();
    }
}

void Dimension::setSpacing(int spacing)
{
    setSpacing((Spacing)spacing);
}

QVector<double> & Dimension::data()
{
    if (_data.isEmpty())
        init();
    
    return _data;
}

void Dimension::init()
{
    if ( _spacing == Linear )
        _data = linSpace(_min, _max, _size);
    else if ( _spacing == Log )
        _data = logSpace(_min, _max, _size);
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
    _data.clear();
}

void Dimension::fromJson(const QJsonObject &json)
{
    _min = json["min"].toDouble();
    _max = json["max"].toDouble();
    _size = json["size"].toInt();
    _spacing = (Dimension::Spacing) json["spacing"].toInt();
    init();
}

QJsonObject Dimension::toJson() const
{
    QJsonObject json;
    json["min"] = _min;
    json["max"] = _max;
    json["size"] = _size;
    json["spacing"] = (int) _spacing;
    return json;
}

QDataStream & operator<<(QDataStream & out, const Dimension* d)
{
    out << (quint8)1;

    out << d->_min << d->_max << d->_size << (int)d->_spacing;

    return out;
}

QDataStream & operator>>(QDataStream & in, Dimension* d)
{
    quint8 ver;
    in >> ver;

    int spacing;

    in >> d->_min >> d->_max >> d->_size >> spacing;

    d->_spacing = (Dimension::Spacing)spacing;
    d->init();

    return in;
}
