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

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

#include <QString>
#include <QVector>

//! Various algorithms that are used repeatedly

//! Interpolate in linear space
auto interp(const QVector<double> &x, const QVector<double> &y,
            const QVector<double> &xi) -> QVector<double>;

//! Interpolate in log-log space
auto logLoginterp(const QVector<double> &x, const QVector<double> &y,
                  const QVector<double> &xi) -> QVector<double>;

//! Return the string corresponding to a boolean
auto boolToString(bool b) -> QString;

//! Convert the location to a string, -1 converts to Bedrock
auto locationToString(double loc) -> QString;

#endif // ALGORITHMS_H_
