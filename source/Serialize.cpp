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

#include <QtCore/QJsonValue>
#include "Serialize.h"

namespace Serialize {
    QJsonArray toJsonArray(const QVector<double> &vector) {
        QJsonArray jsonArray;
        for (const double &v : vector)
            jsonArray << QJsonValue(v);
        return jsonArray;
    }

    void toDoubleVector(const QJsonValue &object, QVector<double> &vector) {
        vector.clear();
        for (const QJsonValue &qjv : object.toArray())
            vector << qjv.toDouble();
    }

    QList<QVariant> toVariantList(const QList<double> &list) {
        QList<QVariant> newList;

        for (const double &d : list)
            newList << QVariant(d);

        return newList;
    }

    QList<QVariant> toVariantList(const QVector<double> &vector) {
        QList<QVariant> newList;

        for (int i = 0; i < vector.size(); ++i)
            newList << QVariant(vector.at(i));

        return newList;
    }

    QList<double> fromVariantList(const QList<QVariant> &list) {
        QList<double> newList;

        for (int i = 0; i < list.size(); ++i)
            newList << list.at(i).toDouble();

        return newList;
    }
}