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

#include <QDebug>

TimeSeriesOutputCatalog::TimeSeriesOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractMutableOutputCatalog(outputCatalog)
{
    m_lookup["Acceleration Time Series"] = "AccelTimeSeriesOutput";
    m_lookup["Velocity Time Series"] = "VelTimeSeriesOutput";
    m_lookup["Displacement Time Series"] = "DispTimeSeriesOutput";
    m_lookup["Shear-Strain Time Series"] = "StrainTimeSeriesOutput";
    m_lookup["Shear-Stress Time Series"] = "StressTimeSeriesOutput";
}

bool TimeSeriesOutputCatalog::needsOutputConditions() const
{
    return true;
}

int TimeSeriesOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();
}

int TimeSeriesOutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant TimeSeriesOutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            return m_outputs.at(index.row())->name();
        case DepthColumn:
            if (role == Qt::DisplayRole) {
                return locationToString(m_outputs.at(index.row())->depth());
            } else {
                return m_outputs.at(index.row())->depth();
            }
        case TypeColumn:
            if (role == Qt::DisplayRole) {
                return AbstractMotion::typeList().at(
                        m_outputs.at(index.row())->type());
            } else {
                return m_outputs.at(index.row())->type();
            }
        default:
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case CorrectedColumn:
            return m_outputs.at(index.row())->baselineCorrect() ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
        }
    }

    return AbstractOutputCatalog::data(index, role);
}

bool TimeSeriesOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent()!=QModelIndex() || m_readOnly)
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
                    m_outputs[index.row()]->setDepth(d);
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
                    m_outputs[index.row()]->setType(type);
                } else
                    return false;

                break;
            }
        }
    } else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case CorrectedColumn:
            m_outputs[index.row()]->setBaselineCorrect(value.toBool());
            break;
        default:
            return false;
        }

    }

    emit wasModified();
    emit dataChanged(index, index);

    return true;
}

QVariant TimeSeriesOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags TimeSeriesOutputCatalog::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    switch (index.column()) {
    case DepthColumn:
        flags |= Qt::ItemIsEditable;
        break;
    case TypeColumn:
        if (qobject_cast<const StrainTimeSeriesOutput* const>(m_outputs.at(index.row()))
            || qobject_cast<const StressTimeSeriesOutput* const>(m_outputs.at(index.row()))) {
            // Do nothing
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

bool TimeSeriesOutputCatalog::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        m_outputs.takeAt(row)->deleteLater();

    emit endRemoveRows();
    emit wasModified();

    if (!rowCount())
        emit timesAreNeededChanged(false);

    return true;
}

void TimeSeriesOutputCatalog::addRow(const QString &name)
{
    if (m_lookup.contains(name)) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_outputs << factory(m_lookup.value(name), m_outputCatalog);

        connect(m_outputs.last(), SIGNAL(wasModified()),
                this, SIGNAL(wasModified()));

        endInsertRows();

        emit wasModified();
    }
}

QList<AbstractOutput*> TimeSeriesOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach(AbstractTimeSeriesOutput* atso, m_outputs )
        list << static_cast<AbstractOutput*>(atso);

    return list;
}

AbstractTimeSeriesOutput* TimeSeriesOutputCatalog::factory(const QString & className, OutputCatalog * parent) const
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
    }

    Q_ASSERT(atso);

    emit timesAreNeededChanged(true);
    return atso;
}


QDataStream & operator<< (QDataStream & out, const TimeSeriesOutputCatalog* tsoc)
{
    out << (quint8)1;

    out << tsoc->m_outputs.size();

    foreach (const AbstractTimeSeriesOutput* atso, tsoc->m_outputs)
        out << QString(atso->metaObject()->className()) << atso;

    return out;
}

QDataStream & operator>> (QDataStream & in, TimeSeriesOutputCatalog* tsoc)
{    
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    tsoc->beginResetModel();
    QString name;
    while (tsoc->m_outputs.size() < size) {
        in >> name;
        tsoc->m_outputs << tsoc->factory(name, tsoc->m_outputCatalog);
        in >> tsoc->m_outputs.last();
    }
    tsoc->endResetModel();

    return in;
}
