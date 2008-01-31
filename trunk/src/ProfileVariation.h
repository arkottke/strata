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

#ifndef PROFILE_VARIATION_H_
#define PROFILE_VARIATION_H_

#include "SoilLayer.h"
#include "RockLayer.h"
#include "Distribution.h"
#include "Units.h"

#include <gsl/gsl_rng.h>

#include <QStringList>
#include <QString>
#include <QMap>
#include <QVariant>

//! Variation of the layer thickness and shear-wave velocity
/*!
 * The variation of the layer thickness and the shear-wave velocity of these
 * layers is varied using the Toro (1995) model.
 */

class ProfileVariation
{
    public:
        ProfileVariation();

        enum VelocityModel{ Custom, GeoMatrix_AB, GeoMatrix_CD, USGS_AB, USGS_CD, USGS_A, USGS_B, USGS_C, USGS_D };

        enum LayeringModel{ 
            CustomLayering, //!< Custom values
            DefaultLayering //!< Default value
        };

        static QStringList velocityModelList();
        static QStringList layeringModelList();
        
        //! Reset the object to the default values
        void reset();
        
        //! Units system
        void setUnits( const Units * units);

        bool enabled() const;
        void setEnabled(bool enabled);

        bool isVelocityVaried() const;
        void setVaryVelocity(bool isVelocityVaried);

        bool isLayeringVaried() const;
        void setVaryLayering(bool isLayeringVaried);
        
        bool isBedrockDepthVaried() const;
        void setVaryBedrockDepth(bool bedrockDepth);

        VelocityModel stdevModel() const;
        void setStdevModel(VelocityModel model);

        bool stdevIsLayerSpecific() const;
        void setStdevIsLayerSpecific(bool stdevIsLayerSpecific);

        double stdev() const;
        void setStdev(double stdev);

        VelocityModel correlModel() const;
        void setCorrelModel(VelocityModel model);

        double correlInitial() const;
        void setCorrelInitial(double correlInitial);

        double correlFinal() const;
        void setCorrelFinal(double correlFinal);

        double correlDelta() const;
        void setCorrelDelta(double correlDelta);

        double correlIntercept() const;
        void setCorrelIntercept(double correlIntercept);

        double correlExponent() const;
        void setCorrelExponent(double correlExponent);

        LayeringModel layeringModel() const;
        void setLayeringModel(LayeringModel model);

        double layeringCoeff() const;
        void setLayeringCoeff(double layeringCoeff);

        double layeringInitial() const;
        void setLayeringInitial(double layeringInitial);
        
        double layeringExponent() const;
        void setLayeringExponent(double layeringExponent);

        Distribution & bedrockDepth();

        void setRandomNumberGenerator(gsl_rng * rng);

        //! Vary the thicknesses of the layers
        QList<double> varyLayering(double depthToBedrock) const;
        
        //! Vary the shear-wave velocity of the layers
        void varyVelocity( QList<SoilLayer*> & soilLayers, RockLayer * bedrock ) const;
        
		QMap<QString, QVariant> toMap() const;
		void fromMap( const QMap<QString, QVariant> & map );

    private:
        //! Enable variation of the profile
        bool m_enabled;
        
        //! Units system 
        const Units * m_units;

        /*
         * Switches for controlling what is varied
         */
        //! Vary the shear-wave velocity of the layers
        bool m_isVelocityVaried;
        
        //! Vary the thickness of the layers
        bool m_isLayeringVaried;

        //! Vary the depth to the bedrock
        bool m_isBedrockDepthVaried;

        /*! @name Standard deviation parameters
         *
         */
        //@{
        
        //! Model for the standard deviation
        VelocityModel m_stdevModel;

        //! Allows the standard deviation to be defined at the layer
        bool m_stdevIsLayerSpecific;

        //! Standard deviation of the entire site
        double m_stdev;
        //@}

        /*! @name Correlation parameters
         *
         */
        //@{
        //! Model for the correlation 
        VelocityModel m_correlModel;

        //! Initial correlation (depth of 0 m)
        double m_correlInitial;

        //! Final correlation  (depth of 200 m)
        double m_correlFinal;

        //! Change in correlation with depth
        double m_correlDelta;

        //! Depth intercept
        double m_correlIntercept;
            
        //! Exponent of the correlation model
        double m_correlExponent;
        //@}

        /*! @name Layering parameters
         *  The layering of the profile is computed from a Poisson process with
         *  depth dependent activity rate, defined as:
         *  \f[ \lambda(d) = a ( d + b ) ^ c \f]
         *  where \f$a\f$, \f$b\f$, and \f$c\f$ are referred to as the layering
         *  coefficient, initial value, and exponent, respectively.
         */
        //@{
        
        //! Source of the variables
        LayeringModel m_layeringModel;

        double m_layeringCoeff;
        double m_layeringInitial;
        double m_layeringExponent;
        //@}

        //! Variation of the bedrock layer
        Distribution m_bedrockDepth;
       
        //! Random number generator
        gsl_rng * m_rng;
};
#endif
