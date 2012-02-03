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

#include "AbstractPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGroupBox>
#include <QSpinBox>
#include <QToolBox>

class SiteResponseModel;
class SoilTypeDelegate;
class TableGroupBox;
class VelocityVariation;

//! Widget for the Soil Profile Page.

class SoilProfilePage : public AbstractPage
{
    Q_OBJECT

public:
    SoilProfilePage(QWidget* parent = 0, Qt::WindowFlags f = 0 );

    void setModel(SiteResponseModel* model);

public slots:
    void setReadOnly(bool readOnly);

protected slots:
    void updateUnits();
    void updateHiddenColumns();


    void setVelocityItemEnabled(bool enabled);
    void setLayeringItemEnabled(bool enabled);
    void setBedrockDepthItemEnabled(bool enabled);

    void setBedrockDepthMin(double min);
    void setBedrockDepthMax(double max);

private:
    //! Create the table group box
    QGroupBox* createTableGroupBox();

    //! Create the profile variation group box
    QGroupBox* createProfileRandomizerGroupBox();

    //! Create the velocity variation group box
    QFrame* createVelocityFrame();

    //! Create the layering group box
    QFrame* createLayeringFrame();

    //! Create the bedrock depth variation group box
    QFrame* createBedrockDepthFrame();

    QToolBox* m_parameterToolBox;

    enum Index {
        VelocityIndex,
        LayeringIndex,
        BedrockDepthIndex
    };

    TableGroupBox* m_soilProfileTableGroup;
    SoilTypeDelegate* m_soilTypeDelegate;

    QGroupBox* m_profileVariationGroupBox;

    QCheckBox* m_isVelocityVariedCheckBox;
    QCheckBox* m_isLayeringVariedCheckBox;
    QCheckBox* m_isBedrockDepthVariedCheckBox;

    QFrame* m_velocityVariationFrame;
    QCheckBox* m_layerSpecificCheckBox;
    QComboBox* m_distributionComboBox;
    QComboBox* m_stdevModelComboBox;
    QDoubleSpinBox* m_stdevSpinBox;
    QComboBox* m_correlModelComboBox;
    QGroupBox* m_correlGroupBox;
    QDoubleSpinBox* m_correlInitialSpinBox;
    QDoubleSpinBox* m_correlFinalSpinBox;
    QDoubleSpinBox* m_correlDeltaSpinBox;
    QDoubleSpinBox* m_correlInterceptSpinBox;
    QDoubleSpinBox* m_correlExponentSpinBox;

    QFrame* m_layerVariationFrame;
    QComboBox* m_layeringModelComboBox;
    QDoubleSpinBox* m_layeringCoeffSpinBox;
    QDoubleSpinBox* m_layeringInitialSpinBox;
    QDoubleSpinBox* m_layeringExponentSpinBox;

    QFrame* m_bedrockDepthFrame;
    QComboBox* m_bedrockModelComboBox;
    QDoubleSpinBox* m_bedrockStdevSpinBox;
    QCheckBox* m_bedrockDepthMinCheckBox;
    QDoubleSpinBox* m_bedrockDepthMinSpinBox;
    QCheckBox* m_bedrockDepthMaxCheckBox;
    QDoubleSpinBox* m_bedrockDepthMaxSpinBox;

    VelocityVariation* m_velocityVariation;
};
#endif
