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

#ifndef MINIMUM_FINDER_H_
#define MINIMUM_FINDER_H_

#include <QList>

//! Simple class that finds the local minimum or maximum value.

class MinimumFinder 
{
    public:
        MinimumFinder( double center, double delta );

        double center() const;
        double minimum() const;
        const QList<double> & trials() const;

        /*! Set the conditions of where to compute the minimum.
         * \param err error values corresponding to the trial values
         */
        void setResults( const QList<double> & err );

        //! Check if the delta is still reasonable
        bool isDeltaReasonable() const;
        
        //! Update the bounds of the minimum finder
        void moveBounds();

    private:
        //! Center estimate
        double m_center;

        //! Amount to vary the center estimate
        double m_delta;

        //! Trial x values
        QList<double> m_trials;

        //! Minimum based on quadratic interpolation
        double m_minimum;

        //! Location of the minimum value
        double m_minLocation;

        //! If the minimum is within the bounds
        bool m_withinBounds;

};
#endif
