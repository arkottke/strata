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

#ifndef SOIL_PROFILE_PAGE_H_
#define SOIL_PROFILE_PAGE_H_

#include "SiteResponseModel.h"
#include "TableGroupBox.h"

#include <QWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

class SoilProfilePage : public QWidget
{
    Q_OBJECT

    public:
        SoilProfilePage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

        void setModel( SiteResponseModel * model );

    public slots:
        void setIsVaried(bool);
        void setVelocityIsVaried(bool);
        void setIsLayerSpecific(bool);

        void updateStdevModel(int model);
        void updateCorrelModel(int model);
        void updateLayeringModel(int model);
        void updateBedrockModel(int model);

        void load();
        void save();

    protected slots:
        void updateUnits();

    signals:
        void hasChanged();

    private:
        SiteResponseModel * m_model;
    
        TableGroupBox * m_tableGroupBox;

        QGroupBox * m_profileVariationGroupBox;
        
        QCheckBox * m_isVelocityVariedCheckBox;
        
        QGroupBox * m_velocityVariationGroupBox;
        QComboBox * m_distributionComboBox;
        QComboBox * m_stdevModelComboBox;
        QComboBox * m_correlModelComboBox;
        QCheckBox * m_layerSpecificCheckBox;
        QGroupBox * m_correlGroupBox;
        QLineEdit * m_stdevLineEdit;
        QLineEdit * m_correlInitialLineEdit;
        QLineEdit * m_correlFinalLineEdit;
        QLineEdit * m_correlDeltaLineEdit;
        QLineEdit * m_depthInterceptLineEdit;
        QLineEdit * m_exponentLineEdit;
        
        QCheckBox * m_isLayeringVariedCheckBox;
        QGroupBox * m_layerVariationGroupBox;
        QComboBox * m_layeringModelComboBox;
        QLineEdit * m_layeringCoeffLineEdit;
        QLineEdit * m_layeringInitialLineEdit;
        QLineEdit * m_layeringExponentLineEdit;

        QCheckBox * m_isBedrockDepthVariedCheckBox;
        QGroupBox * m_bedrockDepthGroupBox;
        QComboBox * m_bedrockModelComboBox;
        QLineEdit * m_bedrockStdevLineEdit;
        QCheckBox * m_bedrockDepthMinCheckBox;
        QDoubleSpinBox * m_bedrockDepthMinSpinBox;
        QCheckBox * m_bedrockDepthMaxCheckBox;
        QDoubleSpinBox * m_bedrockDepthMaxSpinBox;


        //! Create the table group box
        void createTableGroupBox();

        //! Create the profile variation group box
        void createProfileVariationGroupBox();
        
        //! Create the velocity variation group box
        void createVelocityVariationGroupBox();

        //! Create the layering group box
        void createLayeringGroupBox();

        //! Create the bedrock depth variation group box
        void createBedrockDepthGroupBox();

        //! Load the standard deviation parameters
        void loadStdev();

        //! Load the correlation parameters
        void loadCorrel();

        //! Load the layering parameters
        void loadLayering();
};
#endif
