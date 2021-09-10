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

#ifndef SERIALIZE_H_
#define SERIALIZE_H_

#include <QList>
#include <QVector>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace Serialize {
    auto toJsonArray(const QVector<double> &vector) -> QJsonArray;

    void toDoubleVector(const QJsonValue &object, QVector<double> &vector);

    auto toVariantList(const QList<double> &list) -> QList<QVariant>;

    auto toVariantList(const QVector<double> &vector) -> QList<QVariant>;

    auto fromVariantList(const QList<QVariant> &list) -> QList<double>;
};
#endif
