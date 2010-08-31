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

#ifndef SPECTRAL_RATIO_OUTPUT_H
#define SPECTRAL_RATIO_OUTPUT_H

#include "AbstractRatioOutput.h"

class OutputCatalog;
class AbstractCalculator;

class SpectralRatioOutput : public AbstractRatioOutput
{
Q_OBJECT
public:
    explicit SpectralRatioOutput(OutputCatalog* catalog);

    virtual bool needsPeriod() const;
    virtual QString name() const;
protected:
    virtual QString shortName() const;
    virtual QwtScaleEngine* xScaleEngine() const;
    virtual QwtScaleEngine* yScaleEngine() const;
    virtual const QString xLabel() const;
    virtual const QString yLabel() const;
    virtual const QVector<double>& ref(int motion = 0) const;   

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;
};

#endif // SPECTRAL_RATIO_OUTPUT_H
