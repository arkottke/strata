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

#ifndef TIME_SERIES_OUTPUT_CATALOG_H
#define TIME_SERIES_OUTPUT_CATALOG_H

#include "AbstractMutableOutputCatalog.h"

#include <QDataStream>
#include <QJsonArray>

class AbstractTimeSeriesOutput;

class TimeSeriesOutputCatalog : public AbstractMutableOutputCatalog
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const TimeSeriesOutputCatalog* tsoc) -> QDataStream &;
    friend auto operator>> (QDataStream & in, TimeSeriesOutputCatalog* tsoc) -> QDataStream &;

public:
    explicit TimeSeriesOutputCatalog(OutputCatalog *outputCatalog);

    auto needsOutputConditions() const -> bool;

    virtual auto rowCount(const QModelIndex & parent = QModelIndex()) const -> int;
    virtual auto columnCount(const QModelIndex & parent = QModelIndex()) const -> int;

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;
    virtual auto setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) -> bool;

    virtual auto headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const -> QVariant;
    virtual auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;

    virtual auto flags(const QModelIndex & index) const -> Qt::ItemFlags;

    virtual void addRow(const QString &name);
    virtual auto outputs() const -> QList<AbstractOutput*>;

    void fromJson(const QJsonArray &array);
    auto toJson() const -> QJsonArray;

protected:   
    auto factory(const QString & className, OutputCatalog * parent) const -> AbstractTimeSeriesOutput*;

    enum Columns {
        NameColumn,
        DepthColumn,
        TypeColumn,
        CorrectedColumn
    };

    QList<AbstractTimeSeriesOutput*> _outputs;
};

#endif // TIME_SERIES_OUTPUT_CATALOG_H
