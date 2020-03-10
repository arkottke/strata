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

#ifndef RVT_MOTION_H_
#define RVT_MOTION_H_

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>

/*! A class that uses random vibration theory to compute the reponse.
 * The motion is characterized by the Fourier Amplitude and random vibration
 * thereory is used to predict the expected response.
 */

class RvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const RvtMotion * rm) -> QDataStream &;
    friend auto operator>> (QDataStream & in, RvtMotion * rm) -> QDataStream &;

public:
    //! Constructor
    explicit RvtMotion(QObject *parent = nullptr);

    //!{@ Method for editing the model with a QTableView
    virtual auto flags( const QModelIndex & index) const -> Qt::ItemFlags;

    virtual auto setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) -> bool;

    virtual auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    virtual auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    //!}@

    virtual auto freq() const -> const QVector<double> &;
    virtual auto toHtml() const -> QString;
    virtual auto loadFromTextStream(QTextStream &stream, double scale = 1.) -> bool;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setDuration(double d);

protected:
    //! Vector of frequency values
    QVector<double> _freq;
};

auto logLogInterp( const QVector<double> & x, const QVector<double> & y,
        const QVector<double> & xi, QVector<double> & yi) -> bool;
#endif
