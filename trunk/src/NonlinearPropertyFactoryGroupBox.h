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

#ifndef NONLINEAR_PROPERTY_FACTORY_GROUP_BOX_H
#define NONLINEAR_PROPERTY_FACTORY_GROUP_BOX_H

#include "AbstractNonlinearPropertyFactory.h"

#include <QGroupBox>
#include <QPushButton>
#include <QListView>

class NonlinearPropertyFactoryGroupBox : public QGroupBox
{
    Q_OBJECT

public:
    explicit NonlinearPropertyFactoryGroupBox(AbstractNonlinearPropertyFactory *model,
                                     const QString & title, QWidget *parent = 0);

    void selectRow(int row);
signals:
    void nlPropertyChanged(NonlinearProperty *nlProperty, bool readOnly);

public slots:
    void clearSelection();

protected slots:
    void addRow();
    void removeRow();

    void updateCurrent(const QModelIndex &current, const QModelIndex &previous);
    void modelsInserted(const QModelIndex & parent, int start, int end);

protected:
    AbstractNonlinearPropertyFactory *m_model;
    QListView *m_view;
    QPushButton *m_removeButton;
};

#endif // NONLINEAR_PROPERTY_FACTORY_GROUP_BOX_H
