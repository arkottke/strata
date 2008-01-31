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

#include "SiteResponseModel.h"
#include "TableGroupBox.h"
#include "DepthComboBox.h"

#include <QWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QModelIndex>

class OutputPage : public QWidget
{
    Q_OBJECT

    public:
        OutputPage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

        void setModel( SiteResponseModel * model );

    public slots:
        void refresh();

        void setMethod(int);

        void load();
        void save();

    signals:
        void hasChanged();

    private:
        SiteResponseModel * m_model;

        TableGroupBox * m_responseTableBox;
        TableGroupBox * m_ratioTableBox;

        QGroupBox * m_soilTypesGroupBox;
        QTableView * m_soilTypeTableView;

        QGroupBox * m_profilesGroupBox; 
        QCheckBox * m_initialShearVelCheckBox;
        QCheckBox * m_finalShearVelCheckBox;
        QCheckBox * m_finalShearModCheckBox;
        QCheckBox * m_finalDampingCheckBox;
        QCheckBox * m_vTotalStressCheckBox;
        QCheckBox * m_maxShearStressCheckBox;
        QCheckBox * m_maxShearStrainCheckBox;
        QCheckBox * m_maxAccelCheckBox;
        QCheckBox * m_stressRatioCheckBox;
        QCheckBox * m_maxErrorCheckBox;

        QGroupBox * m_respSpecGroupBox;
        QDoubleSpinBox * m_dampingSpinBox;
        QDoubleSpinBox * m_periodMinSpinBox;
        QDoubleSpinBox * m_periodMaxSpinBox;
        QSpinBox * m_periodNptsSpinBox;
        QComboBox * m_periodSpacingComboBox;
        
        QGroupBox * m_freqGroupBox;
        QDoubleSpinBox * m_freqMinSpinBox;
        QDoubleSpinBox * m_freqMaxSpinBox;
        QSpinBox * m_freqNptsSpinBox;
        QComboBox * m_freqSpacingComboBox;

        QGroupBox * m_logGroupBox;
        QComboBox * m_logLevelComboBox;

        //! Create the gropu box that contains the soil types
        void createSoilTypesGroupBox();

        //! Create the group box that contains the profile check boxes
        void createProfilesGroupBox();

        //! Create the response spectrum group box
        void createRespSpecGroupBox();

        //! Create the frequency group box
        void createFreqGroupBox();

        //! Create the output group box
        void createLogGroupBox();
};
#endif
