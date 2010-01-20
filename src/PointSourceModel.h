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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef POINT_SOURCE_MODEL_H_
#define POINT_SOURCE_MODEL_H_

#include <QObject>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>

class PointSourceModel : public QObject
{
    Q_OBJECT 

    public:
        //! Location of model
        enum Location {
            Custom, //!< Custom location
            WUS, //!< Western North America
            CEUS //!< Eastern North America
        };

        PointSourceModel( QObject * parent = 0 );

        Location location() const;

        static QStringList locationList();

        QString toHtml();

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

        bool siteSpecificCrustalAmp() const;

        QVector<double> & freq();
        QVector<double> & crustAmp();

        QVector<double> & crustThickness();
        QVector<double> & crustVelocity();
        QVector<double> & crustDensity();

        bool modified() const;

        QVector<double> calcFourierSpectrum( const QVector<double> & freq );
        
        //! Calculate the crustal amplification
        void calcCrustalAmp();
        
        //! Convert the motion to a QMap for saving
        QMap<QString, QVariant> toMap() const;

        //! Load the motion from a QMap
        void fromMap( const QMap<QString, QVariant> & map);

    signals:
        void hypoDistanceChanged(double hypoDistance);
        void pathDurCoeffChanged(double pathDurCoeff);
        void durationChanged(double duration);
        void geoAttenChanged(double geoAtten);

        void crustalAmpChanged();
        void crustalVelChanged();

    public slots:
        void setLocation(Location location);
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
        void setSiteAttenuation(double siteAttenuation);
        void setSiteSpecificCrustalAmp(bool siteSpecificCrustalAmp);

        void setCrustAmpNeedsUpdate();

    private:
        //! Calculate the hypocentral distance
        void calcHypoDistance();

        //! Calculate the corner frequency
        void calcCornerFreq();

        //! Calculate the duration
        void calcDuration();

        //! Calculate the geometric attenuation
        void calcGeoAtten();

        /*! Compute the average value of a property to a max depth.
         * \param thickness vector of the layer thicknesses
         * \param property vector of the property of interest
         * \param maxDepth depth to which the average is computed
         * \return average value
         */
        static double averageValue( const QVector<double> & thickness, const QVector<double> & property, const double maxDepth );

        /*! Location for the parameters.
         * Loads default values from Campbell 2003
         */
        Location m_location;

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

        /*! Duration of the event.
         * Combination of the source and path durations.
         */
        double m_duration;
        
        /*! Modified
         */
        bool m_modified;

        //! Site amplification
        //@{
        //! Frequency
        QVector<double> m_freq;
        
        //! Amplification
        QVector<double> m_crustAmp;
        //@}
        
        //! Site specific crustal amplification based on crustal model.
        bool m_siteSpecificCrustalAmp;

        //! Calculation of the crustal amplification is required.
        bool m_crustAmpNeedsUpdate;
        
        /*! Crustal model.
         * The crustal model is used for development of site specific crustal
         * amplification.
         */
        //@{

        //! Thickness of the crustal layers
        QVector<double> m_crustThickness;
        
        //! Shear-wave velocity of the crustal layers in km/sec
        QVector<double> m_crustVelocity;

        //! Density of the crustal layers in grams/cm^3
        QVector<double> m_crustDensity;
        //@}
};
#endif

