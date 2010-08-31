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

#include "DampingStandardDeviation.h"

#include <cmath>

DampingStandardDeviation::DampingStandardDeviation(QObject *parent) :
    AbstractNonlinearPropertyStandardDeviation(parent)
{
    m_min = 0.2;
    m_max = 15.0;
}

double DampingStandardDeviation::calculate(NonlinearPropertyRandomizer::Model model, double strain, double property)
{
    if (model == NonlinearPropertyRandomizer::Darendeli) {
        return exp(-5) + exp(-0.25) * sqrt(property);
    } else {
        return AbstractNonlinearPropertyStandardDeviation::calculate(model, strain, property);
    }
}

void DampingStandardDeviation::setPropertyValue(double value)
{
    m_engine.globalObject().setProperty("damping", value);
}

void DampingStandardDeviation::setModel(NonlinearPropertyRandomizer::Model model)
{
    if (model == NonlinearPropertyRandomizer::Darendeli)
        setFunction("Math.exp(-5) + Math.exp(-0.25) * Math.sqrt(damping)");
}
