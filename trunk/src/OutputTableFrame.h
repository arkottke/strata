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

#ifndef OUTPUT_TABLE_FRAME_H
#define OUTPUT_TABLE_FRAME_H

#include <QFrame>
#include <QPushButton>

class AbstractMutableOutputCatalog;
class MyTableView;

class OutputTableFrame : public QFrame
{
Q_OBJECT
public:
    explicit OutputTableFrame(QWidget *parent = 0);

    void setModel(AbstractMutableOutputCatalog* amoc);
    void setReadOnly(bool readOnly);

signals:

public slots:
    void add();
    void remove();

protected slots:
    void updateButtons();

protected:
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    MyTableView* m_tableView;

    AbstractMutableOutputCatalog* m_outputCatalog;
};

#endif // OUTPUT_TABLE_FRAME_H
