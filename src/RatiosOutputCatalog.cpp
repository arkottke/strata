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

#include "RatiosOutputCatalog.h"

#include "AbstractOutput.h"
#include "AbstractRatioOutput.h"
#include "AccelTransferFunctionOutput.h"
#include "Algorithms.h"
#include "SpectralRatioOutput.h"
#include "StrainTransferFunctionOutput.h"

#include <QDebug>

RatiosOutputCatalog::RatiosOutputCatalog(OutputCatalog *outputCatalog)
    : AbstractMutableOutputCatalog(outputCatalog)
{
    m_lookup["Acceleration Transfer Function"] = "AccelTransferFunctionOutput";
    m_lookup["Spectral Ratio"] = "SpectralRatioOutput";
    m_lookup["Strain Transfer Function"] = "StrainTransferFunctionOutput";
}

bool RatiosOutputCatalog::needsInputConditions() const
{
    return true;
}

bool RatiosOutputCatalog::needsOutputConditions() const
{
    return true;
}

int RatiosOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();
}

int RatiosOutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant RatiosOutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            return m_outputs.at(index.row())->name();
        case OutDepthColumn:
            if (role == Qt::DisplayRole) {
                return locationToString(m_outputs.at(index.row())->outDepth());
            } else {
                return m_outputs.at(index.row())->outDepth();
            }
        case OutTypeColumn:
            if (role == Qt::DisplayRole) {
                return AbstractMotion::typeList().at(
                        m_outputs.at(index.row())->outType());
            } else {
                return m_outputs.at(index.row())->outType();
            }
        case InDepthColumn:
            if (role == Qt::DisplayRole) {
                return locationToString(m_outputs.at(index.row())->inDepth());
            } else {
                return m_outputs.at(index.row())->inDepth();
            }
        case InTypeColumn:
            if (role == Qt::DisplayRole) {
                return AbstractMotion::typeList().at(
                        m_outputs.at(index.row())->inType());
            } else {
                return m_outputs.at(index.row())->inType();
            }
        default:
            return QVariant();
        }
    }

    return AbstractOutputCatalog::data(index, role);
}

bool RatiosOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent() != QModelIndex() || m_readOnly)
        return false;

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            return false;
        case OutDepthColumn:
        case InDepthColumn:
            {
                bool ok;
                const double d = value.toDouble(&ok);

                if (ok) {
                    if (index.column() == OutDepthColumn)
                        m_outputs[index.row()]->setOutDepth(d);
                    else if (index.column() == InDepthColumn)
                        m_outputs[index.row()]->setInDepth(d);
                } else {
                    return false;
                }
                break;
            }
        case OutTypeColumn:
        case InTypeColumn:
            {
                bool ok;
                AbstractMotion::Type type = AbstractMotion::variantToType(value, &ok);

                if (ok) {
                    if (index.column() == OutTypeColumn)
                        m_outputs[index.row()]->setOutType(type);
                    else if (index.column() == InTypeColumn)
                        m_outputs[index.row()]->setInType(type);
                } else
                    return false;

                break;
            }            
        default:
            return false;
        }
    }

    emit dataChanged(index, index);
    emit wasModified();

    return true;
}

QVariant RatiosOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case NameColumn:
            return tr("Name");
        case OutDepthColumn:
            return tr("Location 1");
        case OutTypeColumn:
            return tr("Type 1");
        case InDepthColumn:
            return tr("Location 2");
        case InTypeColumn:
            return tr("Type 2");
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

Qt::ItemFlags RatiosOutputCatalog::flags(const QModelIndex & index) const
{
    Q_UNUSED(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    switch (index.column()) {
    case OutDepthColumn:
    case OutTypeColumn:
    case InDepthColumn:
    case InTypeColumn:
        flags |= Qt::ItemIsEditable;
    case NameColumn:
        break;
    }

    return flags;
}

bool RatiosOutputCatalog::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i) {
        AbstractRatioOutput* aro = m_outputs.takeAt(row);

        if (aro->needsFreq()) {
            // Check if remaining outputs needs frequencies
            bool needsFreq = false;
            foreach (AbstractRatioOutput* _aro, m_outputs) {
                if (_aro->needsFreq()) {
                    needsFreq = true;
                    break;
                }
            }

            if (!needsFreq)
                emit frequencyIsNeededChanged(needsFreq);

        } else if (aro->needsPeriod()) {
            // Check if remaining outputs needs period
            bool needsPeriod = false;
            foreach (AbstractRatioOutput* _aro, m_outputs) {
                if (_aro->needsPeriod()) {
                    needsPeriod = true;
                    break;
                }
            }

            if (!needsPeriod)
                emit periodIsNeededChanged(needsPeriod);
        }

        aro->deleteLater();
    }

    emit endRemoveRows();
    emit wasModified();

    return true;
}


void RatiosOutputCatalog::addRow(const QString &name)
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

QList<AbstractOutput*> RatiosOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach (AbstractRatioOutput* aro, m_outputs)
        list << static_cast<AbstractOutput*>(aro);

    return list;
}

AbstractRatioOutput* RatiosOutputCatalog::factory(const QString & className, OutputCatalog * parent) const
{
    AbstractRatioOutput* aro = 0;

    if (className == "AccelTransferFunctionOutput") {
        aro = new AccelTransferFunctionOutput(parent);
        emit frequencyIsNeededChanged(true);
    } else if (className == "SpectralRatioOutput") {
        aro = new SpectralRatioOutput(parent);
        emit periodIsNeededChanged(true);
    } else if (className == "StrainTransferFunctionOutput") {
        aro = new StrainTransferFunctionOutput(parent);
        emit frequencyIsNeededChanged(true);
    }
    Q_ASSERT(aro);

    return aro;
}

void RatiosOutputCatalog::fromJson(const QJsonArray &json)
{
    beginResetModel();
    m_outputs.clear();

    foreach (const QJsonValue &qjv, json) {
        QJsonObject qjo = qjv.toObject();
        AbstractRatioOutput *aro = factory(qjo["className"].toString(), m_outputCatalog);
        aro->fromJson(qjo);
        m_outputs << aro;
    }

    endResetModel();
}

QJsonArray RatiosOutputCatalog::toJson() const
{
    QJsonArray json;
    foreach (AbstractRatioOutput *aro, m_outputs) {
        json << aro->toJson();
    }

    return json;
}

QDataStream & operator<< (QDataStream & out, const RatiosOutputCatalog* roc)
{
    out << (quint8)1;

    out << roc->m_outputs.size();

    foreach (const AbstractRatioOutput* aro, roc->m_outputs)
        out << QString(aro->metaObject()->className()) << aro;

    return out;
}

QDataStream & operator>> (QDataStream & in, RatiosOutputCatalog* roc)
{
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    roc->beginResetModel();
    QString name;
    while (roc->m_outputs.size() < size) {
        in >> name;
        roc->m_outputs << roc->factory(name, roc->m_outputCatalog);
        in >> roc->m_outputs.last();
    }
    roc->endResetModel();

    return in;
}
