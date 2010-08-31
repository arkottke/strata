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

#include "DepthComboBoxDelegate.h"
#include "DepthComboBox.h"


#include <QLineEdit>
#include <QDebug>

DepthComboBoxDelegate::DepthComboBoxDelegate( QObject * parent )
    : QItemDelegate(parent)
{
}

QWidget * DepthComboBoxDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & /*option*/, const QModelIndex & /*index*/ ) const
{
    return new DepthComboBox(parent);
}

void DepthComboBoxDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const
{
    DepthComboBox * comboBox = static_cast<DepthComboBox*>(editor);
    // Retrieve the map containing the list of soil names and the selected index    
    comboBox->setDepth(index.model()->data(index, Qt::EditRole).toDouble());
}

void DepthComboBoxDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
    DepthComboBox * depthBox = static_cast<DepthComboBox*>(editor);
    model->setData(index, depthBox->depth());
}
