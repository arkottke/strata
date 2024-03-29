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

#ifndef LOCATION_H_
#define LOCATION_H_

class Location {
public:
  explicit Location(int layer = 0, double depth = 0);

  auto layer() const -> int;

  void setLayer(int layer);

  auto depth() const -> double;

  void setDepth(double depth);

private:
  //! Sublayer index of the location
  int _layer;

  //! Depth from the top of the layer
  double _depth;
};
#endif // LOCATION_H_
