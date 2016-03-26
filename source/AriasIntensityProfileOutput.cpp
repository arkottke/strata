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

#include "AriasIntensityProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

#include <qwt/qwt_scale_engine.h>

AriasIntensityProfileOutput::AriasIntensityProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog, false)
{
    m_offset = 1;
}

QString AriasIntensityProfileOutput::name() const
{
    return tr("Arias Intensity Profile");
}

QString AriasIntensityProfileOutput::shortName() const
{
    return tr("AriasIntensity");
}

const QString AriasIntensityProfileOutput::xLabel() const
{
    return tr("Arias Intensity (m/sec)");
}

QwtScaleEngine* AriasIntensityProfileOutput::xScaleEngine() const
{
    return new QwtLinearScaleEngine;
}

bool AriasIntensityProfileOutput::timeSeriesOnly() const
{
    return true;
}

void AriasIntensityProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref)

    const TimeSeriesMotion* tsm = static_cast<const TimeSeriesMotion*>(calculator->motion());
    const SoilProfile* site = calculator->site();

    // Outcrop for the first layer. Within for subsequent.
    AbstractMotion::Type type = AbstractMotion::Outcrop;

    foreach (double depth, this->ref()) {
        data << tsm->ariasIntensity(calculator->calcAccelTf(
                                site->inputLocation(), tsm->type(),
                                        site->depthToLocation(depth), type)).last();
        type = AbstractMotion::Within;
    }
}
