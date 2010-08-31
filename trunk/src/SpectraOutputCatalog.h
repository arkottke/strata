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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef SPECTRA_OUTPUT_CATALOG_H
#define SPECTRA_OUTPUT_CATALOG_H

#include "AbstractMutableOutputCatalog.h"

class AbstractLocationOutput;

class SpectraOutputCatalog : public AbstractMutableOutputCatalog
{
Q_OBJECT

friend QDataStream & operator<< (QDataStream & out, const SpectraOutputCatalog* soc);
friend QDataStream & operator>> (QDataStream & in, SpectraOutputCatalog* soc);

public:
    explicit SpectraOutputCatalog(OutputCatalog *outputCatalog);

    bool needsOutputConditions() const;

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

    virtual void addRow(const QString &name);
    virtual QList<AbstractOutput*> outputs() const;

protected:
    AbstractLocationOutput* factory(const QString & className, OutputCatalog * parent) const;

    enum Columns {
        NameColumn,
        DepthColumn,
        TypeColumn
    };

    QList<AbstractLocationOutput*> m_outputs;
};

#endif // SPECTRA_OUTPUT_CATALOG_H
