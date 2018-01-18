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
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "Units.h"

#include <QChar>
#include <QDataStream>
#include <QDebug>

Units * Units::_instance = 0;

Units::Units( QObject * parent )
    : QObject(parent)
{
    _system = Metric;
    reset();
}

Units * Units::instance()
{
    if ( _instance == 0 ) {
        _instance = new Units;
    }

    return _instance;
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
    return _system;
}

void Units::setSystem(System system)
{
    if (_system != system) {
        _system = system;

        emit systemChanged((int)_system);
    }
}

void Units::setSystem(int system)
{
    setSystem((System)system);
}

double Units::gravity() const
{
    switch (_system) {
    case Metric:
        return 9.80665;
    case English:
        return 32.174;
    default:
        return -1;
    }
}

double Units::waterUntWt() const
{
    switch (_system) {
    case Metric:
        return gravity();
    case English:
        return 62.4;
    default:
        return -1;
    }
}

double Units::toAtm() const
{
    switch (_system) {
    case Metric:
        return 9.868233e-3;
    case English:
        return 4.725451e-4;
    default:
        return -1;
    }
}

double Units::toMeters() const
{
    switch (_system){
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

    if ( _system == Metric )
        // Convert gravity from meters into centimeters
        conv = gravity() * 100;
    else if ( _system == English )
        // Convert gravity from feet into inches
        conv = gravity() * 12;

    return conv;
}

QString Units::length() const
{
    if ( _system == Metric )
        return "m";
    else if ( _system == English )
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

    if ( _system == Metric )
        label = "cm/s";
    else if ( _system == English )
        label = "in/s";

    return label;
}

QString Units::dispTs() const
{
    QString label = "";

    if ( _system == Metric )
        label = "cm";
    else if ( _system == English )
        label = "in";

    return label;
}

QString Units::vel() const
{
    return QString("%1/s").arg(length());
}

QString Units::wt() const
{
    if ( _system == Metric ) {
        return "kN";
    } else if ( _system == English ) {
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

    out << (int)units->_system;

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
