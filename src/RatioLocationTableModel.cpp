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

#include "RatioLocationTableModel.h"
#include "Units.h"

#include <QBrush>
#include <QColor>
#include <QDebug>

RatioLocationTableModel::RatioLocationTableModel( SiteResponseOutput * model, QObject *parent )
        : MyAbstractTableModel(false, parent), m_model(model)
{
    connect( m_model, SIGNAL(ratioLocationsChanged()), this, SLOT(resetModel()));
}

int RatioLocationTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
    return m_model->ratioLocations().size();
}

int RatioLocationTableModel::columnCount( const QModelIndex& /* parent */ ) const
{
    return 7;
}

QVariant RatioLocationTableModel::data( const QModelIndex &index, int role ) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    // Color the background to red and green for the check state
    if (role==Qt::BackgroundRole) {
        // Color the background light gray for cells that are not editable
        if (!( flags(index) & Qt::ItemIsEditable || flags(index) & Qt::ItemIsUserCheckable )) {
            return QVariant(QBrush(QColor(200,200,200)));
        }

        // Check boxes change color of the cells
        if ( flags(index) & Qt::ItemIsUserCheckable) {
            if ( data(index, Qt::CheckStateRole).toInt() == Qt::Unchecked) 
                return QBrush(QColor(200,200,200));
            else 
                return QBrush(QColor(50,200,50));
        }
    }


    if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
    {
        switch (index.column())
        {
        case 0:
            {
                // To Depth
                double depth = m_model->ratioLocations().at(index.row())->toDepth();
                if ( role == Qt::DisplayRole && depth < 0)
                    return tr("Bedrock");
                else
                    return QString("%1 %2").arg(depth).arg(Units::instance()->length());
            }
        case 1:
            // To Layer type
            if ( role == Qt::DisplayRole ) {
                switch (m_model->ratioLocations().at(index.row())->toType())
                {
                        case Motion::Outcrop:
                    return tr("Outcrop");
                        case Motion::Within:
                    return tr("Within");
                }
            } else {
                QMap<QString, QVariant> map;
                // Values to choose from
                map.insert("list", Motion::typeList());
                // Selected value
                map.insert("index", m_model->ratioLocations().at(index.row())->toType());
                return map;
            }
        case 2:
            {
                // From Depth
                double depth = m_model->ratioLocations().at(index.row())->fromDepth();
                if ( role == Qt::DisplayRole && depth < 0)
                    return tr("Bedrock");
                else
                    return QString("%1 %2").arg(depth).arg(Units::instance()->length());
            }
        case 3:
            // From Layer type
            if ( role == Qt::DisplayRole ) {
                switch (m_model->ratioLocations().at(index.row())->fromType())
                {
                        case Motion::Outcrop:
                    return tr("Outcrop");
                        case Motion::Within:
                    return tr("Within");
                }
            } else {
                QMap<QString, QVariant> map;
                // Values to choose from
                map.insert("list", Motion::typeList());
                // Selected value
                map.insert("index", m_model->ratioLocations().at(index.row())->fromType());
                return map;
            }
        case 4:
            // Acceleration transfer function
        case 5:
            // Strain transfer function
        case 6:
            // Acceleration response spectrum ratio
        default:
            return QVariant();
        }
    } else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
        case 0:
            // To Depth
        case 1:
            // To Layer type
        case 2:
            // From Depth
        case 3:
            // From Layer type
            return QVariant();
        case 4:
            // Acceleration transfer function
            if (m_model->ratioLocations()[index.row()]->transFunc()->enabled())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        case 5:
            // Strain transfer function
            if (m_model->ratioLocations()[index.row()]->strainTransFunc()->enabled())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        case 6:
            // Acceleration response spectrum ratio
            if (m_model->ratioLocations()[index.row()]->respRatio()->enabled())
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }

    return QVariant();
}

bool RatioLocationTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if(index.parent()!=QModelIndex())
        return false;

    if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
    {
        switch (index.column())
        {
        case 0:
            // To Depth
            m_model->ratioLocations()[index.row()]->setToDepth(value.toDouble());
            break;
        case 1:
            // To Layer type
            m_model->ratioLocations()[index.row()]->setToType(
                    (Motion::Type)value.toInt());
            break;
        case 2:
            // To Depth
            m_model->ratioLocations()[index.row()]->setFromDepth(value.toDouble());
            break;
        case 3:
            // To Layer type
            m_model->ratioLocations()[index.row()]->setFromType(
                    (Motion::Type)value.toInt());
            break;
        case 4:
            // Acceleration transfer function
        case 5:
            // Strain transfer function
        case 6:
            // Acceleration response spectrum ratio
        default:
            return false;
        }
    } else if (role==Qt::CheckStateRole) {
        switch (index.column())
        {
        case 0:
            // To Depth
        case 1:
            // To Layer type
        case 2:
            // From Depth
        case 3:
            // From Layer type
            return false;
        case 4:
            // Acceleration transfer function
            m_model->ratioLocations()[index.row()]->transFunc()->setEnabled(value.toBool());
            break;
        case 5:
            // Strain transfer function
            m_model->ratioLocations()[index.row()]->strainTransFunc()->setEnabled(value.toBool());
            break;
        case 6:
            // Acceleration response spectrum ratio
            m_model->ratioLocations()[index.row()]->respRatio()->setEnabled(value.toBool());
            break;
        }
    } else
        return false;

    dataChanged(index, index);
    return true;
}

QVariant RatioLocationTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
        return QVariant();

    switch( orientation ) {
        case Qt::Horizontal:
            switch (section) {
                   case 0:
            // To Depth
            return tr("Location 1");
                case 1:
            // To Layer type
            return tr("Type 1");
                                case 2:
            // From Depth
            return tr("Location 2");
                case 3:
            // From Layer type
            return tr("Type 2");
                case 4:
            // Acceleration transfer function
            return tr("FAS_1 (accel) / FAS_2 (accel)");
                case 5:
            // Strain transfer function
            return tr("FAS_1 (strain) / FAS_2 (accel)");
                    case 6:
            // Acceleration response spectrum ratio
            return tr("Sa_1 / Sa_2");
        }
                case Qt::Vertical:
        return section+1;
                default:
        return QVariant();
    }
}

Qt::ItemFlags RatioLocationTableModel::flags( const QModelIndex &index ) const
{
    if (m_readOnly) {
        return QAbstractTableModel::flags(index);
    }

    switch (index.column())
    {
    case 0:
        // To Depth
    case 1:
        // To Layer type
    case 2:
        // From Depth
    case 3:
        // From Layer type
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    case 4:
        // Acceleration transfer function
    case 5:
        // Strain transfer function
    case 6:
        // Acceleration response spectrum ratio
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    }

    return QAbstractTableModel::flags(index);
}

bool RatioLocationTableModel::insertRows( int row, int count, const QModelIndex &parent )
{
    emit beginInsertRows( parent, row, row+count-1 );

    for (int i=0; i < count; ++i)
        m_model->ratioLocations().insert( row, new RatioLocation );

    emit endInsertRows();

    return true;
}

bool RatioLocationTableModel::removeRows( int row, int count, const QModelIndex &parent )
{
    emit beginRemoveRows( parent, row, row+count-1);

    for (int i = 0; i < count; ++i)
        delete m_model->ratioLocations().takeAt(row);

    emit endRemoveRows();

    return true;
}
