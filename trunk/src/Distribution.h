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

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include <gsl/gsl_rng.h>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>


class Distribution
{
    public:
        Distribution();

        enum Type {
            Uniform, //!< Uniform distribution
            Normal, //!< Normal distribution
            LogNormal //!< Log-Normal distribution
        };

        static QStringList typeList();
        
        Type type() const;
        void setType(Type type);

        double avg() const;
        void setAvg(double avg);

        double stdev() const;
        void setStdev(double stdev);

        bool hasMin() const;
        void setHasMin(bool hasMin);

        double min() const;
        void setMin(double min);

        bool hasMax() const;
        void setHasMax(bool hasMax);

        double max() const;
        void setMax(double max);

        //! Set the random number generator
        void setRandomNumberGenerator(gsl_rng * rng);

        //! Return a random variable from the distribution
        double rand();
		
        QMap<QString, QVariant> toMap() const;
		void fromMap( const QMap<QString, QVariant> & map );

    private:
        //! Type of distribution
        Type m_type;

        //! Average of the distribution
        /*!
         * For uniform distributions this is not used.  For normal
         * distributions this is mean of the distribution.  For log-normal
         * distributions this is the median of the distribution in linear space
         * (or the mean in log space).
         */
        double m_avg;

        //! Standard deviation of the distribution
        /*!
         * For uniform distributions this is not used.  For normal
         * distributions this is the standard deviation in linear space, but
         * for log-normal distributions this is the standard deviation in log
         * space.
         */
        double m_stdev;

        //! If the distribution has a minimum
        bool m_hasMin;

        //! The minimum value of the distribution
        double m_min;
      
        //! If the distribution has a maximum
        bool m_hasMax;

        //! The maximum value of the distribution
        double m_max;

        //! The random number generator of the distribution
        gsl_rng * m_rng;
};
#endif
