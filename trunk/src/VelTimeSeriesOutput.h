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

#ifndef VEL_TIME_SERIES_OUTPUT_H
#define VEL_TIME_SERIES_OUTPUT_H

#include "AbstractTimeSeriesOutput.h"

class OutputCatalog;
class AbstractCalculator;

class VelTimeSeriesOutput : public AbstractTimeSeriesOutput
{
    Q_OBJECT
public:
    explicit VelTimeSeriesOutput(OutputCatalog* catalog);

    QString name() const;
protected:
    QString shortName() const;
    const QString yLabel() const;

    void extract(AbstractCalculator* const calculator,
                 QVector<double> & ref, QVector<double> & data) const;
};

#endif // VEL_TIME_SERIES_OUTPUT_H
