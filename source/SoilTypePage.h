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

#ifndef SOIL_TYPE_PAGE_H_
#define SOIL_TYPE_PAGE_H_

#include "AbstractPage.h"

#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QPointer>
#include <QSpinBox>

class NonlinearProperty;
class NonlinearPropertyDelegate;
class NonlinearPropertyUncertaintyWidget;
class SiteResponseModel;
class SoilTypeCatalog;
class TableGroupBox;

//! Widget for the Soil Type Page.

class SoilTypePage : public AbstractPage
{
    Q_OBJECT

public:
    explicit SoilTypePage(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setModel(SiteResponseModel* model);

signals:
    void soilPropertiesNeeded(bool needed);

public slots:
    void setReadOnly(bool readOnly);

protected slots:
    void selectIndex(const QModelIndex &current, const QModelIndex &previous);
    void setCurrentNonlinearProperty(NonlinearProperty* np);
    void updateUnits();

    void updateDampingRequired(bool dampingRequired);
    void updateNonlinearPropertiesRequired(bool nonlinearPropertiesRequired);
    void updateVariedColumn(bool show);

private:
    // Functions to set up the various group boxes
    auto createLayersGroupBox() -> QGroupBox*;
    auto createBedrockGroupBox() -> QGroupBox*;
    auto createWaterTableDepthGroupBox() -> QGroupBox*;
    auto createVariationGroupBox() -> QGroupBox*;
    auto createSoilPropsGroupBox() -> QGroupBox*;
    auto createNlPropTableBox() -> QGroupBox*;

    TableGroupBox* _soilTypeTableBox;     
    NonlinearPropertyDelegate* _modulusDelegate;
    NonlinearPropertyDelegate* _dampingDelegate;
    TableGroupBox* _nlPropTableBox;

    QDoubleSpinBox* _bedrockUntWtSpinBox;
    QDoubleSpinBox* _bedrockDampingSpinBox;
    QCheckBox* _varyBedrockDampingCheckBox;

    QDoubleSpinBox* _waterTableDepthSpinBox;

    QGroupBox* _randomizerGroupBox;
    QComboBox* _nprModelComboBox;
    NonlinearPropertyUncertaintyWidget* _modulusUncertWidget;
    NonlinearPropertyUncertaintyWidget* _dampingUncertWidget;
    QDoubleSpinBox* _correlSpinBox;

    QGroupBox* _soilPropsGroupBox;
    QDoubleSpinBox* _stressSpinBox;
    QDoubleSpinBox* _piSpinBox;
    QDoubleSpinBox* _ocrSpinBox;
    QDoubleSpinBox* _freqSpinBox;
    QSpinBox* _nCyclesSpinBox;

    QPointer<SoilTypeCatalog> _soilTypeCatalog;

    //! If the widget is in read-only mode
    bool _readOnly;

    //! If nonlinear properties are required by the model.
    bool _nonlinearPropsRequired;
};
#endif
