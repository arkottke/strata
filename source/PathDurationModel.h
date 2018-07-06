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

#ifndef PATHDURATIONMODEL_H
#define PATHDURATIONMODEL_H

#include "MyAbstractTableModel.h"

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>
#include <QVector>

class PathDurationModel : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const PathDurationModel* pdm);
    friend QDataStream & operator>> (QDataStream & in, PathDurationModel* pdm);

public:
    explicit PathDurationModel(QObject *parent = 0);

    enum Columns {
        DistanceColumn,
        RateColumn,
    };

    void setRegion(AbstractRvtMotion::Region region);

    //!@{ Methods for QAbstractTableModel
    virtual int rowCount(const QModelIndex &parent) const;

    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual Qt::ItemFlags flags( const QModelIndex & index) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!@}

    /*! Compute the crustal amplification
     */
    double duration(double distance) const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void wasModified();

private:
    //! Distance (km)
    QVector<double> _distance;
    //! Duration rate (sec)
    QVector<double> _rate;
};

#endif // PATHDURATIONMODEL_H
