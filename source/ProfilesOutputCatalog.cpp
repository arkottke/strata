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

#include <QDebug>
#include <QStringList>

ProfilesOutputCatalog::ProfilesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractOutputCatalog(outputCatalog)
{
    _outputs << new AriasIntensityProfileOutput(_outputCatalog)
              << new DampingProfileOutput(_outputCatalog)
              << new DissipatedEnergyProfileOutput(_outputCatalog)
              << new FinalVelProfileOutput(_outputCatalog)
              << new InitialVelProfileOutput(_outputCatalog)
              << new MaxAccelProfileOutput(_outputCatalog)
              << new MaxDispProfileOutput(_outputCatalog)
              << new MaxErrorProfileOutput(_outputCatalog)
              << new MaxStrainProfileOutput(_outputCatalog)
              << new MaxStressProfileOutput(_outputCatalog)
              << new MaxVelProfileOutput(_outputCatalog)
              << new ModulusProfileOutput(_outputCatalog)
              << new StressRatioProfileOutput(_outputCatalog)
              << new StressReducCoeffProfileOutput(_outputCatalog)
              << new VerticalTotalStressProfileOutput(_outputCatalog)
              << new VerticalEffectiveStressProfileOutput(_outputCatalog);

    for (auto *output : _outputs)
        connect(output, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
}

auto ProfilesOutputCatalog::rowCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);

    return _outputs.size();;
}

auto ProfilesOutputCatalog::columnCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);

    return 1;
}

auto ProfilesOutputCatalog::data(const QModelIndex & index, int role) const -> QVariant
{
    if (index.parent() != QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        return _outputs.at(index.row())->name();
    } else if (role == Qt::CheckStateRole) {
        return _outputs.at(index.row())->enabled() ? Qt::Checked : Qt::Unchecked;
    }

    return AbstractOutputCatalog::data(index, role);
}

auto ProfilesOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role) -> bool
{
    if (index.parent() != QModelIndex() || _readOnly)
        return false;

    if (role == Qt::CheckStateRole) {
        _outputs[index.row()]->setEnabled(value.toBool());
    } else {
        return false;
    }

    emit dataChanged(index, index);
    emit wasModified();
    return false;
}

auto ProfilesOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const -> QVariant
{
    Q_UNUSED(section);

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return tr("Name");

    return QVariant();
}

auto ProfilesOutputCatalog::flags(const QModelIndex & index) const -> Qt::ItemFlags
{
    Q_UNUSED(index);

    Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    // Remove the Enabled flag for RVT motions if the output is for time series only.
    if (_approach == MotionLibrary::RandomVibrationTheory
            && _outputs.at(index.row())->timeSeriesOnly())
        flags &= ~Qt::ItemIsEnabled;

    return flags;
}

auto ProfilesOutputCatalog::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        _outputs.takeAt(row)->deleteLater();

    emit endRemoveRows();
    return true;
}

auto ProfilesOutputCatalog::outputs() const -> QList<AbstractOutput*>
{
    QList<AbstractOutput*> list;

    for (AbstractProfileOutput *apo : _outputs)
        if (apo->enabled())
            list << static_cast<AbstractOutput*>(apo);

    return list;
}
 
void ProfilesOutputCatalog::fromJson(const QJsonArray &json)
{
    beginResetModel();

    QMap<QString, AbstractProfileOutput*> output_map;
    for (AbstractProfileOutput *o : _outputs)
        output_map.insert(o->metaObject()->className(), o);

    for (const QJsonValue &qjv : json) {
        QJsonObject qjo = qjv.toObject();
        QString key = qjo["className"].toString();
        if (output_map.contains(key))
            output_map[key]->fromJson(qjo);
    }

    endResetModel();
}

auto ProfilesOutputCatalog::toJson() const -> QJsonArray
{
    QJsonArray json;
    for (AbstractProfileOutput *apo : _outputs) {
        json << apo->toJson();
    }

    return json;
}

auto operator<< (QDataStream & out, const ProfilesOutputCatalog* poc) -> QDataStream &
{
    out << (quint8)4;

    for (AbstractProfileOutput *apo : poc->_outputs) {
        out << apo;
    }

    return out;
}

auto operator>> (QDataStream & in, ProfilesOutputCatalog* poc) -> QDataStream &
{
    quint8 ver;
    in >> ver;
    poc->beginResetModel();

    for (AbstractProfileOutput *apo : poc->_outputs) {
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
