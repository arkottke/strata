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

#include "AbstractNonlinearPropertyFactory.h"

#include "CustomNonlinearProperty.h"
#include "DarendeliNonlinearProperty.h"

#include <QBrush>
#include <QColor>
#include <QDebug>

AbstractNonlinearPropertyFactory::AbstractNonlinearPropertyFactory(QObject *parent)
    : QAbstractListModel(parent)
{
}

AbstractNonlinearPropertyFactory::~AbstractNonlinearPropertyFactory()
{
}

int AbstractNonlinearPropertyFactory::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_models.size();
}

QVariant AbstractNonlinearPropertyFactory::data(const QModelIndex &index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
        return QBrush(QColor(200,200,200));

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return m_models.at(index.row())->name();
    }

    return QVariant();
}

bool AbstractNonlinearPropertyFactory::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole)
        return false;

    CustomNonlinearProperty *cnp = qobject_cast<CustomNonlinearProperty*>(m_models.at(index.row()));

    if (cnp) {
        cnp->setName(value.toString());
        dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags AbstractNonlinearPropertyFactory::flags(const QModelIndex &index) const
{
    if (index.row() < 2 || !qobject_cast<const CustomNonlinearProperty*>(m_models.at(index.row()))) {
        return QAbstractItemModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }
}

QVariant AbstractNonlinearPropertyFactory::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        return tr("Model Name");
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}

bool AbstractNonlinearPropertyFactory::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginInsertRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        m_models.insert(row, new CustomNonlinearProperty(m_type, true));

    emit endInsertRows();
    return true;
}

bool AbstractNonlinearPropertyFactory::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    for (int i = 0; i < count; ++i) {
        if (flags(index(row)) & Qt::ItemIsEditable) {
            emit beginRemoveRows(parent, row, row);
            m_models.takeAt(row)->deleteLater();
            emit endRemoveRows();
        } else {
            return false;
        }
    }
    return true;
}


NonlinearProperty* AbstractNonlinearPropertyFactory::modelAt(int row) const
{
    return m_models.at(row);
}

NonlinearProperty* AbstractNonlinearPropertyFactory::duplicateAt(int row) const
{
    if (row == 0) {
        return new CustomNonlinearProperty(m_type, false);
    } else if (row == 1) {
        return new DarendeliNonlinearProperty(m_type);
    } else {
        return m_models.at(row)->duplicate();
    }
}

NonlinearProperty* AbstractNonlinearPropertyFactory::duplicateAt(QVariant value) const
{
    int i = -1;
    if (value.type() == QVariant::Int) {
        i = value.toInt();
    } else if (value.type() == QVariant::String) {
        // Strings might come from the clipboard and actually be integers
        QString s = value.toString();

        bool ok;
        i = s.toInt(&ok);

        if (!ok) {
            for (int j = 0; j < m_models.size(); ++j) {
                if (m_models.at(j)->name() == s) {
                    i = j;
                    break;
                }
            }
        }
    }

    if (0 <= i && i < rowCount()) {
        return duplicateAt(i);
    } else {
        return 0;
    }
}

QDataStream & operator<<(QDataStream & out, const AbstractNonlinearPropertyFactory & anpf)
{
    out << (quint8)1;

    // Create a list of models that need to be saved
    QList<NonlinearProperty*>models;
    foreach (NonlinearProperty* np, anpf.m_models) {
        const CustomNonlinearProperty *cnp = qobject_cast<const CustomNonlinearProperty*>(np);

        if (cnp && cnp->retain()) {
            models << np;
        }
    }

    // Save the data
    out << models;

    return out;
}

QDataStream & operator>>(QDataStream & in, AbstractNonlinearPropertyFactory & anpf)
{
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    for (int i = 0; i < size; ++i) {
        CustomNonlinearProperty* cnp = new CustomNonlinearProperty(anpf.m_type, true);
        in >> cnp;
        anpf.m_models << cnp;
    }

    return in;
}
