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

    ModulusFactory* const modulusFactory();
    DampingFactory* const dampingFactory();

    //! Save the defined curves
    bool save() const;

protected:
    ModulusFactory *m_modulusFactory;
    DampingFactory *m_dampingFactory;

    //! File for saving the user create models
    QString m_fileName;
};


#endif // NONLINEAR_PROPERTY_CATALOG_H
