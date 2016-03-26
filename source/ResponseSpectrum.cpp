///////////////////////////////////////////////////////////////////////////////
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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "ResponseSpectrum.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

ResponseSpectrum::ResponseSpectrum(QObject * parent)
        : MyAbstractTableModel(parent)
{
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(wasModified()));
}

bool ResponseSpectrum::modified() const
{
    return m_modified;
}

void ResponseSpectrum::setModified(bool modified)
{
    m_modified = modified;
    emit wasModified();
}

double ResponseSpectrum::damping() const
{
    return m_damping;
}

void ResponseSpectrum::setDamping(double damping)
{
    if ( m_damping != damping ) {
        emit wasModified();
    }

    m_damping = damping;   
}

const QVector<double> & ResponseSpectrum::period() const
{
    return m_period;
}

void ResponseSpectrum::setPeriod(const QVector<double> & period)
{
    beginResetModel();
    m_period = period;
    m_sa.clear();
    endResetModel();
}

const QVector<double> & ResponseSpectrum::sa() const
{
    return m_sa;
}

void ResponseSpectrum::setSa(const QVector<double> & sa)
{
    beginResetModel();
    m_sa = sa;
    endResetModel();
}

void ResponseSpectrum::scaleBy(double scale)
{
    for (int i = 0; i < m_sa.size(); ++i) {
        m_sa[i] *= scale;
    }

    emit dataChanged(index(0, SpecAccelColumn),
                     index(rowCount(), SpecAccelColumn));
    emit wasModified();
}

int ResponseSpectrum::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return qMin(m_period.size(), m_sa.size());
}

int ResponseSpectrum::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant ResponseSpectrum::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
    case Qt::Horizontal:
        switch (section) {
        case PeriodColumn:
            return tr("Period (s)");
        case SpecAccelColumn:
            return tr("Spec. Accel. (g)");
        }
    case Qt::Vertical:
        return section+1;
    default:
        return QVariant();
    }
}

QVariant ResponseSpectrum::data ( const QModelIndex &index, int role ) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
        case PeriodColumn:
            return QString::number(m_period.at(index.row()));
        case SpecAccelColumn:
            return QString::number(m_sa.at(index.row()));
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool ResponseSpectrum::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if(index.parent()!=QModelIndex())
        return false;

    if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()){
        case PeriodColumn:
            m_period[index.row()] = value.toDouble();
            break;
        case SpecAccelColumn:
            m_sa[index.row()] = value.toDouble();
            break;
        default:
            return false;
        }
    }
    else
        return false;

    dataChanged(index,index);
    setModified(true);

    return true;
}

Qt::ItemFlags ResponseSpectrum::flags ( const QModelIndex &index ) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool ResponseSpectrum::insertRows ( int row, int count, const QModelIndex &parent )
{
    emit beginInsertRows( parent, row, row+count-1 );

    for (int i=0; i < count; ++i) {
        m_period.insert(row, 0);
        m_sa.insert(row, 0);
    }

    emit endInsertRows();

    return true;
}

bool ResponseSpectrum::removeRows ( int row, int count, const QModelIndex &parent )
{
    emit beginRemoveRows( parent, row, row+count-1);

    for (int i=0; i < count; ++i) {
        m_period.remove(row);
        m_sa.remove(row);
    }

    emit endRemoveRows();
    return true;
}

void ResponseSpectrum::fromJson(const QJsonObject &json)
{
    m_modified = json["modified"].toBool();
    m_damping = json["damping"].toDouble();

    m_period.clear();
    foreach (const QJsonValue &v, json["period"].toArray())
        m_period << v.toDouble();

    m_sa.clear();
    foreach (const QJsonValue &v, json["sa"].toArray())
        m_sa << v.toDouble();
}

QJsonObject ResponseSpectrum::toJson() const
{
    QJsonObject json;
    json["modified"] = m_modified;
    json["damping"] = m_damping;

    QJsonArray period;
    foreach (const double &d, m_period)
        period << QJsonValue(d);
    json["period"] = period;

    QJsonArray sa;
    foreach (const double &d, m_sa)
        sa << QJsonValue(d);
    json["sa"] = sa;

    return json;
}


QDataStream & operator<< (QDataStream & out, const ResponseSpectrum* rs)
{
    out << (quint8)1;

    out << rs->m_modified
            << rs->m_damping
            << rs->m_period
            << rs->m_sa;

    return out;
}

QDataStream & operator>> (QDataStream & in, ResponseSpectrum* rs)
{
    quint8 ver;
    in >> ver;

    in >> rs->m_modified
            >> rs->m_damping
            >> rs->m_period
            >> rs->m_sa;

    return in;
}
