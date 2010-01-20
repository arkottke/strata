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

#include "ConfiningStressTableModel.h"
#include "Units.h"

#include <QBrush>
#include <QColor>
#include <QDebug>

ConfiningStressTableModel::ConfiningStressTableModel( QObject * parent )
	: MyAbstractTableModel(false, parent), m_waterTableDepth(0.0)
{
    connect( Units::instance(), SIGNAL(systemChanged()), SLOT(computeStress()));
    connect( Units::instance(), SIGNAL(systemChanged()), SLOT(updateHeader()));
}

int ConfiningStressTableModel::rowCount ( const QModelIndex& /* index */ ) const
{
	return m_layers.size();
}

int ConfiningStressTableModel::columnCount ( const QModelIndex& /* parent */ ) const
{
	return 4;
}

QVariant ConfiningStressTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	if( role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::UserRole )
		return QVariant();

	switch( orientation )
	{
		case Qt::Horizontal:
			switch (section)
			{
                case 0:
					// Thickness
					return QString(tr("Thickness (%1)")).arg(Units::instance()->length());
                case 1:
                    // Unit Weight
					return QString(tr("Unit Wt. (%1)")).arg(Units::instance()->untWt());
                case 2:
                    // At Rest Coefficient
					return tr("At Rest Coeff., k0");
                case 3:
                    // Mean Effective stress
					return tr("Mean Eff. Stress (atm)");
			}
		case Qt::Vertical:
			return section+1;
		default:
			return QVariant();
	}
}

QVariant ConfiningStressTableModel::data ( const QModelIndex &index, int role ) const
{
	if (index.parent()!=QModelIndex())
		return QVariant();

    // Color the background light gray for cells that are not editable
    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
            return QBrush(QColor(200,200,200));

	if(  role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
    {
        switch (index.column())
        {
            case 0:
                // Thickness
                return QString::number(m_layers.at(index.row())->thick);
            case 1:
                // Unit Weight
                return QString::number(m_layers.at(index.row())->untWt);
            case 2:
                // At Rest Coefficient
                return QString::number(m_layers.at(index.row())->atRestCoeff);
            case 3:
                // Mean Effective stress
                return QString::number(m_layers.at(index.row())->mEffStress);
        }
    }
		
    return QVariant();
}

bool ConfiningStressTableModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if(index.parent()!=QModelIndex())
		return false;

	if(role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole)
	{
		switch (index.column())
		{
            case 0:
                // Thickness
                m_layers[index.row()]->thick = value.toDouble();
                computeStress(index.row());
                break;
            case 1:
                // Unit Weight
                m_layers[index.row()]->untWt = value.toDouble();
                computeStress(index.row());
                break;
            case 2:
                // At Rest Coefficient
                m_layers[index.row()]->atRestCoeff = value.toDouble();
                computeStress(index.row());
                break;
            case 3:
                return false;
        }
    } 
        
    dataChanged(index, index);
    return true;
}

Qt::ItemFlags ConfiningStressTableModel::flags ( const QModelIndex &index ) const
{
    if (index.column() == 3 ) {
        // Mean effective stress column -- not editable
        return QAbstractTableModel::flags(index);
    } else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

bool ConfiningStressTableModel::insertRows ( int row, int count, const QModelIndex &parent )
{
	emit beginInsertRows( parent, row, row+count-1 );

	for (int i=0; i < count; ++i)  {
		m_layers.insert( row, new Layer );

        m_layers[row]->thick = 0.;
        m_layers[row]->untWt = 0.;
        m_layers[row]->atRestCoeff = 0.5;
    }
    
    emit endInsertRows();
    
	return true;
}

bool ConfiningStressTableModel::removeRows ( int row, int count, const QModelIndex &parent )
{
	emit beginRemoveRows( parent, row, row+count-1);
		
	for (int i=0; i < count; ++i) 
		delete m_layers.takeAt(row);
    
    emit endRemoveRows();
	
    return true;
}
        
double ConfiningStressTableModel::waterTableDepth()
{
    return m_waterTableDepth;
}

void ConfiningStressTableModel::setWaterTableDepth(double depth)
{
    m_waterTableDepth = depth;
    computeStress();
}
        
void ConfiningStressTableModel::computeStress(int layer)
{
    double depth = 0;
    double vStress = 0;
    // Compute the depth and initial stress up to the first layer
    for (int i = 0; i < layer; ++i) {
        depth += m_layers.at(i)->thick;
        vStress += m_layers.at(i)->thick * m_layers.at(i)->untWt;
    }

    // Compute the stress at each of the layers
    for (int i = layer; i < m_layers.size(); ++i) {
        double halfThick = m_layers.at(i)->thick;
        // Add the first half
        depth += halfThick;
        vStress += halfThick * m_layers.at(i)->untWt;

        double porePressure = ( depth < m_waterTableDepth ) ? 0 : (depth - m_waterTableDepth) * Units::instance()->waterUntWt();
        
        double vEffStress = vStress - porePressure;
        
        double mEffStress = vEffStress * ( 1. + 2. * m_layers.at(i)->atRestCoeff ) / 3.;

        // Convert to atm
        m_layers[i]->mEffStress = mEffStress * Units::instance()->toAtm();
        
        // Add the second half of the layer
        depth += halfThick;
        vStress += halfThick * m_layers.at(i)->untWt;
    }

    emit dataChanged( index( layer, 3 ), index( layer, m_layers.size() ));
}

void ConfiningStressTableModel::updateHeader()
{
    emit headerDataChanged( Qt::Horizontal, 0, 2);
}
