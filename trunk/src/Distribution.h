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

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include "AbstractDistribution.h"

#include <gsl/gsl_rng.h>

class Distribution : public AbstractDistribution
{
    Q_OBJECT

public:
    Distribution(gsl_rng * rng, QObject * parent = 0);

    //! Return a random variable from the distribution
    double rand();

protected:
    //! The random number generator of the distribution
    gsl_rng * m_rng;
};
#endif
