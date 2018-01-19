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

#include "Distribution.h"

#include <gsl/gsl_randist.h>

Distribution::Distribution(gsl_rng * rng, QObject * parent)
    : AbstractDistribution(parent), _rng(rng)
{
}

double Distribution::rand()
{
    double value = 0;

    switch(_type) {
    case Uniform:
        // Return the variable -- no trunction needed
        value = gsl_ran_flat(_rng, _min, _max);
        break;
    case Normal:
        // Generate the depth
        value = _avg + gsl_ran_gaussian(_rng, _stdev);
        break;
    case LogNormal:
        value = gsl_ran_lognormal(_rng, log(_avg), _stdev);
        break;
    default:
        return -1;
    }
    
    // setVaried applies the limits to the value
    setVaried(value);

    return _varied;
}
