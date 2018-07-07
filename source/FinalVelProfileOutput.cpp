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

#include "FinalVelProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "Units.h"

FinalVelProfileOutput::FinalVelProfileOutput(OutputCatalog* catalog)
    : AbstractSteppedProfileOutput(catalog)
{

}

QString FinalVelProfileOutput::name() const
{
    return tr("Final Shear-Wave Velocity Profile");
}

QString FinalVelProfileOutput::shortName() const
{
    return tr("finalVs");
}

const QString FinalVelProfileOutput::xLabel() const
{
    return tr("Final Shear-Wave Velocity (%1)").arg(Units::instance()->vel());
}

void FinalVelProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    AbstractSteppedProfileOutput::extract(calculator, ref, data);

    data = calculator->site()->finalVelocityProfile();
}
