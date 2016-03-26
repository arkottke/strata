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

#ifndef MODULUS_STANDARD_DEVIATION_H
#define MODULUS_STANDARD_DEVIATION_H

#include "AbstractNonlinearPropertyStandardDeviation.h"

class ModulusStandardDeviation : public AbstractNonlinearPropertyStandardDeviation
{
    Q_OBJECT
public:
    explicit ModulusStandardDeviation(QObject *parent = 0);

    virtual double calculate(NonlinearPropertyRandomizer::Model model, double strain, double property);

public slots:
    virtual void setPropertyValue(double value);
    virtual void setModel(NonlinearPropertyRandomizer::Model model);
};

#endif // MODULUS_STANDARD_DEVIATION_H
