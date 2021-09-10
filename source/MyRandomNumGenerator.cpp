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

#include "MyRandomNumGenerator.h"

#include <QDataStream>
#include <QDateTime>
#include <QDebug>

MyRandomNumGenerator::MyRandomNumGenerator(QObject * parent)
    : QObject(parent),
    _seedSpecified(false),
    _seed(0)
{
    _gsl_rng = gsl_rng_alloc(gsl_rng_mt19937);
    _seedSpecified = false;
    init();
}

MyRandomNumGenerator::~MyRandomNumGenerator()
{
    gsl_rng_free(_gsl_rng);
}

auto MyRandomNumGenerator::seedSpecified() const -> bool
{
    return _seedSpecified;
}

auto MyRandomNumGenerator::seed() const -> quint32
{
    return _seed;
}

auto MyRandomNumGenerator::gsl_pointer() -> gsl_rng *
{
    return _gsl_rng;
}

void MyRandomNumGenerator::setSeedSpecified(bool seedSpecified)
{
    if (_seedSpecified != seedSpecified) {
        _seedSpecified = seedSpecified;

        emit seedSpecifiedChanged(_seedSpecified);
        emit wasModified();
    }
}

void MyRandomNumGenerator::setSeed(int seed)
{
    setSeed((quint32)seed);
}

void MyRandomNumGenerator::setSeed(quint32 seed)
{
    if (_seed != seed) {
        _seed = seed;
        emit seedChanged((int)_seed);
        emit wasModified();
    }    
}

void MyRandomNumGenerator::init()
{
    if (!_seedSpecified){
        setSeed(1 + rand() % 65534);
    }
    gsl_rng_set(_gsl_rng, _seed);
}

void MyRandomNumGenerator::fromJson(const QJsonObject &json)
{
    _seedSpecified = json["seedSpecified"].toBool();
    _seed = (quint32)json["seed"].toInt();
}

auto MyRandomNumGenerator::toJson() const -> QJsonObject
{
    QJsonObject json;
    json["seedSpecified"] = _seedSpecified;
    json["seed"] = (int) _seed;
    return json;
}


auto operator<< (QDataStream & out, const MyRandomNumGenerator* myGenerator) -> QDataStream &
{
    out << (quint8)1;
    
    out << myGenerator->_seedSpecified
        << myGenerator->_seed;

    return out;
}

auto operator>> (QDataStream & in, MyRandomNumGenerator* myGenerator) -> QDataStream &
{
    quint8 version;
    in >> version;

    in >> myGenerator->_seedSpecified
        >> myGenerator->_seed;

    return in;
}
