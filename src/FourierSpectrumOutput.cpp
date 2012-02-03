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

#include "FourierSpectrumOutput.h"

#include "AbstractCalculator.h"
#include "Dimension.h"
#include "LinearOutputInterpolater.h"
#include "AbstractMotion.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilProfile.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

#include <gsl/gsl_interp.h>

#include <qwt_scale_engine.h>

FourierSpectrumOutput::FourierSpectrumOutput(OutputCatalog* catalog)
    : AbstractLocationOutput(catalog)
{
    m_interp = new LinearOutputInterpolater;

    m_statistics = new OutputStatistics(this);
    connect(m_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}

bool FourierSpectrumOutput::needsFreq() const
{
    return true;
}

QString FourierSpectrumOutput::name() const
{
    return tr("Fourier Amplitude Spectrum");
}

QString FourierSpectrumOutput::shortName() const
{
    return tr("fas");
}

QwtScaleEngine* FourierSpectrumOutput::xScaleEngine() const
{
    return new QwtLog10ScaleEngine;
}

QwtScaleEngine* FourierSpectrumOutput::yScaleEngine() const
{
    return new QwtLog10ScaleEngine;
}

const QString FourierSpectrumOutput::xLabel() const
{
    return tr("Frequency (Hz)");
}

const QString FourierSpectrumOutput::yLabel() const
{
    // FIXME should scale to cm/s or in/s
    return tr("|FAS| (%1/s)").arg(Units::instance()->accel());
}

const QVector<double>& FourierSpectrumOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return m_catalog->frequency()->data();
}


void FourierSpectrumOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    // Layer associated with the depth
    const Location loc = calculator->site()->depthToLocation(m_depth);

    ref = calculator->motion()->freq();

    data = calculator->motion()->absFourierAcc(
            calculator->calcAccelTf(
                    // Input parameters
                    calculator->site()->inputLocation(), calculator->motion()->type(),
                    // Output parameters
                    loc, m_type));

    if (qobject_cast<TimeSeriesMotion*>(calculator->motion())) {
        // Apply a little bit of smoothing
        const int window = 5;

        // Smoothed Fourier amplitudes
        QVector<double> smooth(data.size());

        for (int i = 0; i < smooth.size(); ++i) {
            // Adjust the window size based on the position
            const int _window = qMin(window, qMin(2 * i + 1, 2 * (smooth.size() - 1 - i) + 1));

            double sum = 0;
            for (int j = i - _window / 2; j <= i + _window / 2; ++j)
                sum += data.at(j);

            smooth[i] = sum / _window;
        }
        data = smooth;
    }    
}
