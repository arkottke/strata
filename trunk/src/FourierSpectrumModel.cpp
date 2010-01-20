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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "FourierSpectrumModel.h"
#include <QDebug>

FourierSpectrumModel::FourierSpectrumModel(RvtMotion * rvtMotion, bool readOnly, QObject *parent)
        : MyAbstractTableModel(readOnly, parent), m_rvtMotion(rvtMotion)
{
    connect( rvtMotion, SIGNAL(fourierSpectrumChanged()), this, SLOT(resetModel()));
    connect( this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), rvtMotion, SLOT(setModified()));
}

int FourierSpectrumModel::rowCount ( const QModelIndex& /* index */ ) const
{
    return m_rvtMotion->freq().size();
}

int FourierSpectrumModel::columnCount ( const QModelIndex& /* parent */ ) const
{
    return 2;
}

QVariant FourierSpectrumModel::data ( const QModelIndex &index, int role ) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
    {
        switch (index.column())
        {
        case 0:
            // Freq
            return QString::number(m_rvtMotion->freq().at(index.row()));
        case 1:
            // Spectral Acceleration
            return QString::number(m_rvtMotion->fas().at(index.row()));
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool FourierSpectrumModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if(index.parent()!=QModelIndex())
        return false;

    if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
    {
        switch (index.column())
        {
        case 0:
            // Freq
            m_rvtMotion->freq()[index.row()] = value.toDouble();
            break;
        case 1:
            // Spectral Acceleration
            m_rvtMotion->fas()[index.row()] = value.toDouble();
            break;
                        default:
            return false;
        }
    }
    else
        return false;
    
    dataChanged(index, index);
    return true;
}

QVariant FourierSpectrumModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation )
    {
                case Qt::Horizontal:
        switch (section)
        {
                                case 0:
            // Freq
            return QVariant(tr("Frequency (Hz)"));
                                case 1:
            // Spectral Acceleration
            return QVariant(tr("Fourier Amplitude (g-s)"));
        }
                case Qt::Vertical:
        return QVariant(section+1);
                default:
        return QVariant();
    }
}

Qt::ItemFlags FourierSpectrumModel::flags ( const QModelIndex &index ) const
{
    if (m_readOnly) {
        return QAbstractTableModel::flags(index);
    }

    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool FourierSpectrumModel::insertRows ( int row, int count, const QModelIndex &parent )
{
    emit beginInsertRows( parent, row, row+count-1 );

    for (int i=0; i < count; ++i) {
        m_rvtMotion->freq().insert(row, 0);
        m_rvtMotion->fas().insert(row, 0);
    }

    emit endInsertRows();

    return true;
}

bool FourierSpectrumModel::removeRows ( int row, int count, const QModelIndex &parent )
{
    emit beginRemoveRows( parent, row, row+count-1);

    for (int i=0; i < count; ++i) {
        m_rvtMotion->freq().remove(row);
        m_rvtMotion->fas().remove(row);
    }

    emit endRemoveRows();
    return true;
}
