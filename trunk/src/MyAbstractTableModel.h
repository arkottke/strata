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

#ifndef MY_ABSTRACT_TABLE_MODEL_H_
#define MY_ABSTRACT_TABLE_MODEL_H_

#include <QAbstractTableModel>

class MyAbstractTableModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        MyAbstractTableModel(bool readOnly = false, QObject * parent = 0);

    public slots:
        void resetModel();
        void setReadOnly(bool b);

    protected:
        //! If the table is read-only
        bool m_readOnly;
};
#endif
