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

Units *Units::_instance = nullptr;

Units::Units(QObject *parent) : QObject(parent) {
  _system = Metric;
  reset();
}

auto Units::instance() -> Units * {
  if (_instance == nullptr) {
    _instance = new Units;
  }

  return _instance;
}

auto Units::systemList() -> QStringList {
  return QStringList() << tr("Metric") << tr("English");
}

void Units::reset() { setSystem(Metric); }

auto Units::system() const -> Units::System { return _system; }

void Units::setSystem(System system) {
  if (_system != system) {
    _system = system;

    emit systemChanged((int)_system);
  }
}

void Units::setSystem(int system) { setSystem((System)system); }

auto Units::gravity() const -> double {
  switch (_system) {
  case Metric:
    return 9.80665;
  case English:
    return 32.174;
  default:
    return -1;
  }
}

auto Units::waterUntWt() const -> double {
  switch (_system) {
  case Metric:
    return gravity();
  case English:
    return 62.4;
  default:
    return -1;
  }
}

auto Units::toAtm() const -> double {
  switch (_system) {
  case Metric:
    return 9.868233e-3;
  case English:
    return 4.725451e-4;
  default:
    return -1;
  }
}

auto Units::toMeters() const -> double {
  switch (_system) {
  case Metric:
    return 1.0;
  case English:
    return 0.3048;
  default:
    return -1;
  }
}

auto Units::tsConv() const -> double {
  double conv = 0;

  if (_system == Metric)
    // Convert gravity from meters into centimeters
    conv = gravity() * 100;
  else if (_system == English)
    // Convert gravity from feet into inches
    conv = gravity() * 12;

  return conv;
}

auto Units::length() const -> QString {
  if (_system == Metric)
    return "m";
  else if (_system == English)
    return "ft";

  return "";
}

auto Units::area() const -> QString {
  return QString("%1%2").arg(length()).arg(QChar(0x00B2));
}

auto Units::vol() const -> QString {
  return QString("%1%2").arg(length()).arg(QChar(0x00B3));
}

auto Units::accel() const -> QString { return "g"; }

auto Units::velTs() const -> QString {
  QString label = "";

  if (_system == Metric)
    label = "cm/s";
  else if (_system == English)
    label = "in/s";

  return label;
}

auto Units::dispTs() const -> QString {
  QString label = "";

  if (_system == Metric)
    label = "cm";
  else if (_system == English)
    label = "in";

  return label;
}

auto Units::vel() const -> QString { return QString("%1/s").arg(length()); }

auto Units::wt() const -> QString {
  if (_system == Metric) {
    return "kN";
  } else if (_system == English) {
    return "lb";
  }

  return "";
}

auto Units::untWt() const -> QString {
  return QString("%1/%2").arg(wt()).arg(vol());
}

auto Units::stress() const -> QString {
  return QString("%1/%2").arg(wt()).arg(area());
}

auto operator<<(QDataStream &out, const Units *units) -> QDataStream & {
  out << (quint8)1;

  out << (int)units->_system;

  return out;
}

auto operator>>(QDataStream &in, Units *units) -> QDataStream & {
  quint8 ver;
  in >> ver;

  int system;
  in >> system;

  units->setSystem(system);

  return in;
}
