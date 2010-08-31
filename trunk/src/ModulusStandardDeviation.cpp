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

#include "ModulusStandardDeviation.h"

#include <cmath>

ModulusStandardDeviation::ModulusStandardDeviation(QObject *parent) :
    AbstractNonlinearPropertyStandardDeviation(parent)
{
    m_min = 0.1;
    m_max = 1.0;
}

double ModulusStandardDeviation::calculate(NonlinearPropertyRandomizer::Model model, double strain, double property)
{
    if (model == NonlinearPropertyRandomizer::Darendeli) {
        return exp(-4.23) + sqrt(0.25 / exp(3.62) -
                                  pow(property - 0.5, 2)/exp(3.62));
    } else {
        return AbstractNonlinearPropertyStandardDeviation::calculate(model, strain, property);
    }
}

void ModulusStandardDeviation::setPropertyValue(double value)
{
    m_engine.globalObject().setProperty("modulus", value);
}

void ModulusStandardDeviation::setModel(NonlinearPropertyRandomizer::Model model)
{
    if (model == NonlinearPropertyRandomizer::Darendeli)
        setFunction("Math.exp(-4.23) + Math.sqrt(0.25 / Math.exp(3.62) - Math.pow(modulus - 0.5, 2) / Math.exp(3.62))");
}
