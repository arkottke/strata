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

    friend QDataStream & operator<< (QDataStream & out, const SoilTypeCatalog* stc);
    friend QDataStream & operator>> (QDataStream & in, SoilTypeCatalog* stc);

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

    QString toHtml() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index ) const;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    SoilType* soilType(int row);
    int rowOf(SoilType* st) const;

    //! Determine the appropriate SoilType based on a QVariant
    SoilType* soilTypeOf(QVariant value);

    //! Catalog of nonlinear models
    NonlinearPropertyCatalog* nlCatalog();

    void fromJson(const QJsonArray &json);
    QJsonArray toJson() const;

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
