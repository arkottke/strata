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

#include "MaxDispProfileOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "Units.h"

MaxDispProfileOutput::MaxDispProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{

}

QString MaxDispProfileOutput::name() const
{
    return tr("Peak Ground Displacement Profile");
}

QString MaxDispProfileOutput::shortName() const
{
    return tr("pgd");
}

const QString MaxDispProfileOutput::xLabel() const
{
    return tr("Maximum Displacement (%1)").arg(Units::instance()->dispTs());
}


void MaxDispProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    AbstractProfileOutput::extract(calculator, ref, data);

    data = calculator->maxDispProfile();
}
