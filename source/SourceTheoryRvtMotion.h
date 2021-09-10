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

#ifndef SOURCE_THEORY_RVT_MOTION_H
#define SOURCE_THEORY_RVT_MOTION_H

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>

#include <gsl/gsl_interp.h>

class CrustalAmplification;
class Dimension;
class PathDurationModel;

class SourceTheoryRvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const SourceTheoryRvtMotion* strm) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SourceTheoryRvtMotion* strm) -> QDataStream &;

public:
    SourceTheoryRvtMotion(QObject *parent = nullptr);
    virtual ~SourceTheoryRvtMotion();

    virtual void setRegion(AbstractRvtMotion::Region region);

    virtual auto freq() const -> const QVector<double> &;
    auto freqDimension() -> Dimension*;

    virtual auto toHtml() const -> QString;

    auto isCustomized() const -> bool;
    auto depth() const -> double;
    auto hypoDistance() const -> double;
    auto stressDrop() const -> double;
    auto geoAtten() const -> double;
    auto pathAttenCoeff() const -> double;
    auto pathAttenPower() const -> double;
    auto shearVelocity() const -> double;
    auto density() const -> double;
    auto siteAtten() const -> double;
    auto duration() const -> double;

    //! Crustal amplification
    auto crustalAmp() -> CrustalAmplification*;

    //! Path duration model
    auto pathDuration() -> PathDurationModel*;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

signals:
    void isCustomizedChanged(bool b);

    void stressDropChanged(double d);
    void geoAttenChanged(double d);
    void pathAttenCoeffChanged(double d);
    void pathAttenPowerChanged(double d);
    void shearVelocityChanged(double d);
    void densityChanged(double d);
    void siteAttenChanged(double d);
    void durationChanged(double d);

    void hypoDistanceChanged(double hypoDistance);

public slots:
    void setDensity(double density);
    void setDepth(double depth);
    void setDistance(double dist);
    void setGeoAtten(double geoAtten);
    void setIsCustomized(bool b);
    void setMagnitude(double mag);
    void setStressDrop(double stressDrop);
    void setPathAttenCoeff(double pathAttenCoeff);
    void setPathAttenPower(double pathAttenPower);
    void setShearVelocity(double shearVelocity);
    void setSiteAtten(double siteAtten);

    //! Calculate the FAS and associated response spectrum
    virtual void calculate();

private:
    //! Initialize parameters of the model
    void init();

    //! Calculate the geometric attenuation
    void calcGeoAtten();

    //! If the model is custom
    bool _isCustomized;

    /*! Seismic moment.
     * The seismic moment is calculated from the moment magnitude.
     */
    double _seismicMoment;

    //! Depth in km
    double _depth;

    //! Hypocentral distance in km
    double _hypoDistance;

    /*! Corner frequency.
     * The corner frequency is calculated from the stress drop and
     * seismic moment.
     */
    double _cornerFreq;

    //! Stress drop in bars
    double _stressDrop;

    //! Geometric attenuation
    double _geoAtten;

    //! Path attenuation coefficient
    double _pathAttenCoeff;

    //! Path attenuation power
    double _pathAttenPower;

    //! Shear-velociy in km/sec
    double _shearVelocity;

    //! Density in gm/cm^3
    double _density;

    //! Site attenuation
    double _siteAtten;

    //! Amplification by site effects (changes in density and shear-wave velocity in the crustal)
    CrustalAmplification* _crustalAmp;

    //! Model for the distance dependent duration;
    PathDurationModel* _pathDuration;

    //! Frequency dimension
    Dimension* _freq;
};

#endif // SOURCE_THEORY_RVT_MOTION_H
