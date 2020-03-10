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

#ifndef SOIL_TYPE_CATALOG_H
#define SOIL_TYPE_CATALOG_H

#include "MyAbstractTableModel.h"

#include <QDataStream>
#include <QJsonArray>

class NonlinearPropertyCatalog;
class SoilType;

class SoilTypeCatalog : public MyAbstractTableModel
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const SoilTypeCatalog* stc) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SoilTypeCatalog* stc) -> QDataStream &;

public:
    SoilTypeCatalog(QObject *parent = nullptr);

    enum Columns {
        NameColumn,
        UnitWeightColumn,
        DampingColumn,
        ModulusModelColumn,
        DampingModelColumn,
        DampLimitColumn,
        NotesColumn,
        IsVariedColumn
    };

    auto toHtml() const -> QString;

    auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int;

    auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant;
    auto setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) -> bool;

    auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant;
    auto flags(const QModelIndex &index ) const -> Qt::ItemFlags;

    auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;

    auto soilType(int row) -> SoilType*;
    auto rowOf(SoilType* st) const -> int;

    //! Determine the appropriate SoilType based on a QVariant
    auto soilTypeOf(QVariant value) -> SoilType*;

    //! Catalog of nonlinear models
    auto nlCatalog() -> NonlinearPropertyCatalog*;

    void fromJson(const QJsonArray &json);
    auto toJson() const -> QJsonArray;

signals:
    void soilTypeAdded(SoilType* soilType);
    void soilTypeRemoved(SoilType* soilType);

protected slots:
    void updateUnits();

protected:
    QList<SoilType*> _soilTypes;

    NonlinearPropertyCatalog* _nlCatalog;
};

#endif // SOIL_TYPE_CATALOG_H
