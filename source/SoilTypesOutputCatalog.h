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

    friend QDataStream & operator<< (QDataStream & out, const SoilTypesOutputCatalog* stoc);
    friend QDataStream & operator>> (QDataStream & in, SoilTypesOutputCatalog* stoc);

public:
    explicit SoilTypesOutputCatalog(OutputCatalog *outputCatalog);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

    void setSoilTypeCatalog(SoilTypeCatalog* soilTypeCatalog);
    virtual QList<AbstractOutput*> outputs() const;

    void fromJson(const QJsonArray &json);
    QJsonArray toJson() const;

protected slots:
    void addOutput(SoilType* soilType);
    void removeOutput(SoilType* soilType);

protected:
    QList<SoilTypeOutput*> _outputs;

    //! SoilType catalog
    SoilTypeCatalog* _soilTypeCatalog;
};

#endif // SOIL_TYPES_OUTPUT_CATALOG_H
