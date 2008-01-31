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
  : QObject(parent)
{
  m_sdofTfIsComputed = false;
  m_respSpec = new ResponseSpectrum;
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
}

Motion::Type Motion::type() const
{ 
  return m_type;
}

void Motion::setType(Motion::Type type)
{
  m_type = type;
}

ResponseSpectrum * Motion::respSpec() 
{
  return m_respSpec;
}

QVector<double> & Motion::freq()
{
  return m_freq;
}

const double Motion::freqMin() const
{
  return ( m_freq.first() < m_freq.last() ) ? m_freq.first() : m_freq.last();
}

const double Motion::freqMax() const
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

void Motion::computeSdofTf( const QVector<double> & period, double damping )
{
  // Check if the SDOF needs to be computed
  if ( m_sdofTfIsComputed ) {
    // Check if the periods are different
    bool periodsMatch = true;

    if ( period.size() == m_respSpec->period().size() ) {
      for ( int i = 0; i < period.size(); ++i ) {
        if ( period.at(i) != m_respSpec->period().at(i) ) {
          periodsMatch = false;
          break;
        }
      }
    } else 
      periodsMatch = false;

    // Do not recompute if damping  and periods are the dame
    if ( damping == m_respSpec->damping() && periodsMatch )
      return;
  }

  m_respSpec->period() = period;
  m_respSpec->setDamping(damping);

  // Resize the transfer function
  m_sdofTf.resize( period.size() );

  for ( int i = 0; i < period.size(); i++ )
  {
    m_sdofTf[i].resize( m_freq.size() );    	
    // Compute the natural frequency of the oscillator
    double fn = 1 / period.at(i);

    for ( int j = 0; j < m_freq.size(); j++ )
    {
      // 
      // The single degree of freedom transfer function
      //                          - fn^2
      //  H = -------------------------------------------------
      //       ( f^2 - fn^2 ) - 2 * sqrt(-1) * damping * fn * f
      // 
      m_sdofTf[i][j] = ( -fn * fn ) / std::complex<double>( m_freq.at(j) * m_freq.at(j) - fn * fn, -2 * (damping / 100) * fn * m_freq.at(j) );            
    }
  }

  m_sdofTfIsComputed = true;
}
