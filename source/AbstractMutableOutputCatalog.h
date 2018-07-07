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

#ifndef ABSTRACT_MUTABLE_OUTPUT_CATALOG_H
#define ABSTRACT_MUTABLE_OUTPUT_CATALOG_H

#include "AbstractOutputCatalog.h"

#include <QStringList>

class AbstractMutableOutputCatalog : public AbstractOutputCatalog
{
Q_OBJECT
public:
    explicit AbstractMutableOutputCatalog(OutputCatalog *outputCatalog);

    //! If the OutputCatalog needs input information
    virtual bool needsInputConditions() const;

    //! If the OutputCatalog needs output information
    virtual bool needsOutputConditions() const;

    virtual void addRow(const QString &name) = 0;
    virtual QStringList names() const;

protected:
    //! Relationship between human readable and QMetaObject classnames
    QMap<QString, QString> _lookup;
};

#endif // ABSTRACT_MUTABLE_OUTPUT_CATALOG_H
