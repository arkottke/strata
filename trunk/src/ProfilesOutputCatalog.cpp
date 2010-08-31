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

#include "ProfilesOutputCatalog.h"

#include "AbstractProfileOutput.h"
#include "DampingProfileOutput.h"
#include "FinalVelProfileOutput.h"
#include "InitialVelProfileOutput.h"
#include "MaxAccelProfileOutput.h"
#include "MaxErrorProfileOutput.h"
#include "MaxStrainProfileOutput.h"
#include "MaxStressProfileOutput.h"
#include "MaxVelProfileOutput.h"
#include "ModulusProfileOutput.h"
#include "StressRatioProfileOutput.h"
#include "StressReducCoeffProfileOutput.h"
#include "VerticalStressProfileOutput.h"

#include <QStringList>
#include <QDebug>

ProfilesOutputCatalog::ProfilesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractOutputCatalog(outputCatalog)
{
    m_outputs
            << new DampingProfileOutput(m_outputCatalog)
            << new FinalVelProfileOutput(m_outputCatalog)
            << new InitialVelProfileOutput(m_outputCatalog)
            << new MaxAccelProfileOutput(m_outputCatalog)
            << new MaxErrorProfileOutput(m_outputCatalog)
            << new MaxStrainProfileOutput(m_outputCatalog)
            << new MaxStressProfileOutput(m_outputCatalog)
            << new MaxVelProfileOutput(m_outputCatalog)
            << new ModulusProfileOutput(m_outputCatalog)
            << new StressRatioProfileOutput(m_outputCatalog)
            << new StressReducCoeffProfileOutput(m_outputCatalog)
            << new VerticalStressProfileOutput(m_outputCatalog);

    foreach (AbstractProfileOutput* output, m_outputs)
        connect(output, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
}

int ProfilesOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();
}

int ProfilesOutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant ProfilesOutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent() != QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return m_outputs.at(index.row())->name();
    } else if (role == Qt::CheckStateRole) {
        return m_outputs.at(index.row())->enabled() ? Qt::Checked : Qt::Unchecked;
    }

    return AbstractOutputCatalog::data(index, role);
}

bool ProfilesOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent() != QModelIndex() || m_readOnly)
        return false;

    if (role == Qt::CheckStateRole) {
        m_outputs[index.row()]->setEnabled(value.toBool());
    } else {
        return false;
    }

    emit dataChanged(index, index);
    emit wasModified();
    return false;
}

QVariant ProfilesOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return tr("Name");

    return QVariant();
}

Qt::ItemFlags ProfilesOutputCatalog::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);

    return  Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool ProfilesOutputCatalog::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        m_outputs.takeAt(row)->deleteLater();

    emit endRemoveRows();
    return true;
}

QList<AbstractOutput*> ProfilesOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach(AbstractProfileOutput* apo, m_outputs )
        if (apo->enabled())
            list << static_cast<AbstractOutput*>(apo);

    return list;
}


QDataStream & operator<< (QDataStream & out, const ProfilesOutputCatalog* poc)
{    
    out << (quint8)1;

    foreach (AbstractProfileOutput* apo, poc->m_outputs)
        out << apo;

    return out;
}

QDataStream & operator>> (QDataStream & in, ProfilesOutputCatalog* poc)
{
    quint8 ver;
    in >> ver;

    poc->beginResetModel();

    foreach (AbstractProfileOutput* apo, poc->m_outputs)
        in >> apo;

    poc->endResetModel();

    return in;
}
