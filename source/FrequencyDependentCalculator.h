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

#ifndef FREQUENCY_DEPENDENT_CALCULATOR_H
#define FREQUENCY_DEPENDENT_CALCULATOR_H

#include "AbstractIterativeCalculator.h"

#include <QDataStream>
#include <QJsonObject>

class FrequencyDependentCalculator : public AbstractIterativeCalculator
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out,
                                     const FrequencyDependentCalculator* fdc) -> QDataStream &;
    friend auto operator>> (QDataStream & in,
                                     FrequencyDependentCalculator* fdc) -> QDataStream &;

public:
    explicit FrequencyDependentCalculator(QObject *parent = nullptr);

    virtual auto toHtml() const -> QString;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

    auto useSmoothSpectrum() -> bool;

signals:
    void useSmoothSpectrumChanged(bool b);

public slots:
    void setUseSmoothSpectrum(bool b);

protected:
    virtual auto updateSubLayer(int index, const QVector<std::complex<double> > &strainTf) -> bool;
    virtual void estimateInitialStrains();

    bool _useSmoothSpectrum;
};

#endif // FREQUENCY_DEPENDENT_CALCULATOR_H
