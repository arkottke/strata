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

#ifndef INDEXED_DEPTH_H_
#define INDEXED_DEPTH_H_

class IndexedDepth
{
    public:
        IndexedDepth( int siteIndex, int dataIndex, double depth );

        int dataIndex() const;
        int siteIndex() const;
        double depth() const;

        bool operator<( const IndexedDepth & other ) const;

    private:
        //! Site index
        int m_siteIndex;

        //! Index of the data
        int m_dataIndex;

        //! Depth value
        double m_depth;
};
#endif
