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

#ifndef TABLE_GROUP_BOX_H_
#define TABLE_GROUP_BOX_H_

#include <QAbstractItemDelegate>
#include <QAbstractTableModel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QItemDelegate>
#include <QItemSelection>
#include <QModelIndex>
#include <QPushButton>
#include <QString>

class MyTableView;
class MyAbstractTableModel;

class TableGroupBox : public QGroupBox {
  Q_OBJECT

public:
  explicit TableGroupBox(const QString &title, QWidget *parent = nullptr);

  void setModel(QAbstractTableModel *model);
  void setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate);
  void setColumnHidden(int column, bool hide);

  auto lastRowFixed() const -> bool;
  void setLastRowFixed(bool lastRowFixed);

  void addButton(QPushButton *pushButton);

  auto table() -> MyTableView *;

public slots:
  void setReadOnly(bool readOnly);

private slots:
  void addRow();
  void insertRow();
  void removeRow();

  void
  cellSelected(); // const QModelIndex & current, const QModelIndex & previous);

signals:
  void rowRemoved();
  void dataChanged();

  void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
  //! If table is read only
  bool _readOnly;

  //! If the last row of the table should not be removed
  bool _lastRowFixed;

  QHBoxLayout *_buttonRow;

  QPushButton *_addButton;
  QPushButton *_insertButton;
  QPushButton *_removeButton;

  QList<QPushButton *> _addedButtons;

  MyTableView *_table;
};
#endif
