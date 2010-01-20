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

#ifndef UNITS_H_
#define UNITS_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>

class Units : public QObject
{
    Q_OBJECT

    public:
        static Units * instance();

        //! System of the units
		enum System { 
            Metric, //!< Metric system ( m and kN/m^3 )
            English //!< English system ( ft and pcf )
        };
        
        static QStringList systemList();

        //! Reset the object to the default values
        void reset();

        System system() const;
        void setSystem(System system); 

        //! The gravity in ft/s/s or m/s/s
        double gravity() const;

        //! The unit weight of water in lb/ft^3 or kN/m^3
        double waterUntWt() const;

        //! Convert to Meters
        double toMeters() const;
        
        //! Convert from appropriate pressure to atm
        double toAtm() const;

        //! Time series conversion factor
        double tsConv() const;

        //! Length label
        QString length() const;

        //! Area label
        QString area() const;

        //! Volume label
        QString vol() const;
        
        //! Displacement time series label -- in or cm
        QString dispTs() const;

        //! Velocity time series label -- in/s or cm/s
        QString velTs() const;

        //! Acceleration label
        QString accel() const;

        //! Velocity label 
        QString vel() const;

        //! Weight label
        QString wt() const;

        //! Unit weight label
        QString untWt() const;
		
        //! Stress label
        QString stress() const;

        QMap<QString, QVariant> toMap() const;
		void fromMap(const QMap<QString, QVariant> & map);

    public slots:
        void setSystem(int system); 

    signals:
        void systemChanged();

    protected:
        Units(QObject * parent = 0);

        static Units * m_instance;
        
        System m_system;
};
#endif
