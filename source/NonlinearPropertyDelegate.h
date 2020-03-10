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

#ifndef NONLINEAR_PROPERTY_DELEGATE_H
#define NONLINEAR_PROPERTY_DELEGATE_H

#include <QItemDelegate>

#include <QPointer>

class AbstractNonlinearPropertyFactory;

class NonlinearPropertyDelegate : public QItemDelegate
{
Q_OBJECT
public:
    explicit NonlinearPropertyDelegate(QObject *parent = nullptr);

    void setModel(AbstractNonlinearPropertyFactory* factory);

    auto createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const -> QWidget *;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

protected:
    QPointer<AbstractNonlinearPropertyFactory> _factory;
};

#endif // NONLINEAR_PROPERTY_DELEGATE_H
