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

#ifndef OUTPUT_PAGE_H_
#define OUTPUT_PAGE_H_

#include "AbstractPage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QFrame>
#include <QSpinBox>
#include <QSignalMapper>
#include <QTabWidget>
#include <QTableView>

class DimensionLayout;
class OutputTableFrame;
class SiteResponseModel;

//! Widget for the Output Specification page.

class OutputPage : public AbstractPage
{
    Q_OBJECT

public:
    explicit OutputPage(QWidget *parent = nullptr, Qt::WindowFlags f = 0);

    void setModel(SiteResponseModel* model);

public slots:
    void setReadOnly(bool b);

protected slots:
    void setApproach(int approach);

private:
    QTabWidget* _tabWidget;

    QTableView* _profilesTableView;
    OutputTableFrame* _timeSeriesTableFrame;
    OutputTableFrame* _spectraTableFrame;
    OutputTableFrame* _ratiosTableFrame;
    QTableView* _soilTypesTableView;

    QGroupBox* _respSpecGroupBox;
    DimensionLayout* _periodLayout;
    QDoubleSpinBox* _dampingSpinBox;

    QGroupBox* _freqGroupBox;
    DimensionLayout* _frequencyLayout;

    QGroupBox* _logGroupBox;
    QComboBox* _logLevelComboBox;

    //! Create the response spectrum group box
    auto createRespSpecGroupBox() -> QGroupBox*;

    //! Create the frequency group box
    auto createFreqGroupBox() -> QGroupBox*;

    //! Create the output group box
    auto createLogGroupBox() -> QGroupBox*;

    SiteResponseModel* _model;
};
#endif
