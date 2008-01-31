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

#include "SiteResponseModel.h"

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>

class GeneralPage : public QWidget
{
	Q_OBJECT

	public:
		GeneralPage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

        void setModel( SiteResponseModel * model );

    public slots:
        void setIsSiteVaried(int);
        void setIsSoilVaried(int);
        void setIsVelocityVaried(int);
        void setMethod(int);

        void save();
        void load();

    signals:
        void isSoilVariedChanged(bool);
        void isVelocityVariedChanged(bool);
        
        void methodChanged(int);

        void hasChanged();

	private:
        SiteResponseModel * m_model;

		QGroupBox * m_projectGroupBox;
		QLineEdit * m_titleLineEdit;
		QTextEdit * m_notesTextEdit;
		QLineEdit * m_prefixLineEdit;
		QComboBox * m_unitsComboBox;
        QCheckBox * m_saveMotionDataCheckBox;

        QGroupBox * m_analysisGroupBox;
		QComboBox * m_methodComboBox;
		QCheckBox * m_siteIsVariedCheckBox;

		QGroupBox * m_siteVarGroupBox;
		QSpinBox * m_countSpinBox;
		QCheckBox * m_soilIsVariedCheckBox;
		QCheckBox * m_velIsVariedCheckBox;

		QGroupBox * m_equivLinearGroupBox;
		QLineEdit * m_strainRatioLineEdit;
		QLineEdit * m_errorTolLineEdit;
		QSpinBox * m_iterationsSpinBox;
        
        QGroupBox * m_discretizationGroupBox;
        QLineEdit * m_maxFreqLineEdit;
        QLineEdit * m_waveFractionLineEdit;
       

		// Construct the various group boxes
		void createProjectGroupBox();

        void createAnalysisGroupBox();
		void createVariationGroupBox();
		void createEquivLinearGroupBox();

        //! Create the auto-discretization group box
        void createDiscretizationGroupBox();
};
#endif
