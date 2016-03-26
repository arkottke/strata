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

#include "CustomNonlinearProperty.h"

CustomNonlinearProperty::CustomNonlinearProperty(Type type, bool retain, QObject *parent)
    : NonlinearProperty(parent), m_retain(retain)
{
    m_name = "Custom";
    m_type = type;
}

void CustomNonlinearProperty::setName(const QString &name)
{
    m_name = name;
}

bool CustomNonlinearProperty::retain() const
{
    return m_retain;
}

bool CustomNonlinearProperty::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole) {
        return false;
    }

    bool b;
    const double d = value.toDouble(&b);

    if (b) {
        switch (index.column()) {
        case StrainColumn:
            m_strain[index.row()] = d;
            break;
        case PropertyColumn:
            m_average[index.row()] = d;
            m_varied[index.row()] = d;
            gsl_interp_accel_reset(m_acc);
            break;
        }
        dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags CustomNonlinearProperty::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool CustomNonlinearProperty::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginInsertRows(parent, row, row+count-1);

    m_strain.insert(row, count, 0);
    m_average.insert(row, count, 0);
    m_varied.insert(row, count, 0);

    emit endInsertRows();
    return true;
}

bool CustomNonlinearProperty::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginRemoveRows(parent, row, row+count-1);

    m_strain.remove(row, count);
    m_average.remove(row, count);
    m_varied.remove(row, count);

    emit endRemoveRows();
    return true;
}
