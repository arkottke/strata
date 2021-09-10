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

#include "SpectralRatioOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Algorithms.h"
#include "Dimension.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"

#include <QDebug>

#include <qwt_scale_engine.h>

SpectralRatioOutput::SpectralRatioOutput(OutputCatalog* catalog)
    : AbstractRatioOutput(catalog)
{
}

auto SpectralRatioOutput::needsPeriod() const -> bool
{
    return true;
}

auto SpectralRatioOutput::name() const -> QString
{
    return tr("Spectral Ratio");
}

auto SpectralRatioOutput::shortName() const -> QString
{
    return tr("specRatio");
}

auto SpectralRatioOutput::xScaleEngine() const -> QwtScaleEngine*
{
    return logScaleEngine();
}

auto SpectralRatioOutput::yScaleEngine() const -> QwtScaleEngine*
{
    auto* scaleEngine = new QwtLinearScaleEngine;
    scaleEngine->setAttribute(QwtScaleEngine::IncludeReference, true);

    return scaleEngine;
}

auto SpectralRatioOutput::xLabel() const -> const QString
{
    return tr("Period (s)");
}

auto SpectralRatioOutput::yLabel() const -> const QString
{
    return tr("Sa at %1 / Sa at %2")
            .arg(locationToString(_outDepth))
            .arg(locationToString(_inDepth));
}

auto SpectralRatioOutput::ref(int motion) const -> const QVector<double>&
{
    Q_UNUSED(motion);

    return _catalog->period()->data();
}

void SpectralRatioOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const Location inLoc = calculator->site()->depthToLocation(_inDepth);
    const Location outLoc = calculator->site()->depthToLocation(_outDepth);

    const QVector<double> inSa = calculator->motion()->computeSa(
            _catalog->period()->data(), _catalog->damping(),
            calculator->calcAccelTf(
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    inLoc, _inType));

    const QVector<double> outSa = calculator->motion()->computeSa(
            _catalog->period()->data(), _catalog->damping(),
            calculator->calcAccelTf(
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    outLoc, _outType));

    // Compute the ratio
    data.resize(_catalog->period()->size());

    for (int i = 0; i < data.size(); ++i)
        data[i] = outSa.at(i) / inSa.at(i);
}
