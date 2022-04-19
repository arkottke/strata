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

#include <QAction>
#include <QObject>

//! Class used to facilitate universal edit actions for multiple dialogs.
/*!
 * More details?
 */

class EditActions : public QObject {
  Q_OBJECT

public:
  static auto instance() -> EditActions *;

  auto copyAction() -> QAction *;
  auto pasteAction() -> QAction *;
  auto cutAction() -> QAction *;
  auto clearAction() -> QAction *;

protected slots:
  void paste();
  void copy();
  void cut();
  void clear();

protected:
  explicit EditActions(QObject *parent = nullptr);

  static EditActions *_instance;

  QAction *_copyAction;
  QAction *_pasteAction;
  QAction *_cutAction;
  QAction *_clearAction;
};
