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

#include "TimeSeriesOutputCatalog.h"

#include "AbstractOutput.h"
#include "AbstractTimeSeriesOutput.h"
#include "AccelTimeSeriesOutput.h"
#include "Algorithms.h"
#include "DispTimeSeriesOutput.h"
#include "AbstractMotion.h"
#include "StrainTimeSeriesOutput.h"
#include "StressTimeSeriesOutput.h"
#include "VelTimeSeriesOutput.h"
#include "ViscoElasticStressTimeSeriesOutput.h"

#include <QDebug>

TimeSeriesOutputCatalog::TimeSeriesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractMutableOutputCatalog(outputCatalog)
{
    _lookup["Acceleration Time Series"] = "AccelTimeSeriesOutput";
    _lookup["Velocity Time Series"] = "VelTimeSeriesOutput";
    _lookup["Displacement Time Series"] = "DispTimeSeriesOutput";
    _lookup["Shear-Strain Time Series"] = "StrainTimeSeriesOutput";
    _lookup["Shear-Stress Time Series"] = "StressTimeSeriesOutput";

#ifdef ADVANCED_FEATURES
    _lookup["Visco-Elastic Shear-Stress Time Series"] = "ViscoElasticStressTimeSeriesOutput";
#endif
}

auto TimeSeriesOutputCatalog::needsOutputConditions() const -> bool
{
    return true;
}

auto TimeSeriesOutputCatalog::rowCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);

    return _outputs.size();
}

auto TimeSeriesOutputCatalog::columnCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);
    return 4;
}

auto TimeSeriesOutputCatalog::data(const QModelIndex & index, int role) const -> QVariant
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            return _outputs.at(index.row())->name();
        case DepthColumn:
            if (role == Qt::DisplayRole) {
                return locationToString(_outputs.at(index.row())->depth());
            } else {
                return _outputs.at(index.row())->depth();
            }
        case TypeColumn:
            if (role == Qt::DisplayRole) {
                return AbstractMotion::typeList().at(
                        _outputs.at(index.row())->type());
            } else {
                return _outputs.at(index.row())->type();
            }
        default:
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case CorrectedColumn:
            return _outputs.at(index.row())->baselineCorrect() ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
        }
    }

    return AbstractOutputCatalog::data(index, role);
}

auto TimeSeriesOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role) -> bool
{
    if (index.parent()!=QModelIndex() || _readOnly)
        return false;

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            return false;
        case DepthColumn:
            {
                bool ok;
                const double d = value.toDouble(&ok);

                if (ok) {
                    _outputs[index.row()]->setDepth(d);
                } else {
                    return false;
                }
                break;
            }
        case TypeColumn:
            {
                bool ok;
                AbstractMotion::Type type = AbstractMotion::variantToType(value, &ok);

                if (ok) {
                    _outputs[index.row()]->setType(type);
                } else
                    return false;

                break;
            }
        }
    } else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case CorrectedColumn:
            _outputs[index.row()]->setBaselineCorrect(value.toBool());
            break;
        default:
            return false;
        }

    }

    emit wasModified();
    emit dataChanged(index, index);

    return true;
}

auto TimeSeriesOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case NameColumn:
            return tr("Name");
        case DepthColumn:
            return tr("Location");
        case TypeColumn:
            return tr("Type");
        case CorrectedColumn:
            return tr("Baseline Correct");
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

auto TimeSeriesOutputCatalog::flags(const QModelIndex & index) const -> Qt::ItemFlags
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    switch (index.column()) {
    case DepthColumn:
        flags |= Qt::ItemIsEditable;
        break;
    case TypeColumn:
        if (qobject_cast<const StrainTimeSeriesOutput* const>(_outputs.at(index.row()))
            || qobject_cast<const StressTimeSeriesOutput* const>(_outputs.at(index.row()))
            || qobject_cast<const ViscoElasticStressTimeSeriesOutput* const>(_outputs.at(index.row()))
            ) {
            // Do nothing because it can only be within motions.
        } else {
            flags |= Qt::ItemIsEditable;
        }
        break;
    case CorrectedColumn:
        flags |= Qt::ItemIsUserCheckable;
        break;
    case NameColumn:
    default:
        break;
    }

    return flags;
}

auto TimeSeriesOutputCatalog::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        _outputs.takeAt(row)->deleteLater();

    emit endRemoveRows();
    emit wasModified();

    if (!rowCount())
        emit timesAreNeededChanged(false);

    return true;
}

void TimeSeriesOutputCatalog::addRow(const QString &name)
{
    if (_lookup.contains(name)) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        _outputs << factory(_lookup.value(name), _outputCatalog);

        connect(_outputs.last(), SIGNAL(wasModified()),
                this, SIGNAL(wasModified()));

        endInsertRows();

        emit wasModified();
    }
}

auto TimeSeriesOutputCatalog::outputs() const -> QList<AbstractOutput*>
{
    QList<AbstractOutput*> list;

    for (auto *atso : _outputs)
        list << static_cast<AbstractOutput*>(atso);

    return list;
}

auto TimeSeriesOutputCatalog::factory(const QString & className, OutputCatalog * parent) const -> AbstractTimeSeriesOutput*
{
    AbstractTimeSeriesOutput* atso = 0;

    if (className == "AccelTimeSeriesOutput") {
        atso = new AccelTimeSeriesOutput(parent);
    } else if (className == "VelTimeSeriesOutput") {
        atso = new VelTimeSeriesOutput(parent);
    } else if (className == "DispTimeSeriesOutput") {
        atso = new DispTimeSeriesOutput(parent);
    } else if (className == "StrainTimeSeriesOutput") {
        atso = new StrainTimeSeriesOutput(parent);
    } else if (className == "StressTimeSeriesOutput") {
        atso = new StressTimeSeriesOutput(parent);
    } else if (className == "ViscoElasticStressTimeSeriesOutput") {
        atso = new ViscoElasticStressTimeSeriesOutput(parent); 
    }

    Q_ASSERT(atso);

    emit timesAreNeededChanged(true);
    return atso;
}

void TimeSeriesOutputCatalog::fromJson(const QJsonArray &array)
{
    beginResetModel();

    while (_outputs.size())
        _outputs.takeLast()->deleteLater();

    for (const QJsonValue &v : array) {
        QJsonObject json = v.toObject();
        AbstractTimeSeriesOutput *atso = factory(json["className"].toString(), _outputCatalog);
        atso->fromJson(json);
        _outputs << atso;
    }

    endResetModel();
}

auto TimeSeriesOutputCatalog::toJson() const -> QJsonArray
{
    QJsonArray array;
    for (auto *atso : _outputs)
        array << atso->toJson();

    return array;
}


auto operator<< (QDataStream & out, const TimeSeriesOutputCatalog* tsoc) -> QDataStream &
{
    out << (quint8)1;

    out << tsoc->_outputs.size();

    for (auto *atso : tsoc->_outputs)
        out << QString(atso->metaObject()->className()) << atso;

    return out;
}

auto operator>> (QDataStream & in, TimeSeriesOutputCatalog* tsoc) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    tsoc->beginResetModel();
    QString name;
    while (tsoc->_outputs.size() < size) {
        in >> name;
        tsoc->_outputs << tsoc->factory(name, tsoc->_outputCatalog);
        in >> tsoc->_outputs.last();
    }
    tsoc->endResetModel();

    return in;
}
