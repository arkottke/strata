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
#include <QPushButton>
#include <QTableView>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

//! Widget for the Soil Profile Page.

class SoilProfilePage : public QWidget
{
    Q_OBJECT

    public:
        SoilProfilePage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    public slots:
        void setIsVaried(bool);
        void setVelocityIsVaried(bool);
        void setIsLayerSpecific(bool);

        void setReadOnly(bool);

        void updateStdevModel(int model);
        void updateCorrelModel(int model);
        void updateLayeringModel(int model);
        void updateBedrockModel(int model);

        void load();

    protected slots:
        void updateUnits();

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
        QDoubleSpinBox * m_stdevSpinBox;
        QDoubleSpinBox * m_correlInitialSpinBox;
        QDoubleSpinBox * m_correlFinalSpinBox;
        QDoubleSpinBox * m_correlDeltaSpinBox;
        QDoubleSpinBox * m_depthInterceptSpinBox;
        QDoubleSpinBox * m_exponentSpinBox;
        
        QCheckBox * m_isLayeringVariedCheckBox;
        QGroupBox * m_layerVariationGroupBox;
        QComboBox * m_layeringModelComboBox;
        QDoubleSpinBox * m_layeringCoeffSpinBox;
        QDoubleSpinBox * m_layeringInitialSpinBox;
        QDoubleSpinBox * m_layeringExponentSpinBox;

        QCheckBox * m_isBedrockDepthVariedCheckBox;
        QGroupBox * m_bedrockDepthGroupBox;
        QComboBox * m_bedrockModelComboBox;
        QDoubleSpinBox * m_bedrockStdevSpinBox;
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
