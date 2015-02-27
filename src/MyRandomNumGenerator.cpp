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

#include "MyRandomNumGenerator.h"

#include <QDateTime>
#include <QDebug>

MyRandomNumGenerator::MyRandomNumGenerator(QObject * parent)
    : QObject(parent)
{
    m_gsl_rng = gsl_rng_alloc(gsl_rng_mt19937);
    m_seedSpecified = false;
    init();
}

MyRandomNumGenerator::~MyRandomNumGenerator()
{
    gsl_rng_free(m_gsl_rng);
}

bool MyRandomNumGenerator::seedSpecified() const
{
    return m_seedSpecified;
}

quint32 MyRandomNumGenerator::seed() const
{
    return m_seed;
}

gsl_rng * MyRandomNumGenerator::gsl_pointer()
{
    return m_gsl_rng;
}

void MyRandomNumGenerator::setSeedSpecified(bool seedSpecified)
{
    if (m_seedSpecified != seedSpecified) {
        m_seedSpecified = seedSpecified;

        emit seedSpecifiedChanged(m_seedSpecified);
        emit wasModified();
    }
}

void MyRandomNumGenerator::setSeed(int seed)
{
    setSeed((quint32)seed);
}

void MyRandomNumGenerator::setSeed(quint32 seed)
{
    if (m_seed != seed) {
        m_seed = seed;
        emit seedChanged((int)m_seed);
        emit wasModified();
    }    
}

void MyRandomNumGenerator::init()
{
    if (!m_seedSpecified){
        setSeed(1 + rand() % 65534);
    }
    gsl_rng_set(m_gsl_rng, m_seed);
}

void MyRandomNumGenerator::ptRead(const ptree &pt)
{
    m_seedSpecified = pt.get<bool>("seedSpecified");
    m_seed = pt.get<quint32>("seed");
}

void MyRandomNumGenerator::ptWrite(ptree &pt) const
{
    pt.put("seedSpecified", m_seedSpecified);
    pt.put("seed", m_seed);
}

QDataStream & operator<< (QDataStream & out, const MyRandomNumGenerator* myGenerator)
{
    out << (quint8)1;
    
    out << myGenerator->m_seedSpecified
        << myGenerator->m_seed;

    return out;
}

QDataStream & operator>> (QDataStream & in, MyRandomNumGenerator* myGenerator)
{
    quint8 version;
    in >> version;

    in >> myGenerator->m_seedSpecified
        >> myGenerator->m_seed;

    return in;
}
