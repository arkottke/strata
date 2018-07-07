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

#ifndef LINEAR_OUTPUT_INTERPOLATER_H
#define LINEAR_OUTPUT_INTERPOLATER_H

#include "AbstractOutputInterpolater.h"

class LinearOutputInterpolater : public AbstractOutputInterpolater
{
public:
    LinearOutputInterpolater();

    QVector<double> calculate(const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi);
};

#endif // LINEAR_OUTPUT_INTERPOLATER_H
