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

#include "SoilTypeCatalog.h"

#include "DampingFactory.h"
#include "ModulusFactory.h"
#include "NonlinearPropertyCatalog.h"
#include "SoilType.h"
#include "Units.h"

#include <QDataStream>
#include <QDebug>

SoilTypeCatalog::SoilTypeCatalog(QObject *parent)
    : MyAbstractTableModel(parent)
{
    m_nlCatalog = new NonlinearPropertyCatalog;

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(updateUnits()));
}

QString SoilTypeCatalog::toHtml() const
{
    QString html = "<a name=\"soil-types\">Soil Types</a>";

    // Create hyper links for soil types
    html += "<ol>";

    foreach (const SoilType* st, m_soilTypes)
        html += QString("<li><a href=\"#%1\">%1</a></li>").arg(st->name());

    // Bedrock information defined in SoilProfile::toHtml()
    html += "<li><a href=\"#Bedrock\">Bedrock<a></li>";

    html += "</ol>";

    // Generate the output for each soil type
    html += "<ol>";
    foreach (const SoilType* st, m_soilTypes)
        html += "<li>" + st->toHtml() + "</li>";

    // Need to leave the ordered list open because the bedrock data will be added to it.

    return html;
}

int SoilTypeCatalog::rowCount(const QModelIndex &parent) const
{
   Q_UNUSED(parent);

   return m_soilTypes.size();
}

int SoilTypeCatalog::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 7;
}

QVariant SoilTypeCatalog::data(const QModelIndex &index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
        case NameColumn:
            return m_soilTypes.at(index.row())->name();
        case UnitWeightColumn:
            return QString::number(m_soilTypes.at(index.row())->untWt(), 'f', 2);
        case DampingColumn:
            return QString::number(m_soilTypes.at(index.row())->damping(), 'f', 2);
        case ModulusModelColumn:
            return m_soilTypes.at(index.row())->modulusModel()->name();
        case DampingModelColumn:
            return m_soilTypes.at(index.row())->dampingModel()->name();
        case NotesColumn:
            return m_soilTypes.at(index.row())->notes();
        case IsVariedColumn:
        default:
            return QVariant();
        }
    } else if (index.column() == IsVariedColumn && role == Qt::CheckStateRole) {
        return m_soilTypes.at(index.row())->isVaried() ? Qt::Checked : Qt::Unchecked;
    }

    return MyAbstractTableModel::data(index, role);
}

bool SoilTypeCatalog::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.parent()!=QModelIndex() || m_readOnly)
        return false;

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
            m_soilTypes.at(index.row())->setName(value.toString());
            break;
        case UnitWeightColumn:
            {
                bool success;
                double d = value.toDouble(&success);

                if (success) {
                    m_soilTypes.at(index.row())->setUntWt(d);
                    break;
                } else {
                    return false;
                }
            }
        case DampingColumn:
            {
                bool success;
                double d = value.toDouble(&success);

                if (success) {
                    m_soilTypes.at(index.row())->setDamping(d);
                    break;
                } else {
                    return false;
                }
            }
        case ModulusModelColumn:
            if (NonlinearProperty* np = m_nlCatalog->modulusFactory()->duplicateAt(value)) {
                m_soilTypes.at(index.row())->setModulusModel(np);
                break;
            } else {
                return false;
            }
        case DampingModelColumn:
            if (NonlinearProperty* np = m_nlCatalog->dampingFactory()->duplicateAt(value)) {
                m_soilTypes.at(index.row())->setDampingModel(np);
                break;
            } else {
                return false;
            }
        case NotesColumn:
            m_soilTypes.at(index.row())->setNotes(value.toString());
            break;
        case IsVariedColumn:
        default:
            return false;
        }
    } else if (index.column() == IsVariedColumn && role == Qt::CheckStateRole) {
        m_soilTypes.at(index.row())->setIsVaried(value.toBool());
    } else {
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

QVariant SoilTypeCatalog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case NameColumn:
            return tr("Name");
        case UnitWeightColumn:
            return QString(tr("Unit Weight (%1)")).arg(Units::instance()->untWt());
        case DampingColumn:
            return tr("Damping (%)");
        case ModulusModelColumn:
            return tr("G/G_max Model");
        case DampingModelColumn:
            return tr("Damping Model");
        case NotesColumn:
            return tr("Notes");
        case IsVariedColumn:
            return tr("Varied");
        }
    case Qt::Vertical:
        return section+1;
                default:
        return QVariant();
    }
}

Qt::ItemFlags SoilTypeCatalog::flags(const QModelIndex &index ) const
{
    if (index.column() == IsVariedColumn) {
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

bool SoilTypeCatalog::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginInsertRows(parent, row, row+count-1);

    for (int i=0; i < count; ++i) {
        m_soilTypes.insert(row, new SoilType(this));
        emit soilTypeAdded(m_soilTypes.at(row));
    }

    emit endInsertRows();
    return true;
}

bool SoilTypeCatalog::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i){
        emit soilTypeRemoved(m_soilTypes.at(row));
        m_soilTypes.takeAt(row)->deleteLater();
    }

    emit endRemoveRows();

    return true;
}

SoilType* SoilTypeCatalog::soilType(int row)
{
    return m_soilTypes.at(row);
}

int SoilTypeCatalog::rowOf(SoilType* st) const
{
    if (m_soilTypes.contains(st)) {
        return m_soilTypes.indexOf(st);
    } else {
        return -1;
    }
}

SoilType* SoilTypeCatalog::soilTypeOf(QVariant value)
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
            for (int j = 0; j < m_soilTypes.size(); ++j) {
                if (m_soilTypes.at(j)->name() == s) {
                    i = j;
                    break;
                }
            }
        }
    }

    if (0 <= i && i < rowCount()) {
        return m_soilTypes.at(i);
    } else {
        return 0;
    }
}

NonlinearPropertyCatalog* SoilTypeCatalog::nlCatalog()
{
    return m_nlCatalog;
}

void SoilTypeCatalog::updateUnits()
{
    emit headerDataChanged(Qt::Horizontal, UnitWeightColumn, UnitWeightColumn);
}

void SoilTypeCatalog::fromJson(const QJsonArray &json)
{
    beginResetModel();
    while (m_soilTypes.size())
        m_soilTypes.takeLast()->deleteLater();

    foreach (const QJsonValue &v, json) {
        SoilType *st = new SoilType(this);
        st->fromJson(v.toObject());
        m_soilTypes << st;
    }

    endResetModel();
}

QJsonArray SoilTypeCatalog::toJson() const
{
    QJsonArray json;
    foreach (const SoilType *st, m_soilTypes)
        json << st->toJson();
    return json;
}


QDataStream & operator<< (QDataStream & out, const SoilTypeCatalog* stc)
{
    out << (quint8)1;

    out << stc->m_soilTypes.size();

    foreach (SoilType* st, stc->m_soilTypes)
        out << st;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilTypeCatalog* stc)
{
    quint8 ver;
    in >> ver;

    stc->beginResetModel();

    int size;
    in >> size;

    while (stc->m_soilTypes.size() < size) {
        SoilType* st = new SoilType(stc);
        in >> st;
        stc->m_soilTypes << st;
    }

    stc->endResetModel();

    return in;
}
