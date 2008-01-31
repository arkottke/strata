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

#include "CheckBoxDelegate.h"
#include <QCheckBox>
#include <QApplication>

CheckBoxDelegate::CheckBoxDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

QWidget* CheckBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /*option*/,
        const QModelIndex & /*index*/ ) const
{
    QCheckBox *editor = new QCheckBox(parent);
    editor->installEventFilter(const_cast<CheckBoxDelegate*>(this));
    return editor;
}

void CheckBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QCheckBox * checkBox = static_cast<QCheckBox*>(editor);
    // Retrieve the data from the index
    checkBox->setChecked(index.model()->data(index, Qt::EditRole).toBool());
    
    return;
}

void CheckBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QCheckBox * checkBox = static_cast<QCheckBox*>(editor);
    model->setData( index, checkBox->isChecked());
    
    return;
}

void CheckBoxDelegate::paint( QPainter * painter, const QStyleOptionViewItem &
        option, const QModelIndex & index ) const
{
     if (option.state & QStyle::State_Selected)
         painter->fillRect(option.rect, option.palette.highlight());

     if (index.model()->data(index, Qt::DisplayRole).toBool())
         drawCheck( painter, option, option.rect, Qt::Checked);
     else
         drawCheck( painter, option, option.rect, Qt::Unchecked);
}
