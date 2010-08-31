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

#include "Location.h"

Location::Location( int layer, double depth)
    : m_layer(layer), m_depth(depth)
{
}

int Location::layer() const
{
    return m_layer;
}

void Location::setLayer(const int layer)
{
    m_layer = layer;
}

double Location::depth() const
{
    return m_depth;
}

void Location::setDepth(const double depth)
{
    m_depth = depth;
}
