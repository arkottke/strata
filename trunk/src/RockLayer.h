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

#ifndef ROCK_LAYER_H_
#define ROCK_LAYER_H_

#include "VelocityLayer.h"

#include <QMap>
#include <QString>
#include <QVariant>

//! Describes the velocity variation and linear response of bedrock

class RockLayer : public VelocityLayer
{
    Q_OBJECT

	public:
		RockLayer(QObject * parent = 0);
        ~RockLayer();
        
        //! Reset the object to the default values
        void reset();

        double untWt() const;
        double density() const;
        
        //! Randomized damping and strain dependent damping
        double damping() const;

        //! Set the varied damping and strain dependent damping
        void setDamping( double damping );

        //! Set the average damping only
        double avgDamping() const;
        
        //! A description of the layer for tables
        QString toString() const;

		QMap<QString, QVariant> toMap() const;
		void fromMap( const QMap<QString, QVariant> & map );

    public slots:
        void setUntWt(double untWt);
        //! Set both the average and varied damping
        void setAvgDamping( double damping );

    signals:
        void wasModified();

    private:
        //! Unit weight of the rock
        double m_untWt;

        //! Damping of the rock -- randomized
        double m_damping;
        
        //! Damping of the rock -- average
        double m_avgDamping;
};
#endif
