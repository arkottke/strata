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
    explicit GeneralPage(QWidget *parent = nullptr, Qt::WindowFlags f = 0);

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

  QLineEdit *_titleLineEdit;
  QTextEdit *_notesTextEdit;
  QLineEdit *_prefixLineEdit;
  QComboBox *_unitsComboBox;
  QCheckBox *_saveMotionDataCheckBox;

  QComboBox *_methodComboBox;
  QComboBox *_approachComboBox;
  QCheckBox *_propertiesAreVariedCheckBox;

  QGroupBox *_variationGroupBox;
  QSpinBox *_countSpinBox;
  QCheckBox *_nlPropertiesAreVariedCheckBox;
  QCheckBox *_siteIsVariedCheckBox;
  QCheckBox *_specifiedSeedCheckBox;
  QSpinBox *_seedSpinBox;

  MethodGroupBox *_methodGroupBox;

  QDoubleSpinBox *_maxFreqSpinBox;
  QDoubleSpinBox *_waveFractionSpinBox;
  QCheckBox *_disableDiscretzationCheckBox;
};
#endif
