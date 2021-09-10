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

#ifndef COMPATIBLE_RVT_MOTION_H
#define COMPATIBLE_RVT_MOTION_H

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>

class Dimension;

/*! RvtAbstractMotion defined by a response spectrum.
  */

class CompatibleRvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const CompatibleRvtMotion* crm) -> QDataStream &;
    friend auto operator>> (QDataStream & in, CompatibleRvtMotion* crm) -> QDataStream &;

public:
    CompatibleRvtMotion(QObject *parent = nullptr);
    virtual ~CompatibleRvtMotion();

    virtual auto freq() const -> const QVector<double> &;
    auto freqDimension() -> Dimension*;

    //! Target response spectrum
    auto targetRespSpec() -> ResponseSpectrum *;

    //! Limit the FAS to be seismologically plausible
    auto limitFas() const -> bool;

    //! Create a html document containing the information of the model
    virtual auto toHtml() const -> QString;

    //! Load the motion from a TextStream
    virtual auto loadFromTextStream(QTextStream &stream, double scale = 1.) -> bool;

    virtual void fromJson(const QJsonObject &json);
    virtual auto toJson() const -> QJsonObject;

public slots:
    void setDuration(double duration);
    void setLimitFas(bool limitFas);

    /*! Compute the FAS from a response spectrum.
     * The targetPeriod, targetSa, and targetDamping must be set ahead of calling the inversion
     * \return returns true if the inversion provided a valid result (determined by rmse value)
     */
    virtual void calculate();

private:
    /*! Compute the FAS based on the Vanmarcke 1976 inversion technique.
     * \return Fourier amplitude spectrum spaced at the same periods as the target response spectrum
     */
    auto vanmarckeInversion() const -> QVector<double>;

    //! If the FAS should be corrected to better fit theory
    bool _limitFas;

    //! Target response spectrum
    ResponseSpectrum * _targetRespSpec;

    //! Frequency dimension
    Dimension* _freq;
};

#endif // COMPATIBLE_RVT_MOTION_H
