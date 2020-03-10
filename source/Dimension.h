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

#ifndef DIMENSION_H_
#define DIMENSION_H_

#include <QDataStream>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

//! Class to create vectors of evening spaced values in either log or linear space.

class Dimension : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const Dimension* d) -> QDataStream &;
    friend auto operator>> (QDataStream & in, Dimension* d) -> QDataStream &;

public:
    explicit Dimension(QObject *parent = nullptr);

    enum Spacing{
        Linear, //!< Equally spaced in linear space
        Log, //!< Equally spaced in log space
    };

    static auto spacingList() -> QStringList;

    auto min() const -> double;
    auto max() const -> double;
    auto size() const -> int;
    auto at(int i) const -> double;

    void setSpacing(Spacing spacing);
    auto spacing() const -> Spacing;

    auto data() -> QVector<double> &;

    //! If the dimension is populated with values
    auto ready() const -> bool;

    //! Calculate the values in the vector
    void init();

    static auto linSpace( double min, double max, int size ) -> QVector<double>;
    static auto logSpace( double min, double max, int size ) -> QVector<double>;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setMin(double min);
    void setMax(double max);
    void setSize(int npts);
    void setSpacing(int spacing);

    void clear();

signals:
    void minChanged(double min);
    void maxChanged(double max);
    void sizeChanged(int size);
    void spacingChanged(int spacing);
    void wasModified();

private:
    //! Minimum value
    double _min;

    //! Maximum value
    double _max;

    //! Number of points
    int _size;

    //! Spacing
    Spacing _spacing;

    //! Data points
    QVector<double> _data;
};
#endif
