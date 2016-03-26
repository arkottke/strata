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

#include "ResponseSpectrumOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Dimension.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilProfile.h"
#include "Units.h"

#include <QDebug>

ResponseSpectrumOutput::ResponseSpectrumOutput(OutputCatalog* catalog)
    : AbstractLocationOutput(catalog)
{    
    m_statistics = new OutputStatistics(this);
    connect(m_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}

bool ResponseSpectrumOutput::needsPeriod() const
{
    return true;
}

QString ResponseSpectrumOutput::name() const
{
    return tr("Acceleration Response Spectrum");
}

QString ResponseSpectrumOutput::shortName() const
{
    return tr("respSpec");
}

QwtScaleEngine* ResponseSpectrumOutput::xScaleEngine() const
{
    return logScaleEngine();
}

QwtScaleEngine* ResponseSpectrumOutput::yScaleEngine() const
{
    return logScaleEngine();
}

const QString ResponseSpectrumOutput::xLabel() const
{
    return tr("Period (s)");
}

const QString ResponseSpectrumOutput::yLabel() const
{
    return tr("Spectral Accel. (g)");
}

const QVector<double>& ResponseSpectrumOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return m_catalog->period()->data();
}

void ResponseSpectrumOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(ref);

    data = calculator->motion()->computeSa(
            m_catalog->period()->data(), m_catalog->damping(),
            calculator->calcAccelTf(
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    calculator->site()->depthToLocation(m_depth), m_type));
}
