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

#ifndef MOTION_PAGE_H_
#define MOTION_PAGE_H_

#include "AbstractPage.h"

#include <QGroupBox>
#include <QModelIndex>
#include <QPushButton>

class DepthComboBox;
class MotionLibrary;
class MyTableView;
class SiteResponseModel;

//! Widget for the AbstractMotion Page.

class MotionPage : public AbstractPage
{
    Q_OBJECT

public:
    MotionPage(QWidget*  parent = 0, Qt::WindowFlags f = 0 );

    void setModel(SiteResponseModel* model);

public slots:
    void setReadOnly(bool b);
    void setApproach(int i);

private slots:
    void add();
    void remove();
    void importSuite();

    void edit();

    void updateButtons();

private:
    //! Create the group box for defining the input location
    QGroupBox* createInputLocationGroupBox();

    //! Create the group box for editting the input motions
    QGroupBox* createMotionsTableGroupBox();

    QGroupBox* _inputLocationGroupBox;
    QGroupBox* _motionsTableGroupBox;

    QPushButton* _addButton;
    QPushButton* _removeButton;
    QPushButton* _editButton;
    QPushButton* _importButton;
    MyTableView* _tableView;

    DepthComboBox* _depthComboBox;

    MotionLibrary* _motionLibrary;

    //! If the model is in read-only mode
    bool _readOnly;
};
#endif
