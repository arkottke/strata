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

#include "StrainTransferFunctionOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "Algorithms.h"
#include "Dimension.h"
#include "LinearOutputInterpolater.h"
#include "MyQwtCompatibility.h"
#include "OutputCatalog.h"
#include "SoilProfile.h"

#include <qwt_scale_engine.h>

StrainTransferFunctionOutput::StrainTransferFunctionOutput(OutputCatalog* catalog)
    : AbstractRatioOutput(catalog)
{
    _interp = new LinearOutputInterpolater;
}

bool StrainTransferFunctionOutput::needsFreq() const
{
    return true;
}

QString StrainTransferFunctionOutput::name() const
{
    return tr("Strain Transfer Function");
}

QString StrainTransferFunctionOutput::shortName() const
{
    return tr("strainTf");
}


QwtScaleEngine* StrainTransferFunctionOutput::xScaleEngine() const
{
    return logScaleEngine();
}

QwtScaleEngine* StrainTransferFunctionOutput::yScaleEngine() const
{
    return logScaleEngine();
}

const QString StrainTransferFunctionOutput::xLabel() const
{
    return tr("Frequency (Hz");
}

const QString StrainTransferFunctionOutput::yLabel() const
{
    return tr("FAS (strain) at %1 / FAS (accel) at %2")
            .arg(locationToString(_outDepth))
            .arg(locationToString(_inDepth));
}

const QVector<double>& StrainTransferFunctionOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return _catalog->frequency()->data();
}

void StrainTransferFunctionOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    const Location inLoc = calculator->site()->depthToLocation(_inDepth);
    const Location outLoc = calculator->site()->depthToLocation(_outDepth);

    ref = calculator->motion()->freq();
    QVector<std::complex<double> > tf = calculator->calcStrainTf(
            inLoc, _inType, outLoc);

    data.clear();

    foreach (std::complex<double> c, tf)
        data << abs(c);
}
