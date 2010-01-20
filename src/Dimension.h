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
#include <QObject>
#include <QString>
#include <QVariant>

//! Class to create vectors of evening spaced values in either log or linear space.

class Dimension : public QObject
{
    Q_OBJECT

    public:
        Dimension(QObject * parent = 0);

        enum Spacing{
            Linear, //!< Equally spaced in linear space
            Log, //!< Equally spaced in log space
        };

        static QStringList spacingList();

        double min() const;

        double max() const;

        int size() const;

        double at(int i) const;

        void setSpacing(Spacing spacing);
        Spacing spacing() const;

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

    public slots:
        void setMin(double min);
        void setMax(double max);
        void setSize(int npts);
        void setSpacing(int spacing);


    signals:
        void wasModified();

    private:
        //! Minimum value
        double m_min;
        
        //! Maximum value
        double m_max;

        //! Number of points
        int m_size;

        //! Spacing
        Spacing m_spacing;

        //! Data points
        QVector<double> m_data;
};
#endif
