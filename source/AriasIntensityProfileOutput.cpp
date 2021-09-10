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

#include "AriasIntensityProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

#include <qwt_scale_engine.h>

AriasIntensityProfileOutput::AriasIntensityProfileOutput(OutputCatalog* catalog)
    : AbstractProfileOutput(catalog, false)
{
}

auto AriasIntensityProfileOutput::name() const -> QString
{
    return tr("Arias Intensity Profile");
}

auto AriasIntensityProfileOutput::shortName() const -> QString
{
    return tr("AriasIntensity");
}

auto AriasIntensityProfileOutput::xLabel() const -> const QString
{
    return tr("Arias Intensity (m/sec)");
}

auto AriasIntensityProfileOutput::xScaleEngine() const -> QwtScaleEngine*
{
    return new QwtLinearScaleEngine;
}

auto AriasIntensityProfileOutput::timeSeriesOnly() const -> bool
{
    return true;
}

void AriasIntensityProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref)

    const auto* tsm = static_cast<const TimeSeriesMotion*>(calculator->motion());
    const SoilProfile* site = calculator->site();

    // Outcrop for the first layer. Within for subsequent.
    AbstractMotion::Type type = AbstractMotion::Outcrop;

    for (const double &depth : this->ref()) {
        data << tsm->ariasIntensity(calculator->calcAccelTf(
                                site->inputLocation(), tsm->type(),
                                        site->depthToLocation(depth), type)).last();
        type = AbstractMotion::Within;
    }
}
