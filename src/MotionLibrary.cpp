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

#include "MotionLibrary.h"

#include "CompatibleRvtMotion.h"
#include "RvtMotion.h"
#include "SourceTheoryRvtMotion.h"
#include "TimeSeriesMotion.h"
#include "Units.h"

#include <QBrush>
#include <QColor>
#include <QDebug>

MotionLibrary::MotionLibrary(QObject *parent)
    : MyAbstractTableModel(parent)
{
    m_approach = TimeSeries;
    m_saveData = true;

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(updateUnits()));
}

QStringList MotionLibrary::approachList()
{
    return QStringList()
            << tr("Time Series")
            << tr("Random Vibration Theory");
}

QString MotionLibrary::toHtml() const
{
    QString html = "<table border=\"1\"><tr>";

    // Header
    for (int i = 0; i < columnCount(); ++i)
        html += "<th>" + headerData(i, Qt::Horizontal).toString() + "</th>";

    html += "</tr>";

    // Data
    for (int r = 0; r < rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < columnCount(); ++c)
            html += "<td>" + data(index(r, c)).toString() + "</td>";

        html += "</tr>";
    }
    html += "</table>";

//    // Details
//    html += "<ol>";
//    foreach (Motion *motion, m_motions)
//        html += "<li>" + motion->toHtml() + "</li>";
//
//    html += "</ol>";

    return html;
}

void MotionLibrary::setSaveData(bool b)
{
    if (m_saveData != b) {
        m_saveData = b;
        emit saveDataChanged(m_saveData);

        foreach (AbstractMotion* m, m_motions) {
            if (TimeSeriesMotion *tsm = qobject_cast<TimeSeriesMotion *>(m)) {
                tsm->setSaveData(b);
            }
        }
    }
}

bool MotionLibrary::saveData() const
{
    return m_saveData;
}

int MotionLibrary::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return m_motions.size();
}

int MotionLibrary::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return 6;
}

QVariant MotionLibrary::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
        case NameColumn:
            return m_motions.at(index.row())->name();
        case DescriptionColumn:
            return m_motions.at(index.row())->description();
        case TypeColumn:
            if (role == Qt::DisplayRole) {
                // Return a simple label
                return AbstractMotion::typeList().at((int)m_motions.at(index.row())->type());
            } else {
                return (int)m_motions.at(index.row())->type();
            }
        case PgaColumn:
            return QString::number(m_motions.at(index.row())->pga(), 'f', 2);
        case PgvColumn:
            return QString::number(m_motions.at(index.row())->pgv(), 'f', 2);
        case ScaleColumn:
            {
                TimeSeriesMotion *motion = dynamic_cast<TimeSeriesMotion*>(m_motions.at(index.row()));

                if (motion) {
                    return QString::number(motion->scale(), 'f', 2);
                }
            }
        default:
            return QVariant();
        }
    } else if (index.column() == NameColumn && role == Qt::CheckStateRole ) {
        if ( m_motions.at(index.row())->enabled() )
            return QVariant( Qt::Checked );
        else
            return QVariant( Qt::Unchecked );
    }

    return MyAbstractTableModel::data(index, role);
}

QVariant MotionLibrary::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
    case Qt::Horizontal:
        switch (section) {
        case NameColumn:
            return tr("Name");
        case DescriptionColumn:
            return tr("Description");
        case TypeColumn:
            return tr("Type");
        case PgaColumn:
            return tr("PGA (g)");
        case PgvColumn:
            return QString(tr("PGV (%1)")).arg(Units::instance()->velTs());
        case ScaleColumn:
            return tr("Scale Factor");
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

Qt::ItemFlags MotionLibrary::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    switch (index.column()) {
    case NameColumn:
        flags |= Qt::ItemIsUserCheckable;
        break;
    case DescriptionColumn:
    case TypeColumn:
        flags |= Qt::ItemIsEditable;
        break;
    case ScaleColumn:
        if (m_approach == TimeSeries)
            flags |= Qt::ItemIsEditable;
        break;
    case PgvColumn:
    case PgaColumn:
        break;
    }

    return flags;
}

bool MotionLibrary::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent()!=QModelIndex() || m_readOnly)
        return false;

    if (role==Qt::EditRole) {
        switch (index.column()) {
        case NameColumn:
        case DescriptionColumn:
        case PgaColumn:
            return false;
        case TypeColumn:
            {
                bool ok;
                AbstractMotion::Type type = AbstractMotion::variantToType(value, &ok);

                if (ok)
                {
                    m_motions[index.row()]->setType(type);
                }
                else
                    return false;
                break;
            }
        case ScaleColumn:
            {
                TimeSeriesMotion * tsMotion = qobject_cast<TimeSeriesMotion*>(m_motions[index.row()]);

                bool ok;
                const double d = value.toDouble(&ok);

                if (tsMotion && ok)
                {
                    tsMotion->setScale(d);
                }
                else
                    return false;

                break;
            }
        }
        // Signal that the data has changed
        dataChanged(index, index);
    } else if (role==Qt::CheckStateRole && index.column() == NameColumn) {
        m_motions[index.row()]->setEnabled(value.toBool());
        // Change entire row information
        dataChanged(index.sibling(index.row(), 0), index.sibling(index.row(), columnCount()));
    }

    return false;
}

bool MotionLibrary::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;
    emit beginRemoveRows(parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
    {
        m_motions.takeAt(row)->deleteLater();
    }

    emit endRemoveRows();
    return true;
}

int MotionLibrary::motionCount() const
{
    int count = 0;

    foreach (const AbstractMotion* m, m_motions)
        if (m->enabled())
            ++count;

    return count;
}

void MotionLibrary::addMotion(AbstractMotion *motion)
{
    int row = rowCount();
    motion->setParent(this);

    if (TimeSeriesMotion* tsm = qobject_cast<TimeSeriesMotion*>(motion))
        tsm->setSaveData(m_saveData);

    emit beginInsertRows(QModelIndex(), row, row);
    m_motions.insert(row, motion);
    emit wasModified();
    emit endInsertRows();
}

AbstractMotion* MotionLibrary::motionAt(int row)
{
    return m_motions.at(row);
}

void MotionLibrary::updateRow(int row)
{
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

MotionLibrary::Approach MotionLibrary::approach() const
{
    return m_approach;
}

void MotionLibrary::setApproach(Approach approach)
{
    if (m_approach != approach) {
        m_approach = approach;
        emit approachChanged(m_approach);
        // Clear the data
        removeRows(0, rowCount());
    }
}

void MotionLibrary::setApproach(int approach)
{
    setApproach((Approach)approach);
}

void MotionLibrary::setReadOnly(bool readOnly)
{
    MyAbstractTableModel::setReadOnly(readOnly);

    for (int i = 0; i < m_motions.size(); ++i) {
        m_motions[i]->setReadOnly(readOnly);
    }
}

void MotionLibrary::updateUnits()
{
    emit headerDataChanged(Qt::Horizontal, PgvColumn, PgvColumn);
}

void MotionLibrary::ptRead(const ptree &pt)
{
    int approach = pt.get<int>("approach");
    setApproach(approach);
    bool saveData = pt.get<bool>("saveData");
    setSaveData(saveData);

    beginResetModel();

    ptree motions = pt.get_child("motions");
    foreach(const ptree::value_type &v, motions)
    {
        AbstractMotion * m;
        if (v.first.find("TimeSeriesMotion")!=std::string::npos) {
            m = new TimeSeriesMotion(this);
            ((TimeSeriesMotion *) m)->ptRead(v.second);
            ((TimeSeriesMotion *) m)->setSaveData(m_saveData);
            if (((TimeSeriesMotion *) m)->accel().size()) {
                m_motions.append(m);
            } else {
                deleteLater();
            }
        } else if (v.first.find("RvtMotion")!=std::string::npos) {
            m = new RvtMotion(this);
            ((RvtMotion *) m)->ptRead(v.second);
            m_motions.append(m);
        } else if (v.first.find("CompatibleRvtMotion")!=std::string::npos) {
            m = new CompatibleRvtMotion(this);
            ((CompatibleRvtMotion *) m)->ptRead(v.second);
            m_motions.append(m);
        } else if (v.first.find("SourceTheoryRvtMotion")!=std::string::npos) {
            m = new SourceTheoryRvtMotion(this);
            ((SourceTheoryRvtMotion *) m)->ptRead(v.second);
            m_motions.append(m);
        }
    }

    endResetModel();
}

void MotionLibrary::ptWrite(ptree &pt) const
{
    pt.put("approach", (int) m_approach);
    pt.put("saveData", m_saveData);

    // generate unqiue key names
    int counter = 0;

    ptree motions;
    foreach(AbstractMotion *m, m_motions)
    {
        ptree motion;
        std::stringstream fmt;
        const QString & className = m->metaObject()->className();
        if (className == "TimeSeriesMotion") {
            qobject_cast<const TimeSeriesMotion*>(m)->ptWrite(motion);
            fmt << "TimeSeriesMotion-" << counter++;
            motions.add_child(fmt.str(), motion);
        } else if (className == "RvtMotion") {
            qobject_cast<const RvtMotion*>(m)->ptWrite(motion);
            fmt << "RvtMotion-" << counter++;
            motions.add_child(fmt.str(), motion);
        } else if (className == "CompatibleRvtMotion") {
            qobject_cast<const CompatibleRvtMotion*>(m)->ptWrite(motion);
            fmt << "CompatibleRvtMotion-" << counter++;
            motions.add_child(fmt.str(), motion);
        } else if (className == "SourceTheoryRvtMotion") {
            qobject_cast<const SourceTheoryRvtMotion*>(m)->ptWrite(motion);
            fmt << "SourceTheoryRvtMotion-" << counter++;
            motions.add_child(fmt.str(), motion);
        }
    }
    pt.add_child("motions", motions);
}

QDataStream & operator<< (QDataStream & out, const MotionLibrary* ml)
{
    out << (quint8)1;

    out << (int)ml->m_approach << ml->m_saveData << ml->m_motions.size();

    foreach (const AbstractMotion * m, ml->m_motions) {
        const QString & className = m->metaObject()->className();
        out << className;

        if (className == "TimeSeriesMotion") {
            out << qobject_cast<const TimeSeriesMotion*>(m);
        } else if (className == "RvtMotion") {
            out << qobject_cast<const RvtMotion*>(m);
        } else if (className == "CompatibleRvtMotion") {
            out << qobject_cast<const CompatibleRvtMotion*>(m);
        } else if (className == "SourceTheoryRvtMotion") {
            out << qobject_cast<const SourceTheoryRvtMotion*>(m);
        }
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, MotionLibrary* ml)
{
    quint8 ver;
    in >> ver;

    int approach;
    bool saveData;
    int size;

    in >> approach >> saveData >> size;

    ml->setApproach(approach);
    ml->setSaveData(saveData);

    ml->beginResetModel();
    QString className;

    while (ml->m_motions.size() < size) {
        in >> className;

        if (className == "TimeSeriesMotion") {
            TimeSeriesMotion *m = new TimeSeriesMotion(ml);
            m->setSaveData(ml->m_saveData);
            in >> m;

            if (m->accel().size()) {
                ml->m_motions << m;
            } else {
                m->deleteLater();
                // Need to reduce the size of motions to load so we know when to stop
                --size;
            }
        } else if (className == "RvtMotion") {
            RvtMotion *m = new RvtMotion(ml);
            in >> m;
            ml->m_motions << m;
        } else if (className == "CompatibleRvtMotion") {
            CompatibleRvtMotion *m = new CompatibleRvtMotion(ml);
            in >> m;
            ml->m_motions << m;
        } else if (className == "SourceTheoryRvtMotion") {
            SourceTheoryRvtMotion *m = new SourceTheoryRvtMotion(ml);
            in >> m;
            ml->m_motions << m;
        }
    }

    ml->endResetModel();

    return in;
}
