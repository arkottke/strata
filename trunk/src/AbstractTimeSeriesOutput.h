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

#ifndef ABSTRACT_TIME_SERIES_OUTPUT_H
#define ABSTRACT_TIME_SERIES_OUTPUT_H

#include "AbstractLocationOutput.h"

class OutputCatalog;

class AbstractTimeSeriesOutput : public AbstractLocationOutput
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractTimeSeriesOutput* atso);
    friend QDataStream & operator>> (QDataStream & in, AbstractTimeSeriesOutput* atso);

public:
    explicit AbstractTimeSeriesOutput(OutputCatalog* catalog);

    virtual QString fullName() const;

    virtual bool needsTime() const;

    bool baselineCorrect() const;

public slots:
    void setBaselineCorrect(bool baseLineCorrect);

signals:
    void baselineCorrectChanged(int baseLineCorrect);

protected:
    virtual QString fileName(int motion = 0) const;
    virtual QwtScaleEngine* xScaleEngine() const;
    virtual QwtScaleEngine* yScaleEngine() const;
    virtual const QString xLabel() const;
    virtual const QVector<double>& ref(int motion) const;
    virtual const QString suffix() const;

    //! Field width of the motion index
    const int fieldWidth() const;

    //! If the time series is baseline corrected
    bool m_baselineCorrect;

};
#endif // ABSTRACT_TIME_SERIES_OUTPUT_H
