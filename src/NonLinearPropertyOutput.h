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

#ifndef NON_LINEAR_PROPERTY_OUTPUT_H_
#define NON_LINEAR_PROPERTY_OUTPUT_H_

#include "Output.h"
#include "SiteResponseOutput.h"

class NonLinearPropertyOutput
{
    public:
        NonLinearPropertyOutput();

        //! Clear the data
        void clear();

        //! Define the location data -- clears the data vectors
        void setStrain( const QVector<double> & strain );
        
        QMap<QString, QVariant> toMap(bool saveData = false) const;
		void fromMap(const QMap<QString, QVariant> & map);

    protected:
        const QVector<QVector<double> > & strain() const;

    private:
        bool m_enabled;

        //! Holds information about the strain of the data points
        QVector<double> m_strain;

        //! Holds information about the property
        QVector<QVector<double> > m_prop;
};
#endif
