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

#ifndef ACCEL_TRANSFER_FUNCTION_OUTPUT_H
#define ACCEL_TRANSFER_FUNCTION_OUTPUT_H

#include "AbstractRatioOutput.h"

class AbstractCalculator;
class OutputCatalog;

class AccelTransferFunctionOutput : public AbstractRatioOutput
{
Q_OBJECT
public:
    explicit AccelTransferFunctionOutput(OutputCatalog* catalog);

    auto name() const -> QString;
    auto needsFreq() const -> bool;

protected:
    auto shortName() const -> QString;

    auto xScaleEngine() const -> QwtScaleEngine*;
    auto yScaleEngine() const -> QwtScaleEngine*;
    auto xLabel() const -> const QString;
    auto yLabel() const -> const QString;
    auto ref(int motion = 0) const -> const QVector<double>&;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;
};
#endif // ACCEL_TRANSFER_FUNCTION_OUTPUT_H
