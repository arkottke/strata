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

#include "SteppedOutputInterpolater.h"

#include <QDebug>

SteppedOutputInterpolater::SteppedOutputInterpolater()
{
}

auto SteppedOutputInterpolater::calculate(
        const QVector<double> & x, const QVector<double> & y, const QVector<double> & xi) -> QVector<double>
{
    // x is at the base of the layer
    Q_ASSERT(x.size() <= y.size());

    // Interpolated data
    QVector<double> yi;

    int j = 0;
    for (int i = 0; i < xi.size(); ++i) {
        // Adjust j -- may skip points if the step of xi is too great
        while (j < x.size() && x.at(j) <= xi.at(i))
            ++j;

        if (j < x.size()) {
            yi << y.at(j);
        } else {
            break;
        }
    }

    // Add the bedrock values to the end
    if (x.size() < y.size())
        yi << y.last();

    return yi;
}
