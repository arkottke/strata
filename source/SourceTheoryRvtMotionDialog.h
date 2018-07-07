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

#ifndef SOURCE_THEORY_RVT_MOTION_DIALOG_H
#define SOURCE_THEORY_RVT_MOTION_DIALOG_H

#include "AbstractRvtMotionDialog.h"

class TableGroupBox;
class SourceTheoryRvtMotion;

class SourceTheoryRvtMotionDialog : public AbstractRvtMotionDialog
{
Q_OBJECT
public:
    explicit SourceTheoryRvtMotionDialog(SourceTheoryRvtMotion *motion, bool readOnly,
                                         QWidget *parent = nullptr);
protected slots:
    void updatePathDurSource(int source);
    void updateCrustalAmpSource(int source);

protected:
    virtual QFormLayout* createParametersLayout();

    int _crustModelIndex;

    QTabWidget* _paramsTabWidget;

    TableGroupBox* _pathDurTableGroupBox;
    TableGroupBox* _crustalModelGroupBox;
    TableGroupBox* _crustalAmpGroupBox;
};

#endif // SOURCE_THEORY_RVT_MOTION_DIALOG_H
