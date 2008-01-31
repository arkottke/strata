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

#ifndef SITE_PROFILE_H_
#define SITE_PROFILE_H_

#include "SoilType.h"
#include "SoilLayer.h"
#include "RockLayer.h"
#include "SubLayer.h"
#include "Location.h"
#include "NonLinearPropertyVariation.h"
#include "ProfileVariation.h"
#include "Units.h"
#include "TextLog.h"

#include <gsl/gsl_rng.h>

#include <QVector>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QDataStream>
#include <QObject>
#include <QStringList>

class SiteProfile : public QObject
{
    Q_OBJECT 

    public:
        SiteProfile(QObject * parent = 0);
        ~SiteProfile();
        
        QList<SoilType*> & soilTypes();
        QList<SoilLayer*> & soilLayers();
        QList<SubLayer> & subLayers();
        RockLayer * bedrock();
        
        //! Reset the object to the default values
        void reset();

		bool isSiteVaried() const;
		void setIsSiteVaried(bool isSiteVaried);
        
        double inputDepth() const;
        void setInputDepth(double depth);

        const Location & inputLocation() const;

        /*! Compute the layer index associated with a depth.
         * \param depth depth in the site profile
         * \return Location corresponding to the depth.
         */
        const Location depthToLocation(const double depth) const;

        QStringList soilTypeNameList() const;
        QStringList soilLayerNameList() const;

        int profileCount() const;
		void setProfileCount(int count);
        
        ProfileVariation & profileVariation();
        NonLinearPropertyVariation & nonLinearPropertyVariation();
        
        double maxFreq() const;
        void setMaxFreq(double maxFreq);

        double waveFraction() const;
        void setWaveFraction(double waveFraction);

		//! Create the sublayers for a given realization
		void createSubLayers(TextLog & textLog);
        
        //! Reset the properties of the SubLayers to their initial properties
        void resetSubLayers();

        int subLayerCount() const;

        /*! @name Convience accessors
         * The following accessors allow for the SubLayers and Bedrock to be
         * accessed using the same functions.
         */
        //@{
        double untWt( int layer ) const;
        double density( int layer ) const;
        double shearVel( int layer ) const;
        double shearMod( int layer) const;
        double damping( int layer ) const;
        //@}
        
        //! Return the shear-wave velocity of the sublayers and bedrock
        QVector<double> depthProfile() const;
        QVector<double> depthToMidProfile() const;
        QVector<double> shearVelProfile() const;
        QVector<double> shearModProfile() const;
        QVector<double> dampingProfile() const;
        QVector<double> vTotalStressProfile() const;
        QVector<double> maxShearStrainProfile() const;
        QVector<double> shearStressProfile() const;
        QVector<double> stressRatioProfile() const;
        QVector<double> maxErrorProfile() const;

        //! Units system
        const Units * units() const;
        void setUnits( const Units * units);

        //! Set the thickness of the layer and update the depths below it
        void setThicknessAt(int layer, double depth);

        QMap<QString, QVariant> toMap() const;
        void fromMap( const QMap<QString, QVariant> & map );

        //! Create a table of the sublayers
        QString subLayerTable() const;

        //! Create a html document containing the information of the model
        QString toHtml() const;

    public slots:
        //! Refresh depths of the layers
        void updateDepths();

    signals:
        void soilTypeChanged();
        void soilLayersChanged();
        void depthsChanged();

    private:
        //! Units system 
        const Units * m_units;

        //! Return the layer with the longest travel time between the two depths
        SoilLayer * representativeSoilLayer( double top, double base);

        /*
         * A site profile consists of a hierarchy of three different layers
         * SoilTypes -> VelocityLayers -> SubLayers
         * The SubLayers are what matters for the site response calculation
         * phase.  The ordering of soil layers and velocity layers is used to
         * facilitate the varying of the soil properties.
         */
        QList<SoilType*> m_soilTypes;
        QList<SoilLayer*> m_soilLayers;
        QList<SubLayer> m_subLayers;

        //! Bedrock layer in the model
        RockLayer * m_bedrock; 

        /*! @name Input motion specification
         */
        //{@
        //! Location where the motion is applied
        /*! The motions are applied at a certain depth and propagated to the
         * surface.  If the depth is less than zero then the depth of the
         * bedrock is used.  The user defines the depth of the input location.
         */
        double m_inputDepth;

        //! Index associated with the location
        Location m_inputLocation;
        //@}

        /*! @name Variation parameter
         */
        //{@
        //! Variation of the velocity profile
        ProfileVariation m_profileVariation;
        
        //! Variation of the soil properites
        NonLinearPropertyVariation m_nonLinearPropertyVariation;

        //! If the site is varied
        bool m_isSiteVaried;

        //! Number of artificial profiles to generate
		int m_profileCount;

        //! Random number generator
        gsl_rng * m_rng;
        //@}

        /*! @name Layer discretization parameters
         */
        //@{
        //! Maximum frequency of interest
        double m_maxFreq;

        //! Wavelength fraction
        double m_waveFraction;
        //@}
};
#endif
