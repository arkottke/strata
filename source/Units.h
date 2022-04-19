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

#ifndef UNITS_H_
#define UNITS_H_

#include <QObject>
#include <QString>
#include <QStringList>

class Units : public QObject {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const Units *units) -> QDataStream &;
  friend auto operator>>(QDataStream &in, Units *units) -> QDataStream &;

public:
  static auto instance() -> Units *;

  //! System of the units
  enum System {
    Metric, //!< Metric system ( m and kN/m^3 )
    English //!< English system ( ft and pcf )
  };

  static auto systemList() -> QStringList;

  //! Reset the object to the default values
  void reset();

  auto system() const -> System;
  void setSystem(System system);

  //! The gravity in ft/s/s or m/s/s
  auto gravity() const -> double;

  //! The unit weight of water in lb/ft^3 or kN/m^3
  auto waterUntWt() const -> double;

  //! Convert to Meters
  auto toMeters() const -> double;

  //! Convert from appropriate pressure to atm
  auto toAtm() const -> double;

  //! Time series conversion factor
  auto tsConv() const -> double;

  //! Length label
  auto length() const -> QString;

  //! Area label
  auto area() const -> QString;

  //! Volume label
  auto vol() const -> QString;

  //! Displacement time series label -- in or cm
  auto dispTs() const -> QString;

  //! Velocity time series label -- in/s or cm/s
  auto velTs() const -> QString;

  //! Acceleration label
  auto accel() const -> QString;

  //! Velocity label
  auto vel() const -> QString;

  //! Weight label
  auto wt() const -> QString;

  //! Unit weight label
  auto untWt() const -> QString;

  //! Stress label
  auto stress() const -> QString;

public slots:
  void setSystem(int system);

signals:
  void systemChanged(int system);

protected:
  explicit Units(QObject *parent = nullptr);

  static Units *_instance;

  System _system;
};
#endif
