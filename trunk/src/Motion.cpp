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

#include "Motion.h"
#include <QDebug>

#include <cmath>


#include <iostream>

Motion::Motion(QObject * parent)
    : QObject(parent), m_maxEngFreq(25.)
{
    m_respSpec = new ResponseSpectrum(true);
    connect(m_respSpec, SIGNAL(wasModified()), SLOT(setModified()));
}

Motion::~Motion()
{
    delete m_respSpec;
}

QStringList Motion::typeList()
{
    QStringList list;
    list << "Outcrop" << "Within";

    return list;
}

bool Motion::modified() const
{
    return m_modified;
}

void Motion::setModified(bool modified)
{
    m_modified = modified;

    if (m_modified)
        emit wasModified();
}

Motion::Type Motion::type() const
{ 
    return m_type;
}

void Motion::setType(int type)
{
    setType((Type)type);
    setModified(true);
}

void Motion::setType(Motion::Type type)
{
    m_type = type;
    setModified(true);
}

ResponseSpectrum * Motion::respSpec() 
{
    return m_respSpec;
}

QVector<double> & Motion::freq()
{
    return m_freq;
}

void Motion::setMaxEngFreq(double maxEngFreq)
{
    m_maxEngFreq = maxEngFreq;
    setModified(true);
}

double Motion::maxEngFreq() const
{
    return m_maxEngFreq;
}

double Motion::freqMin() const
{
    return ( m_freq.first() < m_freq.last() ) ? m_freq.first() : m_freq.last();
}

double Motion::freqMax() const
{
    return ( m_freq.first() < m_freq.last() ) ? m_freq.last() : m_freq.first();
}

int Motion::freqCount() const
{
    return m_freq.size();
}

double Motion::angFreqAt( int i ) const
{
    return 2 * M_PI * m_freq.at(i);
}

double Motion::freqAt( int i ) const
{
    return m_freq.at(i);
}

QVector<std::complex<double> > Motion::calcSdofTf(const double period, const double damping) const
{
    QVector<std::complex<double> > tf(m_freq.size());

    // Natural frequency of the oscillator
    const double fn = 1 / period;

    for (int i = 0; i < m_freq.size(); i++) {
        //
        // The single degree of freedom transfer function
        //                          - fn^2
        //  H = -------------------------------------------------
        //       ( f^2 - fn^2 ) - 2 * sqrt(-1) * damping * fn * f
        //
        tf[i] = (-fn * fn) / std::complex<double>(m_freq.at(i) * m_freq.at(i) - fn * fn, -2 * (damping / 100) * fn * m_freq.at(i));
    }

    return tf;
}
