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

#ifndef GENERAL_PAGE_H_
#define GENERAL_PAGE_H_

#include "AbstractPage.h"

#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>


class MethodGroupBox;

//! Widget for the General Page.

class GeneralPage : public AbstractPage
{
    Q_OBJECT

public:
    GeneralPage(QWidget* parent = 0, Qt::WindowFlags f = 0);

    void setModel(SiteResponseModel* model);

public slots:
    void setReadOnly(bool readOnly);

protected:
    //@{ Construct the various group boxes
    QGroupBox* createProjectGroupBox();
    QGroupBox* createAnalysisGroupBox();
    QGroupBox* createVariationGroupBox();
    QGroupBox* createDiscretizationGroupBox();
    //@}

    QLineEdit* m_titleLineEdit;
    QTextEdit* m_notesTextEdit;
    QLineEdit* m_prefixLineEdit;
    QComboBox* m_unitsComboBox;
    QCheckBox* m_saveMotionDataCheckBox;

    QComboBox* m_methodComboBox;
    QComboBox* m_approachComboBox;
    QCheckBox* m_propertiesAreVariedCheckBox;

    QGroupBox* m_variationGroupBox;
    QSpinBox* m_countSpinBox;
    QCheckBox* m_nlPropertiesAreVariedCheckBox;
    QCheckBox* m_siteIsVariedCheckBox;
    QCheckBox* m_specifiedSeedCheckBox;
    QSpinBox* m_seedSpinBox;

    MethodGroupBox* m_methodGroupBox;

    QDoubleSpinBox* m_maxFreqSpinBox;
    QDoubleSpinBox* m_waveFractionSpinBox;
    QCheckBox* m_disableDiscretzationCheckBox;
};
#endif
