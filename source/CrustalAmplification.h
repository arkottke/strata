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

#ifndef CRUSTAL_AMPLIFICATION_H
#define CRUSTAL_AMPLIFICATION_H

#include "MyAbstractTableModel.h"

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <gsl/gsl_interp.h>

class CrustalModel;

class CrustalAmplification : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const CrustalAmplification* ca);
    friend QDataStream & operator>> (QDataStream & in, CrustalAmplification* ca);

public:
    CrustalAmplification(QObject *parent=0);
    ~CrustalAmplification();

    enum Columns {
        FreqColumn,
        AmpColumn
    };

    enum Source {
        Default,
        Specified,
        Calculated
    };

    static QStringList sourceList();

    void setRegion(AbstractRvtMotion::Region region);
    void setSource(Source source);
    Source source() const;

    //!@{ Methods for QAbstractTableModel
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual Qt::ItemFlags flags( const QModelIndex & index) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!@}

    CrustalModel* crustalModel();

    double interpAmpAt(double freq);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void readOnlyChanged(bool b);
    void sourceChanged(int source);
    void wasModified();

public slots:
    void setRegion(int region);
    void setSource(int source);

private slots:
    void calculate();

private:
    //! Initialize the interpolator
    void initInterp();

    //! Clear the interpolator
    void clearInterp();

    //! If the amplification is calculated
    Source _source;

    //! Frequency
    QVector<double> _freq;

    //! Amplification
    QVector<double> _amp;

    //! Specific crustal model
    CrustalModel *_crustalModel;

    //! GSL interpolator
    gsl_interp *_interpolator;

    //! Accelerator for the interpolation
    gsl_interp_accel *_accelerator;
};

#endif // CRUSTAL_AMPLIFICATION_H
