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

#ifndef RESPONSE_SPECTRUM_OUTPUT_H
#define RESPONSE_SPECTRUM_OUTPUT_H

#include "AbstractLocationOutput.h"

class AbstractCalculator;

class ResponseSpectrumOutput : public AbstractLocationOutput
{
Q_OBJECT
public:
    explicit ResponseSpectrumOutput(OutputCatalog* catalog);

    virtual auto needsPeriod() const -> bool;
    virtual auto name() const -> QString;
protected:
    virtual auto shortName() const -> QString;
    virtual auto xScaleEngine() const -> QwtScaleEngine*;
    virtual auto yScaleEngine() const -> QwtScaleEngine*;
    virtual auto xLabel() const -> const QString;
    virtual auto yLabel() const -> const QString;
    virtual auto ref(int motion = 0) const -> const QVector<double>&;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;

};


#endif // RESPONSE_SPECTRUM_OUTPUT_H
