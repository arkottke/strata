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

#ifndef DIMENSION_H_
#define DIMENSION_H_

#include <QVector>
#include <QStringList>
#include <QMap>
#include <QString>
#include <QVariant>

class Dimension
{
    public:
        Dimension();

        enum Spacing{
            Linear, //!< Equally spaced in linear space
            Log, //!< Equally spaced in log space
        };

        static QStringList spacingList();

        double min() const;
        void setMin(double min);

        double max() const;
        void setMax(double max);

        int npts() const; void setNpts(int npts);

        Spacing spacing() const;
        void setSpacing(Spacing spacing);

        QVector<double> & data();

        /*! Compute the data -- must be called before data can be used.
         * \param hasMin has a minimum value
         * \param minValue minimum possible value
         * \param hasMax has a maximum value
         * \param maxValue maximum possible value
         */
        void init( bool hasMin = false, double minValue = 0, 
                bool hasMax = false, double maxValue = 0);
	
        //! Save the object to a map
        /*!
         * \param saveData saves the m_data vector.
         * \return a map of the object
         */
        QMap<QString, QVariant> toMap(bool saveData = false) const;
		void fromMap(const QMap<QString, QVariant> & map);

        static QVector<double> linSpace( double min, double max, int size );
        static QVector<double> logSpace( double min, double max, int size );

    private:
        //! Minimum value
        double m_min;
        
        //! Maximum value
        double m_max;

        //! Number of points
        int m_npts;

        //! Spacing
        Spacing m_spacing;

        //! Data points
        QVector<double> m_data;
};
#endif
