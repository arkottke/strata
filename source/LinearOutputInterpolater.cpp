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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "LinearOutputInterpolater.h"

#include <gsl_interp.h>

LinearOutputInterpolater::LinearOutputInterpolater()
{
}

QVector<double> LinearOutputInterpolater::calculate(
        const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi)
{
    QVector<double> yi;

    // Allocate the interpolator
    gsl_interp* interpolator = gsl_interp_alloc(gsl_interp_linear, y.size());
    gsl_interp_init(interpolator, x.data(), y.data(), y.size());
    gsl_interp_accel* accelerator =  gsl_interp_accel_alloc();

    for (int i = 0; i < xi.size(); ++i) {
        if (xi.at(i) < x.last()) {
            yi << gsl_interp_eval(interpolator, x.data(), y.data(), xi.at(i), accelerator);
        } else {
            break;
        }
    }

    // Delete the interpolator and accelerator
    gsl_interp_free(interpolator);
    gsl_interp_accel_free(accelerator);

    return yi;
}
