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

#ifndef RESPONSE_LOCATION_H_
#define RESPONSE_LOCATION_H_

#include "Dimension.h"
#include "Output.h"
#include "Motion.h"
#include "EquivLinearCalc.h"

class ResponseLocation
{
    public:
        ResponseLocation();
        
        double depth() const;
        void setDepth(double depth);
    
        Motion::Type type() const;
        void setType(Motion::Type type);

        bool isBaselineCorrected() const;
        void setBaselineCorrected(bool baselineCorrected);

        Output * fourierSpec();
        Output * respSpec();
        Output * accelTs();
        Output * velTs();
        Output * dispTs();
        Output * strainTs();
        Output * stressTs();

        //! Check if any of the time series are enabled
        bool needsTime() const;

        //! Check if the raw frequency is required
        bool needsRawFreq() const;

        //! Check if the response spectrum is enabled
        bool needsPeriod() const;

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
        //! Depth of the response
        double  m_depth;

        //! Type of the repsonse
        Motion::Type m_type;

        //! If a baseline correction should be applied to the time series
        bool m_isBaselineCorrected;

        /*! @name Responses
         */
        //@{
        //! Fourier spectrum of the acceleration
        Output * m_fourierSpec;

        //! Acceleration response spectrum
        Output * m_respSpec;
        
        //! Acceleration time series
        Output * m_accelTs;
        
        //! Velocity time series
        Output * m_velTs;
        
        //! Displacement time series
        Output * m_dispTs;
        
        //! Strain time series
        Output * m_strainTs;
        
        //! Stress time series
        Output * m_stressTs;
        //@}
};
#endif
