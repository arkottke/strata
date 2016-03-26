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

#include "SpectraOutputCatalog.h"

#include "AbstractLocationOutput.h"
#include "Algorithms.h"
#include "FourierSpectrumOutput.h"
#include "ResponseSpectrumOutput.h"

#include <QDebug>

SpectraOutputCatalog::SpectraOutputCatalog(OutputCatalog *outputCatalog) :
    AbstractMutableOutputCatalog(outputCatalog)
{
    m_lookup["Fourier Amplitude Spectrum"] = "FourierSpectrumOutput";
    m_lookup["Acceleration Response Spectrum"] = "ResponseSpectrumOutput";
}

bool SpectraOutputCatalog::needsOutputConditions() const
{
    return true;
}

int SpectraOutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return m_outputs.size();
}

int SpectraOutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QVariant SpectraOutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
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
        }
    }

    return AbstractOutputCatalog::data(index, role);
}

bool SpectraOutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
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

                if (ok)
                    m_outputs[index.row()]->setType(type);
                else
                    return false;

                break;
            }
        }
    }

    emit wasModified();
    emit dataChanged(index, index);

    return true;
}

QVariant SpectraOutputCatalog::headerData ( int section, Qt::Orientation orientation, int role) const
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

        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

Qt::ItemFlags SpectraOutputCatalog::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    switch (index.column()) {
    case DepthColumn:
    case TypeColumn:
        flags |= Qt::ItemIsEditable;
    case NameColumn:
        break;
    }
    return flags;
}

bool SpectraOutputCatalog::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i) {
        AbstractLocationOutput* alo = m_outputs.takeAt(row);

        if (alo->needsFreq()) {
            // Check if remaining outputs needs frequencies
            bool needsFreq = false;
            foreach (AbstractLocationOutput* _alo, m_outputs) {
                if (_alo->needsFreq()) {
                    needsFreq = true;
                    break;
                }
            }

            if (!needsFreq)
                emit frequencyIsNeededChanged(needsFreq);

        } else if (alo->needsPeriod()) {
            // Check if remaining outputs needs period
            bool needsPeriod = false;
            foreach (AbstractLocationOutput* _alo, m_outputs) {
                if (_alo->needsPeriod()) {
                    needsPeriod = true;
                    break;
                }
            }

            if (!needsPeriod)
                emit periodIsNeededChanged(needsPeriod);
        }

        alo->deleteLater();
    }

    emit endRemoveRows();
    emit wasModified();
    return true;
}

void SpectraOutputCatalog::addRow(const QString &name)
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

QList<AbstractOutput*> SpectraOutputCatalog::outputs() const
{
    QList<AbstractOutput*> list;

    foreach(AbstractLocationOutput* alo, m_outputs )
        list << static_cast<AbstractOutput*>(alo);

    return list;
}

AbstractLocationOutput* SpectraOutputCatalog::factory(const QString & className, OutputCatalog * parent) const
{
    AbstractLocationOutput* alo = 0;

    if (className == "FourierSpectrumOutput") {
        alo = new FourierSpectrumOutput(parent);
        emit frequencyIsNeededChanged(true);
    } else if (className == "ResponseSpectrumOutput") {
        alo = new ResponseSpectrumOutput(parent);
        emit periodIsNeededChanged(true);
    }

    Q_ASSERT(alo);

    return alo;
}

void SpectraOutputCatalog::fromJson(const QJsonArray &array)
{
    beginResetModel();
    while (m_outputs.size())
        m_outputs.takeLast()->deleteLater();

    foreach (const QJsonValue &v, array) {
        QJsonObject json = v.toObject();
        AbstractLocationOutput *alo = factory(json["className"].toString(), m_outputCatalog);
        alo->fromJson(json);
        m_outputs << alo;
    }
    endResetModel();
}

QJsonArray SpectraOutputCatalog::toJson() const
{
    QJsonArray array;
    foreach (AbstractLocationOutput *alo, m_outputs)
        array << alo->toJson();

    return array;
}


QDataStream & operator<< (QDataStream & out, const SpectraOutputCatalog* soc)
{
    out << (quint8)1;

    out << soc->m_outputs.size();

    foreach (const AbstractLocationOutput* alo, soc->m_outputs)
        out << QString(alo->metaObject()->className()) << alo;

    return out;
}

QDataStream & operator>> (QDataStream & in, SpectraOutputCatalog* soc)
{
    quint8 ver;
    in >> ver;

    int size;
    in >> size;

    soc->beginResetModel();
    QString name;
    while (soc->m_outputs.size() < size) {
        in >> name;
        soc->m_outputs << soc->factory(name, soc->m_outputCatalog);
        in >> soc->m_outputs.last();
    }
    soc->endResetModel();

    return in;
}
