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

#ifndef LINEAR_ELASTIC_CALCULATOR_H
#define LINEAR_ELASTIC_CALCULATOR_H

#include "AbstractCalculator.h"

class LinearElasticCalculator : public AbstractCalculator
{
    Q_OBJECT

public:
    explicit LinearElasticCalculator(QObject* parent = 0);

    virtual bool run(AbstractMotion* motion, SoilProfile* site);
};

#endif // LINEAR_ELASTIC_CALCULATOR_H
