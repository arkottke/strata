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

#include "Algorithms.h"

#include "Units.h"

#include <QDebug>
#include <QObject>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_math.h>


QVector<double> interp( const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi )
{
    Q_ASSERT_X( x.size() == y.size(), "Algorithms::interp", "input x and y vectors must be same size" );

    QVector<double> yi(xi.size());

    gsl_interp * interpolator = gsl_interp_alloc(gsl_interp_linear, x.size());
    gsl_interp_init( interpolator, x.data(), y.data(), x.size() );
    gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

    for ( int i = 0; i < xi.size(); ++i ) {
        // Interpolate
        yi[i] = gsl_interp_eval( interpolator, x.data(), y.data(), xi.at(i), accelerator);
    }

    // Clean up the interpolating objects
    gsl_interp_free( interpolator );
    gsl_interp_accel_free( accelerator );

    return yi;
}

QVector<double> logLoginterp( const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi )
{
    QVector<double> logX(x.size());
    QVector<double> logY(x.size());
    QVector<double> logXi(xi.size());

    for (int i = 0; i < x.size(); ++i) {
        logX[i] = log(x.at(0));
        logY[i] = log(y.at(0));
    }
    
    for (int i = 0; i < xi.size(); ++i) {
        logXi[i] = log(xi.at(0));
    }
    
    QVector<double> logYi = interp( logX, logY, logXi );

    QVector<double> yi(logYi.size());

    for (int i = 0; i < logYi.size(); ++i) {
        yi[i] = exp( logYi.at(i) );
    }

    return yi;
}

QString boolToString(bool b)
{
    QString str;

    if ( b )
        str = QObject::tr("True");
    else
        str = QObject::tr("False");

    return str;
}

QString locationToString(double loc)
{
    if (loc < 0) {
        return QObject::tr("Bedrock");
    } else {
        return QString("%1 %2")
                .arg(loc, 0, 'f', 2)
                .arg(Units::instance()->length());
    }
}
