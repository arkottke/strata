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
// Copyright 2010-2018 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "OnlyIncreasingDelegate.h"

#include <QDoubleValidator>
#include <QLineEdit>

#include <cfloat>

OnlyIncreasingDelegate::OnlyIncreasingDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

auto OnlyIncreasingDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const -> QWidget*
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    auto* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (lineEdit) {
        double min = 0;
        double max = 20;

        QModelIndex prev = index.sibling(index.row() - 1, index.column());
        if (prev.isValid()) {
            min = prev.data(Qt::UserRole).toDouble() + DBL_EPSILON;
        }

        QModelIndex next = index.sibling(index.row() + 1, index.column());
        if (next.isValid()) {
            max = next.data(Qt::UserRole).toDouble() - DBL_EPSILON;
        }

        auto* validator = new QDoubleValidator(min, max, 5);
        lineEdit->setValidator(validator);
    }

    return editor;
}
