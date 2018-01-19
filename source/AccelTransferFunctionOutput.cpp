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

#include "AccelTransferFunctionOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Algorithms.h"
#include "Dimension.h"
#include "LinearOutputInterpolater.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"

#include <qwt_scale_engine.h>

AccelTransferFunctionOutput::AccelTransferFunctionOutput(OutputCatalog* catalog)
    : AbstractRatioOutput(catalog)
{
    _interp = new LinearOutputInterpolater;
}

bool AccelTransferFunctionOutput::needsFreq() const
{
    return true;
}

QString AccelTransferFunctionOutput::name() const
{
    return tr("Acceleration Transfer Function");
}

QString AccelTransferFunctionOutput::shortName() const
{
    return tr("accelTf");
}

QwtScaleEngine* AccelTransferFunctionOutput::xScaleEngine() const
{
    return logScaleEngine();
}

QwtScaleEngine* AccelTransferFunctionOutput::yScaleEngine() const
{
    return new QwtLinearScaleEngine;
}

const QString AccelTransferFunctionOutput::xLabel() const
{
    return tr("Frequency (Hz)");
}

const QString AccelTransferFunctionOutput::yLabel() const
{
    return tr("FAS (accel) at %1 / FAS (accel) at %2")
            .arg(locationToString(_outDepth))
            .arg(locationToString(_inDepth));
}

const QVector<double>& AccelTransferFunctionOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return _catalog->frequency()->data();
}

void AccelTransferFunctionOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const Location inLoc = calculator->site()->depthToLocation(_inDepth);
    const Location outLoc = calculator->site()->depthToLocation(_outDepth);

    ref = calculator->motion()->freq();
    QVector<std::complex<double> > tf = calculator->calcAccelTf(
            inLoc, _inType, outLoc, _outType);

    data.clear();

    for (const std::complex<double> &c : tf)
        data << abs(c);
}
