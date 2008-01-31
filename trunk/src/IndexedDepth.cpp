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

#include "IndexedDepth.h"

IndexedDepth::IndexedDepth( int siteIndex, int dataIndex, double depth )
     : m_siteIndex(siteIndex), m_dataIndex(dataIndex), m_depth(depth)
{
}

int IndexedDepth::dataIndex() const
{
    return m_dataIndex;
}

double IndexedDepth::depth() const
{
    return m_depth;
}

int IndexedDepth::siteIndex() const
{
    return m_siteIndex;
}

bool IndexedDepth::operator<( const IndexedDepth & other ) const
{
    return m_depth < other.depth();
}
