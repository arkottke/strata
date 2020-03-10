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

#ifndef NONLINEAR_PROPERTY_CATALOG_H
#define NONLINEAR_PROPERTY_CATALOG_H

#include <QString>

class ModulusFactory;
class DampingFactory;

class NonlinearPropertyCatalog
{

public:
    NonlinearPropertyCatalog();
    ~NonlinearPropertyCatalog();

    auto modulusFactory() -> ModulusFactory*;
    auto dampingFactory() -> DampingFactory*;

    //! Save the defined curves
    auto save() const -> bool;

protected:
    ModulusFactory *_modulusFactory;
    DampingFactory *_dampingFactory;

    //! File for saving the user create models
    QString _fileName;
};


#endif // NONLINEAR_PROPERTY_CATALOG_H
