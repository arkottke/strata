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

Distribution::Distribution(gsl_rng * rng, QObject * parent)
    : AbstractDistribution(parent), m_rng(rng)
{
}

double Distribution::rand()
{
    double value = 0;

    switch(m_type) {
    case Uniform:
        // Return the variable -- no trunction needed
        return gsl_ran_flat(m_rng, m_min, m_max);
    case Normal:
        // Generate the depth
        value = m_avg + gsl_ran_gaussian(m_rng, m_stdev);
        break;
    case LogNormal:
        value = gsl_ran_lognormal(m_rng, log(m_avg), m_stdev);
        break;
    default:
        return -1;
    }

    setVaried(value);

    return m_varied;
}
