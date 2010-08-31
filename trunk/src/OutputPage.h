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
// Copyright 2007 Albert Kottke
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
    OutputPage(QWidget* parent = 0, Qt::WindowFlags f = 0 );

    void setModel(SiteResponseModel* model);

public slots:
    void setReadOnly(bool b);

protected slots:
    void setApproach(int approach);

private:
    QTabWidget* m_tabWidget;

    QTableView* m_profilesTableView;
    OutputTableFrame* m_timeSeriesTableFrame;
    OutputTableFrame* m_spectraTableFrame;
    OutputTableFrame* m_ratiosTableFrame;
    QTableView* m_soilTypesTableView;

    QGroupBox* m_respSpecGroupBox;
    DimensionLayout* m_periodLayout;
    QDoubleSpinBox* m_dampingSpinBox;

    QGroupBox* m_freqGroupBox;
    DimensionLayout* m_frequencyLayout;

    QGroupBox* m_logGroupBox;
    QComboBox* m_logLevelComboBox;

    //! Create the response spectrum group box
    QGroupBox* createRespSpecGroupBox();

    //! Create the frequency group box
    QGroupBox* createFreqGroupBox();

    //! Create the output group box
    QGroupBox* createLogGroupBox();

    SiteResponseModel* m_model;
};
#endif
