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

#ifndef MAX_DISP_PROFILE_OUTPUT_H
#define MAX_DISP_PROFILE_OUTPUT_H

#include "AbstractProfileOutput.h"

class AbstractCalculator;

class MaxDispProfileOutput : public AbstractProfileOutput
{
Q_OBJECT
public:
    explicit MaxDispProfileOutput(OutputCatalog* catalog);

    virtual QString name() const;
protected:
    virtual QString shortName() const;
    virtual const QString xLabel() const;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;
};

#endif // MAX_DISP_PROFILE_OUTPUT_H
