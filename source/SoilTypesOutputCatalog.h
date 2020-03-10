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

#ifndef SOIL_TYPES_OUTPUT_CATALOG_H
#define SOIL_TYPES_OUTPUT_CATALOG_H

#include "AbstractOutputCatalog.h"

#include <QDataStream>
#include <QJsonArray>

class SoilType;
class SoilTypeOutput;
class SoilTypeCatalog;

class SoilTypesOutputCatalog : public AbstractOutputCatalog
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const SoilTypesOutputCatalog* stoc) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SoilTypesOutputCatalog* stoc) -> QDataStream &;

public:
    explicit SoilTypesOutputCatalog(OutputCatalog *outputCatalog);

    virtual auto rowCount(const QModelIndex & parent = QModelIndex()) const -> int;
    virtual auto columnCount(const QModelIndex & parent = QModelIndex()) const -> int;

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;
    virtual auto setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) -> bool;

    virtual auto headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const -> QVariant;

    virtual auto flags(const QModelIndex & index) const -> Qt::ItemFlags;

    void setSoilTypeCatalog(SoilTypeCatalog* soilTypeCatalog);
    virtual auto outputs() const -> QList<AbstractOutput*>;

    void fromJson(const QJsonArray &json);
    auto toJson() const -> QJsonArray;

protected slots:
    void addOutput(SoilType* soilType);
    void removeOutput(SoilType* soilType);

protected:
    QList<SoilTypeOutput*> _outputs;

    //! SoilType catalog
    SoilTypeCatalog* _soilTypeCatalog;
};

#endif // SOIL_TYPES_OUTPUT_CATALOG_H
