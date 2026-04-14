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

#include "EditActions.h"

#include <QApplication>
#include <QDebug>
#include <QMetaMethod>
#include <QWidget>

EditActions *EditActions::_instance = nullptr;

EditActions::EditActions(QObject *parent) : QObject(parent) {
  // Paste Action
  _pasteAction =
      new QAction(QIcon(":/images/edit-paste.svg"), tr("&Paste"), this);
  _pasteAction->setShortcut(QKeySequence::Paste);
  connect(_pasteAction, &QAction::triggered, this, &EditActions::paste);

  // Copy Action
  _copyAction = new QAction(QIcon(":/images/edit-copy.svg"), tr("&Copy"), this);
  _copyAction->setShortcut(QKeySequence::Copy);
  connect(_copyAction, &QAction::triggered, this, &EditActions::copy);

  // Cut Action
  _cutAction = new QAction(QIcon(":/images/edit-cut.svg"), tr("Cu&t"), this);
  _cutAction->setShortcut(QKeySequence::Cut);
  connect(_cutAction, &QAction::triggered, this, &EditActions::cut);

  // Clear Action
  _clearAction =
      new QAction(QIcon(":/images/edit-clear.svg"), tr("Clear"), this);
  connect(_clearAction, &QAction::triggered, this, &EditActions::clear);
}

auto EditActions::instance() -> EditActions * {
  if (_instance == nullptr) {
    _instance = new EditActions;
  }

  return _instance;
}

auto EditActions::cutAction() -> QAction * { return _cutAction; }

auto EditActions::copyAction() -> QAction * { return _copyAction; }

auto EditActions::pasteAction() -> QAction * { return _pasteAction; }

auto EditActions::clearAction() -> QAction * { return _clearAction; }

void EditActions::cut() {
  copy();
  clear();
}

static void invokeSlot(QWidget *w, const char *slot) {
  if (!w)
    return;
  int idx = w->metaObject()->indexOfSlot(slot);
  if (idx >= 0)
    w->metaObject()->method(idx).invoke(w);
}

void EditActions::copy() { invokeSlot(QApplication::focusWidget(), "copy()"); }

void EditActions::paste() {
  invokeSlot(QApplication::focusWidget(), "paste()");
}

void EditActions::clear() {
  invokeSlot(QApplication::focusWidget(), "clear()");
}
