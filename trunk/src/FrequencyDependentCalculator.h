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

#ifndef FREQUENCY_DEPENDENT_CALCULATOR_H
#define FREQUENCY_DEPENDENT_CALCULATOR_H

#include "AbstractIterativeCalculator.h"

#include <QDataStream>

class FrequencyDependentCalculator : public AbstractIterativeCalculator
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out,
                                     const FrequencyDependentCalculator* fdc);
    friend QDataStream & operator>> (QDataStream & in,
                                     FrequencyDependentCalculator* fdc);

public:
    explicit FrequencyDependentCalculator(QObject *parent = 0);

    virtual QString toHtml() const;

protected:
    virtual bool updateSubLayer(int index, const QVector<std::complex<double> > strainTf);
};

#endif // FREQUENCY_DEPENDENT_CALCULATOR_H
