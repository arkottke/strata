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

#include "Units.h"
#include <QChar>
#include <QDebug>

Units * Units::m_instance = 0;

Units::Units( QObject * parent )
    : QObject(parent)
{
    reset();
}

Units * Units::instance()
{
    if ( m_instance == 0 ) {
        m_instance = new Units;
    }

    return m_instance;
}

QStringList Units::systemList()
{
    return QStringList() << tr("Metric") << tr("English");
}

void Units::reset()
{
    setSystem(Metric);
}

Units::System Units::system() const
{
    return m_system;
}

void Units::setSystem(System system)
{
    if (m_system != system) {
        m_system = system;

        emit systemChanged((int)m_system);
    }
}

void Units::setSystem(int system)
{
    setSystem((System)system);
}

double Units::gravity() const
{
    if ( m_system == Metric )
        return 9.80665;
    else if ( m_system == English )
        return 32.174;
        
    return -1;
}

double Units::waterUntWt() const
{
    if ( m_system == Metric )
        return 9.81;
    else if ( m_system == English )
        return 62.4;
        
    return -1;
}

double Units::toAtm() const
{
    if ( m_system == Metric )
        return 9.868233e-3;
    else if ( m_system == English )
        return 4.725451e-4;
        
    return -1;
}

double Units::toMeters() const
{
    switch ( m_system )
    {
        case Metric:
            return 1.0;
        case English:
            return 0.3048;
        default:
            return -1;
    }
}

double Units::tsConv() const
{
    double conv = 0;

    if ( m_system == Metric )
        // Convert gravity from meters into centimeters
        conv = gravity() * 100;
    else if ( m_system == English )
        // Convert gravity from feet into inches
        conv = gravity() * 12;

    return conv;
}

QString Units::length() const
{
    if ( m_system == Metric )
        return "m";
    else if ( m_system == English )
        return "ft";

    return "";
}

QString Units::area() const
{
    return QString("%1%2").arg(length()).arg(QChar(0x00B2));
}

QString Units::vol() const
{
    return QString("%1%2").arg(length()).arg(QChar(0x00B3));
}

QString Units::accel() const
{
    return "g";
}

QString Units::velTs() const
{
    QString label = "";

    if ( m_system == Metric )
        label = "cm/s";
    else if ( m_system == English )
        label = "in/s";

    return label;
}

QString Units::dispTs() const
{
    QString label = "";

    if ( m_system == Metric )
        label = "cm";
    else if ( m_system == English )
        label = "in";

    return label;
}

QString Units::vel() const
{
    return QString("%1/s").arg(length());
}

QString Units::wt() const
{
    if ( m_system == Metric ) {
        return "kN";
    } else if ( m_system == English ) {
        return "lb";
    }

    return "";
}

QString Units::untWt() const
{
    return QString("%1/%2").arg(wt()).arg(vol());
}

QString Units::stress() const
{
    return QString("%1/%2").arg(wt()).arg(area());
}

QDataStream & operator<< (QDataStream & out, const Units* units)
{
    out << (quint8)1;

    out << (int)units->m_system;

    return out;
}

QDataStream & operator>> (QDataStream & in, Units* units)
{
    quint8 ver;
    in >> ver;

    int system;
    in >> system;

    units->setSystem(system);

    return in;
}
