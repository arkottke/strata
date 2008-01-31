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

#ifndef VELOCITY_LAYER_H_
#define VELOCITY_LAYER_H_

#include <QStringList>

//! A virtual class that describes the shear-wave velocity of a specific layer

class VelocityLayer
{
	public:
		VelocityLayer();
        VelocityLayer( const VelocityLayer & other );
        virtual ~VelocityLayer() = 0;
    
        //! Type of statistical distribution
		enum Distribution{ 
            Normal, //!< Normal distribution
            LogNormal, //!< Log-normal distribution
            Uniform //!< Uniform distribution
        };
        
        static QStringList distributionList();

        double depth() const;
        void setDepth(double depth);

        virtual double untWt() const = 0;
        virtual double density() const = 0;

        double shearVel() const;
        //! Set the randomized shear-wave velocity
        void setShearVel(double shearVel);
        
        double shearMod() const;

		double avg() const;
		void setAvg(double avg);

		VelocityLayer::Distribution distribution() const;
		void setDistribution(VelocityLayer::Distribution dist);

		double stdev() const;
		void setStdev(double stdev);
        
        bool hasMax() const;
        void setHasMax(bool hasMax);

		double max() const;
		void setMax(double max);

        bool hasMin() const;
        void setHasMin(bool hasMin);

		double min() const;
		void setMin(double min);

		double isVaried() const;
		void setIsVaried(bool isVaried);
        
        //! A description of the layer for tables
        virtual QString toString() const = 0;

		/*
		 * Before the layer can be used it must be initialized if the layer is not set
		 * to be varied then this simply copies the average value over.
		 */
		void vary( double randVar );

		virtual QMap<QString, QVariant> toMap() const = 0;
		virtual void fromMap( const QMap<QString, QVariant> & map ) = 0;

	protected:
        //! If the shear-wave velocity is varied with randomization
		bool m_isVaried;

        //! Shear-wave velocity of the layer -- varies with randomization
        double m_shearVel;

        //! Type of distribution
        /*! The velocity can be varied based on a Normal, LogNormal, or Uniform distribution
         */
		Distribution m_distribution;

        //! The average velocity
        /*! For a normal distribution the average value is representative of
         * the mean, but for the log normal value the average represents the
         * median.
         */
		double m_avg;
        
        //! The standard deviation
        /*! The standard deviation of the distribution.  Not used if the
         * distribution is defined as uniform.
         */
		double m_stdev;
        
        //! The maximum value
		double m_max;
        bool m_hasMax;

        //! The minimum value
		double m_min;
        bool m_hasMin;

        //! Depth to the top of the layer
        double m_depth;
};
#endif 
