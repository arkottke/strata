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

#include "MinimumFinder.h"
#include <QDebug>
#include <cmath>

MinimumFinder::MinimumFinder( double center, double delta )
    : m_center(center), m_delta(delta)
{
    m_trials 
        << m_center - m_delta 
        << m_center 
        << m_center + m_delta;
}

double MinimumFinder::center() const
{
    return m_center;
}

double MinimumFinder::minimum() const
{
    return m_minimum;
}

const QList<double> & MinimumFinder::trials() const
{
    return m_trials;
}

void MinimumFinder::setResults(const QList<double> & err)
{
    // Compute the coefficients of a quadratic equations
    double c0 = err.at(1);
    double c1 = -( err.at(0) - err.at(2) ) / ( 2 * m_delta );
    double c2 = ( err.at(2) - 2 * err.at(1) + err.at(0) ) / ( 2 * m_delta * m_delta );

    if ( c2 < 0 ) {
        // value is actually a maximum, use the value corresponding to the
        // smallest error and set that the minimum did not fall instead the
        // bounds.


        int minIdx = 0;

        for (int i = 1; i < err.size(); ++i )
            if ( err.at(i) < err.at(minIdx) )
                minIdx = i;
        
        m_minimum = err.at(minIdx);
        m_minLocation = (minIdx - 1) * m_delta + m_center;
        m_withinBounds = false;
    } else {

        // Compute the maximum/minimum position by differentiating and setting equal to zero
        double minShift = -c1 / ( 2 * c2 );

        // Check if the shift is within the tested bounds
        m_withinBounds = true;
        if ( minShift < 0 && minShift < -m_delta )
            m_withinBounds = false;
        else if ( minShift > 0 && minShift > m_delta )
            m_withinBounds = false;

        if ( !m_withinBounds ) {
            int sign = (minShift<0) ? -1 : 1;
            minShift = sign * m_delta;
        }

        m_minLocation = m_center + minShift;

        // Compute the value at this point
        m_minimum = c2 * m_minLocation * m_minLocation + c1 * m_minLocation + c0;
    }
}

bool MinimumFinder::isDeltaReasonable() const
{
    // Use 5 percent as a limit
    return fabs(m_delta/m_center) > 0.005;
}

void MinimumFinder::moveBounds()
{
    m_center = m_minLocation;

    if ( m_withinBounds )
        m_delta /= 2.0;
    else
        m_delta *= 2.0;
   
    m_trials.clear();
    m_trials 
        << m_center - m_delta 
        << m_center 
        << m_center + m_delta;
        
    // Limit the value to positive numbers
    if ( m_trials.first() < 0 ) {
        double shift = -m_trials.first();

        for (int i = 0; i < m_trials.size(); ++i)
            m_trials[i] += shift;

        m_center = m_trials.at(1);
    } 
}
