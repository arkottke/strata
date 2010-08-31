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

#include "AbstractSteppedProfileOutput.h"

#include "AbstractCalculator.h"
#include "SoilProfile.h"
#include "SteppedOutputInterpolater.h"
#include "SubLayer.h"
#include "OutputCatalog.h"

#include <qwt_plot_curve.h>

AbstractSteppedProfileOutput::AbstractSteppedProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog)
{
    if (m_interp)
        delete m_interp;

    m_interp = new SteppedOutputInterpolater;
}

void AbstractSteppedProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(data);
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();

    Q_ASSERT(subLayers.size());

    // Populate the reference with the depth to the base of the layers
    ref.clear();

    foreach (const SubLayer & sl, subLayers)
        ref << sl.depthToBase();
}
