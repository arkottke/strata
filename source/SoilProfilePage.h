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

class SoilProfilePage : public AbstractPage {
  Q_OBJECT

public:
  SoilProfilePage(QWidget *parent = nullptr,
                  Qt::WindowFlags f = Qt::WindowFlags());

  void setModel(SiteResponseModel *model);

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
  auto createTableGroupBox() -> QGroupBox *;

  //! Create the profile variation group box
  auto createProfileRandomizerGroupBox() -> QGroupBox *;

  //! Create the velocity variation group box
  auto createVelocityFrame() -> QFrame *;

  //! Create the layering group box
  auto createLayeringFrame() -> QFrame *;

  //! Create the bedrock depth variation group box
  auto createBedrockDepthFrame() -> QFrame *;

  QToolBox *_parameterToolBox;

  enum Index { VelocityIndex, LayeringIndex, BedrockDepthIndex };

  TableGroupBox *_soilProfileTableGroup;
  SoilTypeDelegate *_soilTypeDelegate;

  QGroupBox *_profileVariationGroupBox;

  QCheckBox *_isVelocityVariedCheckBox;
  QCheckBox *_isLayeringVariedCheckBox;
  QCheckBox *_isBedrockDepthVariedCheckBox;

  QFrame *_velocityVariationFrame;
  QCheckBox *_layerSpecificCheckBox;
  QComboBox *_distributionComboBox;
  QComboBox *_stdevModelComboBox;
  QDoubleSpinBox *_stdevSpinBox;
  QComboBox *_correlModelComboBox;
  QGroupBox *_correlGroupBox;
  QDoubleSpinBox *_correlInitialSpinBox;
  QDoubleSpinBox *_correlFinalSpinBox;
  QDoubleSpinBox *_correlDeltaSpinBox;
  QDoubleSpinBox *_correlInterceptSpinBox;
  QDoubleSpinBox *_correlExponentSpinBox;

  QFrame *_layerVariationFrame;
  QComboBox *_layeringModelComboBox;
  QDoubleSpinBox *_layeringCoeffSpinBox;
  QDoubleSpinBox *_layeringInitialSpinBox;
  QDoubleSpinBox *_layeringExponentSpinBox;

  QFrame *_bedrockDepthFrame;
  QComboBox *_bedrockModelComboBox;
  QDoubleSpinBox *_bedrockStdevSpinBox;
  QCheckBox *_bedrockDepthMinCheckBox;
  QDoubleSpinBox *_bedrockDepthMinSpinBox;
  QCheckBox *_bedrockDepthMaxCheckBox;
  QDoubleSpinBox *_bedrockDepthMaxSpinBox;

  VelocityVariation *_velocityVariation;
};
#endif
