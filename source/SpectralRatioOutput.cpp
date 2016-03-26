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

#include "SpectralRatioOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Algorithms.h"
#include "Dimension.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"

#include <QDebug>

#include <qwt/qwt_scale_engine.h>

SpectralRatioOutput::SpectralRatioOutput(OutputCatalog* catalog)
    : AbstractRatioOutput(catalog)
{
}

bool SpectralRatioOutput::needsPeriod() const
{
    return true;
}

QString SpectralRatioOutput::name() const
{
    return tr("Spectral Ratio");
}

QString SpectralRatioOutput::shortName() const
{
    return tr("specRatio");
}

QwtScaleEngine* SpectralRatioOutput::xScaleEngine() const
{
    return logScaleEngine();
}

QwtScaleEngine* SpectralRatioOutput::yScaleEngine() const
{
    QwtLinearScaleEngine* scaleEngine = new QwtLinearScaleEngine;
    scaleEngine->setAttribute(QwtScaleEngine::IncludeReference, true);

    return scaleEngine;
}

const QString SpectralRatioOutput::xLabel() const
{
    return tr("Period (s)");
}

const QString SpectralRatioOutput::yLabel() const
{
    return tr("Sa at %1 / Sa at %2")
            .arg(locationToString(m_outDepth))
            .arg(locationToString(m_inDepth));
}

const QVector<double>& SpectralRatioOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return m_catalog->period()->data();
}

void SpectralRatioOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    const Location inLoc = calculator->site()->depthToLocation(m_inDepth);
    const Location outLoc = calculator->site()->depthToLocation(m_outDepth);

    const QVector<double> inSa = calculator->motion()->computeSa(
            m_catalog->period()->data(), m_catalog->damping(),
            calculator->calcAccelTf(
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    inLoc, m_inType));

    const QVector<double> outSa = calculator->motion()->computeSa(
            m_catalog->period()->data(), m_catalog->damping(),
            calculator->calcAccelTf(
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    outLoc, m_outType));

    // Compute the ratio
    data.resize(m_catalog->period()->size());

    for (int i = 0; i < data.size(); ++i)
        data[i] = outSa.at(i) / inSa.at(i);
}
