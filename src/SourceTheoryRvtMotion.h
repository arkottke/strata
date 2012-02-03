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

#ifndef SOURCE_THEORY_RVT_MOTION_H
#define SOURCE_THEORY_RVT_MOTION_H

#include "AbstractRvtMotion.h"

#include <QDataStream>

#include <gsl/gsl_interp.h>

class CrustalAmplification;
class Dimension;

class SourceTheoryRvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const SourceTheoryRvtMotion* strm);
    friend QDataStream & operator>> (QDataStream & in, SourceTheoryRvtMotion* strm);

public:
    SourceTheoryRvtMotion(QObject * parent = 0);
    virtual ~SourceTheoryRvtMotion();

    //! Location of model
    enum Model {
        Custom, //!< Custom location
        WUS, //!< Generic Western North America Parameters
        CEUS //!< Generic Eastern North America Parameters
    };

    static QStringList sourceList();


    virtual const QVector<double> & freq() const;
    Dimension* freqDimension();

    QString nameTemplate() const;
    virtual QString name() const;

    virtual QString toHtml() const;
    virtual bool loadFromTextStream(QTextStream &stream);

    Model model() const;
    void setModel(Model s);

    double momentMag() const;
    double distance() const;
    double depth() const;
    double hypoDistance() const;
    double stressDrop() const;
    double geoAtten() const;
    double pathDurCoeff() const;
    double pathAttenCoeff() const;
    double pathAttenPower() const;
    double shearVelocity() const;
    double density() const;
    double siteAtten() const;
    double duration() const;

    //! Site amplification
    CrustalAmplification* crustalAmp();

signals:
    void isCustomizeable(bool b);

    void momentMagChanged(double d);
    void stressDropChanged(double d);
    void geoAttenChanged(double d);
    void pathDurCoeffChanged(double d);
    void pathAttenCoeffChanged(double d);
    void pathAttenPowerChanged(double d);
    void shearVelocityChanged(double d);
    void densityChanged(double d);
    void siteAttenChanged(double d);
    void durationChanged(double d);

    void hypoDistanceChanged(double hypoDistance);    

public slots:
    void setModel(int s);
    void setMomentMag(double momentMag);
    void setDistance(double distance);
    void setDepth(double depth);
    void setStressDrop(double stressDrop);
    void setPathDurCoeff(double pathDurCoeff);
    void setGeoAtten(double geoAtten);
    void setPathAttenCoeff(double pathAttenCoeff);
    void setPathAttenPower(double pathAttenPower);
    void setShearVelocity(double shearVelocity);
    void setDensity(double density);
    void setSiteAtten(double siteAtten);

    //! Calculate the FAS and associated response spectrum
    virtual void calculate();

private:
    //! Calculate the hypocentral distance
    void calcHypoDistance();

    //! Calculate the corner frequency
    void calcCornerFreq();

    //! Calculate the duration
    void calcDuration();

    //! Calculate the geometric attenuation
    void calcGeoAtten();

    /*! Model of the parameters.
     * Loads default values from Campbell 2003
     */
    Model m_model;

    //! Moment magnitude
    double m_momentMag;

    /*! Seismic moment.
     * The seismic moment is calculated from the moment magnitude.
     */
    double m_seismicMoment;

    //! Epicentral distance in km
    double m_distance;

    //! Depth in km
    double m_depth;

    //! Hypocentral distance in km
    double m_hypoDistance;

    /*! Corner frequency.
     * The corner frequency is calculated from the stress drop and
     * seismic moment.
     */
    double m_cornerFreq;

    //! Stress drop in bars
    double m_stressDrop;

    //! Geometric attenuation
    double m_geoAtten;

    //! Path duration coefficient
    double m_pathDurCoeff;

    //! Path attenuation coefficient
    double m_pathAttenCoeff;

    //! Path attenuation power
    double m_pathAttenPower;

    //! Shear-velociy in km/sec
    double m_shearVelocity;

    //! Density in gm/cm^3
    double m_density;

    //! Site attenuation
    double m_siteAtten;

    //! Amplification by site effects (changes in density and shear-wave velocity in the crustal)
    CrustalAmplification *m_crustalAmp;

    //! Frequency dimension
    Dimension* m_freq;
};

#endif // SOURCE_THEORY_RVT_MOTION_H
