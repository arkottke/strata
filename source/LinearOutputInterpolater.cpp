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

#include "LinearOutputInterpolater.h"

#include <QDebug>

#include <gsl/gsl_interp.h>

#include <float.h>

LinearOutputInterpolater::LinearOutputInterpolater()
{
}

QVector<double> LinearOutputInterpolater::calculate(
        const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi)
{
    QVector<double> yi;

    // Ensure that x is increasing, if not nudge it.
    // FIXME: Better solution?
    QVector<double> xm(x);
    for (int i = 0; i < (xm.size() - 1); ++i) {
        if (xm.at(i + 1) == xm.at(i)) {
            qDebug() << i;
            xm[i + 1] *= 1 + 1E-6;
        }
    }

    // Allocate the interpolator
    gsl_interp* interpolator = gsl_interp_alloc(gsl_interp_linear, y.size());
    gsl_interp_init(interpolator, xm.data(), y.data(), y.size());
    gsl_interp_accel* accelerator =  gsl_interp_accel_alloc();

    for (int i = 0; i < xi.size(); ++i) {
        if (xi.at(i) < xm.last()) {
            yi << gsl_interp_eval(interpolator, xm.data(), y.data(), xi.at(i), accelerator);
        } else {
            break;
        }
    }

    // Delete the interpolator and accelerator
    gsl_interp_free(interpolator);
    gsl_interp_accel_free(accelerator);

    return yi;
}
