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
#include "AriasIntensityProfileOutput.h"
#include "DampingProfileOutput.h"
#include "DissipatedEnergyProfileOutput.h"
#include "FinalVelProfileOutput.h"
#include "InitialVelProfileOutput.h"
#include "MaxAccelProfileOutput.h"
#include "MaxDispProfileOutput.h"
#include "MaxErrorProfileOutput.h"
#include "MaxStrainProfileOutput.h"
#include "MaxStressProfileOutput.h"
#include "MaxVelProfileOutput.h"
#include "MotionLibrary.h"
#include "ModulusProfileOutput.h"
#include "StressRatioProfileOutput.h"
#include "StressReducCoeffProfileOutput.h"
#include "VerticalEffectiveStressProfileOutput.h"
#include "VerticalTotalStressProfileOutput.h"

#include <QStringList>
#include <QDebug>

ProfilesOutputCatalog::ProfilesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractOutputCatalog(outputCatalog)
{
    m_outputs << new AriasIntensityProfileOutput(m_outputCatalog)
              << new DampingProfileOutput(m_outputCatalog)
              << new DissipatedEnergyProfileOutput(m_outputCatalog)
              << new FinalVelProfileOutput(m_outputCatalog)
              << new InitialVelProfileOutput(m_outputCatalog)
              << new MaxAccelProfileOutput(m_outputCatalog)
              << new MaxDispProfileOutput(m_outputCatalog)
              << new MaxErrorProfileOutput(m_outputCatalog)
              << new MaxStrainProfileOutput(m_outputCatalog)
              << new MaxStressProfileOutput(m_outputCatalog)
              << new MaxVelProfileOutput(m_outputCatalog)
              << new ModulusProfileOutput(m_outputCatalog)
              << new StressRatioProfileOutput(m_outputCatalog)
              << new StressReducCoeffProfileOutput(m_outputCatalog)
              << new VerticalTotalStressProfileOutput(m_outputCatalog)
              << new VerticalEffectiveStressProfileOutput(m_outputCatalog);

    foreach (AbstractProfileOutput* output, m_outputs)
        connect(output, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
}

int ProfilesOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();;
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

    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    // Remove the Enabled flag for RVT motions if the output is for time series only.
    if (m_approach == MotionLibrary::RandomVibrationTheory
            && m_outputs.at(index.row())->timeSeriesOnly())
        flags &= ~Qt::ItemIsEnabled;

    return flags;
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

 
void ProfilesOutputCatalog::ptRead(const ptree &pt)
{
    QMap<QString, AbstractProfileOutput*> output_map = QMap<QString, AbstractProfileOutput*>();
    foreach (AbstractProfileOutput* o, m_outputs)
        output_map.insert(o->metaObject()->className(), o);

    beginResetModel();

    foreach(const ptree::value_type &v, pt)
    {
        QString key = QString::fromStdString(
                    v.second.get<std::string>("className"));
        if (output_map.contains(key)) {
            output_map[key]->ptRead(v.second);
        }
    }

    endResetModel();
}

void ProfilesOutputCatalog::ptWrite(ptree &pt) const
{
    foreach(AbstractProfileOutput *apo, m_outputs) {
        ptree apt;
        apo->ptWrite(apt);
        pt.push_back(std::make_pair("", apt));
    }
}

QDataStream & operator<< (QDataStream & out, const ProfilesOutputCatalog* poc)
{
    out << (quint8)4;

    foreach (AbstractProfileOutput* apo, poc->m_outputs) {
        out << apo;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, ProfilesOutputCatalog* poc)
{
    quint8 ver;
    in >> ver;
    poc->beginResetModel();

    foreach (AbstractProfileOutput* apo, poc->m_outputs) {
        // Skip profiles not included in earlier versions
        if (ver < 2 && qobject_cast<MaxDispProfileOutput*>(apo))
            continue;
        if (ver < 2 && qobject_cast<DissipatedEnergyProfileOutput*>(apo))
            continue;
        if (ver < 3 && qobject_cast<VerticalEffectiveStressProfileOutput*>(apo))
            continue;
        if (ver < 4 && qobject_cast<AriasIntensityProfileOutput*>(apo))
            continue;

        in >> apo;
    }

    poc->endResetModel();
    return in;
}
