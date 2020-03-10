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

#ifndef ABSTRACT_PROFILE_OUTPUT_H
#define ABSTRACT_PROFILE_OUTPUT_H

#include "AbstractOutput.h"

#include <QDataStream>
#include <QJsonObject>

class OutputStatistics;
class OutputCatalog;

class AbstractProfileOutput : public AbstractOutput
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractProfileOutput* ao) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractProfileOutput* ao) -> QDataStream &;

public:
    enum CurveType{
        Yfx, //!< Y data is a function of x data
        Xfx  //!< X data is a function of y data
    };

    explicit AbstractProfileOutput(OutputCatalog* catalog, bool interpolated = true);

    virtual auto fullName() const -> QString;
    virtual auto needsDepth() const -> bool;

    auto enabled() const -> bool;
    void setEnabled(bool enabled);

    virtual auto curveType() const -> AbstractOutput::CurveType;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

protected:
    auto fileName(int motion = 0) const -> QString;
    virtual auto xScaleEngine() const -> QwtScaleEngine*;
    virtual auto yScaleEngine() const -> QwtScaleEngine*;
    virtual auto yLabel() const -> const QString;
    virtual auto ref(int motion = 0) const -> const QVector<double>&;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;

    //! Extrapolate the data -- used for the final point in a strain based profiles
    void extrap(const QVector<double> & ref, QVector<double> & data, double layerThickness) const;

    //! If the output is enabled
    bool _enabled;
};

#endif // ABSTRACT_PROFILE_OUTPUT_H
