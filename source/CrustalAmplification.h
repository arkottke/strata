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

    friend auto operator<< (QDataStream & out, const CrustalAmplification* ca) -> QDataStream &;
    friend auto operator>> (QDataStream & in, CrustalAmplification* ca) -> QDataStream &;

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

    static auto sourceList() -> QStringList;

    void setRegion(AbstractRvtMotion::Region region);
    void setSource(Source source);
    auto source() const -> Source;

    //!@{ Methods for QAbstractTableModel
    virtual auto rowCount(const QModelIndex & parent = QModelIndex()) const -> int;
    virtual auto columnCount(const QModelIndex & parent = QModelIndex()) const -> int;

    virtual auto headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant;

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;
    virtual auto setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) -> bool;

    virtual auto flags( const QModelIndex & index) const -> Qt::ItemFlags;

    virtual auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    virtual auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    //!@}

    auto crustalModel() -> CrustalModel*;

    auto interpAmpAt(double freq) -> double;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

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
