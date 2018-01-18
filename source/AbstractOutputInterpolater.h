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

#ifndef ABSTRACT_OUTPUT_INTERPOLATER_H
#define ABSTRACT_OUTPUT_INTERPOLATER_H

#include <QVector>

class AbstractOutputInterpolater
{
public:
    AbstractOutputInterpolater();
    virtual ~AbstractOutputInterpolater();

    virtual QVector<double> calculate(const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi) = 0;
};

#endif // ABSTRACT_OUTPUT_INTERPOLATER_H
