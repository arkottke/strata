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

#ifndef ABSTRACT_NONLINEAR_PROPERTY_FACTORY_H
#define ABSTRACT_NONLINEAR_PROPERTY_FACTORY_H

#include "NonlinearProperty.h"

#include <QAbstractListModel>

class AbstractNonlinearPropertyFactory : public QAbstractListModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractNonlinearPropertyFactory & anpf);
    friend QDataStream & operator>> (QDataStream & in, AbstractNonlinearPropertyFactory & anpf);

public:
    AbstractNonlinearPropertyFactory(QObject *parent = 0);
    ~AbstractNonlinearPropertyFactory();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    //! Return the model at the specified row
    NonlinearProperty* modelAt(int row) const;

    //! Return a duplicate of the model at the specified row
    NonlinearProperty* duplicateAt(int row) const;

    //! Tries the QVariant as an integer and a QString to match a possible model
    NonlinearProperty* duplicateAt(QVariant value) const;

protected:
    //! Type of models the factor produces
    NonlinearProperty::Type m_type;

    QList<NonlinearProperty*> m_models;
};

#endif // ABSTRACTNONLINEARPROPERTYFACTORY_H
