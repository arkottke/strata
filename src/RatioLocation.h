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

#ifndef RATIO_LOCATION_H_
#define RATIO_LOCATION_H_

#include "Output.h"
#include "EquivLinearCalc.h"
#include "Motion.h"

#include <QVector>

//! Stores data for when a response ratio is computed between two locations. 

class RatioLocation
{
    public:
        RatioLocation();

        double toDepth() const;
        void setToDepth(double toDepth);
        
        double fromDepth() const;
        void setFromDepth(double fromDepth);
    
        Motion::Type toType() const;
        void setToType(Motion::Type toType);
        
        Motion::Type fromType() const;
        void setFromType(Motion::Type fromType);
        
        Output * transFunc();
        Output * strainTransFunc();
        Output * respRatio();

        //! Clear the data saved in the output
        void clear();
        
        //! Save the results
        void saveResults( const EquivLinearCalc * calc, const QVector<double> & freq, const QVector<double> & period, double damping );
        
        //! Remove the last result
        void removeLast();
        
        QMap<QString, QVariant> toMap() const;
		void fromMap(const QMap<QString, QVariant> & map);

    protected:
        //! Rename the outputs
        void renameOutputs();

    private:
        //! Location to which the response went to
        double m_toDepth;

        //! Location from which the response came from
        double m_fromDepth;
        
        //! The type of surface to with the reponse went to
        Motion::Type m_toType;

        //! The type of surface from which the reponse came from
        Motion::Type m_fromType;

        //! Acceleration transfer function
        Output * m_transFunc;

        //! Strain transfer function
        Output * m_strainTransFunc;

        //! Acceleration response ratio
        Output * m_respRatio;
};
#endif
