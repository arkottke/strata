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

#ifndef MODULUS_FACTORY_H
#define MODULUS_FACTORY_H

#include "AbstractNonlinearPropertyFactory.h"

class ModulusFactory : public AbstractNonlinearPropertyFactory
{
    Q_OBJECT

public:
    ModulusFactory(QObject *parent = 0);
};

#endif // MODULUSREDUCTIONNONLINEARPROPERTYFACTORY_H
