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

#ifndef SERIALIZER_H_
#define SERIALIZER_H_

#include <QList>
#include <QVector>
#include <QVariant>

class Serializer
{
    public:
        Serializer();
       
        static QList<QVariant> toVariantList(const QList<double> & list); 
        static QList<QVariant> toVariantList(const QVector<double> & vector); 

        static QList<double> fromVariantList(const QList<QVariant> & list);
};
#endif
