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

#ifndef RESPONSE_SPECTRUM_H_
#define RESPONSE_SPECTRUM_H_

#include "MyAbstractTableModel.h"

#include <QDataStream>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>

class ResponseSpectrum : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const ResponseSpectrum* rs);
    friend QDataStream & operator>> (QDataStream & in, ResponseSpectrum* rs);

public:
    ResponseSpectrum(QObject * parent = 0);

    bool modified() const;

    double damping() const;

    const QVector<double> & period() const;
    void setPeriod(const QVector<double> & period);

    const QVector<double> & sa() const;
    void setSa(const QVector<double> & sa);

    //! Scale the response spectrum by a factor
    void scaleBy(double scale);

    void fromMap( const QMap<QString, QVariant> & map);

    int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

    QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool setData ( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags ( const QModelIndex &index ) const;

    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void wasModified();

public slots:
    void setModified(bool modified = true);
    void setDamping(double damping);

private:
    enum Columns {
        PeriodColumn,
        SpecAccelColumn
    };

    //! If the data has been modified -- FIXME still needed?
    bool _modified;

    //! Damping in percent
    double _damping;

    //! Period values in seconds
    QVector<double> _period;

    //! Spectral acceleration in gravity
    QVector<double> _sa;
};
#endif
