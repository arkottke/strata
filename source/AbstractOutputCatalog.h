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

#ifndef ABSTRACT_OUTPUT_CATALOG_H
#define ABSTRACT_OUTPUT_CATALOG_H

#include "MyAbstractTableModel.h"
#include "MotionLibrary.h"

#include <QList>

class AbstractOutput;
class OutputCatalog;

class AbstractOutputCatalog : public MyAbstractTableModel
{
    Q_OBJECT

public:
    explicit AbstractOutputCatalog(OutputCatalog *outputCatalog);

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;

    virtual auto outputs() const -> QList<AbstractOutput*> = 0;

public slots:
    void setApproach(int approach);

signals:
    void timesAreNeededChanged(bool timesAreNeeded) const;
    void periodIsNeededChanged(bool periodIsNeeded) const;
    void frequencyIsNeededChanged(bool frequencyIsNeeded) const;

    void wasModified();

protected:
    OutputCatalog* _outputCatalog;
    MotionLibrary::Approach _approach;
};

#endif // ABSTRACT_OUTPUT_CATALOG_H
