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
#include "Serializer.h"

ResponseSpectrum::ResponseSpectrum(bool readOnly, QObject * parent)
        : MyAbstractTableModel(readOnly, parent)
{
    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(wasModified()));
}

void ResponseSpectrum::reset()
{
    m_modified = false; 
    m_damping = 5.0;

    m_period.clear();
    m_sa.clear();
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
    m_period = period;
    m_sa.clear();
    emit wasModified();
}

const QVector<double> & ResponseSpectrum::sa() const
{
    return m_sa;
}

void ResponseSpectrum::setSa(const QVector<double> & sa)
{
    m_sa = sa;
    emit wasModified();
}

QMap<QString, QVariant> ResponseSpectrum::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("modified", m_modified);
    map.insert("damping", m_damping);
    map.insert("period", Serializer::toVariantList(m_period));
    map.insert("sa", Serializer::toVariantList(m_sa));

    return map;
}

void ResponseSpectrum::fromMap( const QMap<QString, QVariant> & map)
{
    m_modified = map.value("modified").toBool();
    m_damping = map.value("damping").toDouble();
    m_period = Serializer::fromVariantList(map.value("period").toList()).toVector();
    m_sa = Serializer::fromVariantList(map.value("sa").toList()).toVector();

    emit wasModified();
}

int ResponseSpectrum::rowCount ( const QModelIndex& /* index */ ) const
{
    return m_period.size();
}

int ResponseSpectrum::columnCount ( const QModelIndex& /* parent */ ) const
{
    return 2;
}

QVariant ResponseSpectrum::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
    case Qt::Horizontal:
        switch (section) {
        case 0:
            // Period
            return QVariant(tr("Period (s)"));
        case 1:
            // Spectral Acceleration
            return QVariant(tr("Spec. Accel. (g)"));
        }
    case Qt::Vertical:
        return QVariant(section+1);
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
        case 0:
            // Period
            return QVariant(QString::number(m_period.at(index.row())));
        case 1:
            // Spectral Acceleration
            return QVariant(QString::number(m_sa.at(index.row())));
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
        case 0:
            // Period
            m_period[index.row()] = value.toDouble();
            break;
        case 1:
            // Spectral Acceleration
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
    if (m_readOnly) {
        return QAbstractTableModel::flags(index);
    }

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
