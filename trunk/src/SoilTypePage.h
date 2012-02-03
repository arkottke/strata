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
class NonlinearPropertyStandardDeviationWidget;
class SiteResponseModel;
class SoilTypeCatalog;
class TableGroupBox;

//! Widget for the Soil Type Page.

class SoilTypePage : public AbstractPage
{
    Q_OBJECT

public:
    SoilTypePage(QWidget* parent = 0, Qt::WindowFlags f = 0 );

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
    QGroupBox* createLayersGroupBox();
    QGroupBox* createBedrockGroupBox();
    QGroupBox* createWaterTableDepthGroupBox();
    QGroupBox* createVariationGroupBox();
    QGroupBox* createSoilPropsGroupBox();
    QGroupBox* createNlPropTableBox();

    TableGroupBox* m_soilTypeTableBox;     
    NonlinearPropertyDelegate* m_modulusDelegate;
    NonlinearPropertyDelegate* m_dampingDelegate;
    TableGroupBox* m_nlPropTableBox;

    QDoubleSpinBox* m_bedrockUntWtSpinBox;
    QDoubleSpinBox* m_bedrockDampingSpinBox;
    QCheckBox* m_varyBedrockDampingCheckBox;

    QDoubleSpinBox* m_waterTableDepthSpinBox;

    QGroupBox* m_randomizerGroupBox;
    QComboBox* m_nprModelComboBox;
    NonlinearPropertyStandardDeviationWidget* m_modulusStdevWidget;
    NonlinearPropertyStandardDeviationWidget* m_dampingStdevWidget;
    QDoubleSpinBox* m_correlSpinBox;

    QGroupBox* m_soilPropsGroupBox;
    QDoubleSpinBox* m_stressSpinBox;
    QDoubleSpinBox* m_piSpinBox;
    QDoubleSpinBox* m_ocrSpinBox;
    QDoubleSpinBox* m_freqSpinBox;
    QSpinBox* m_nCyclesSpinBox;

    QPointer<SoilTypeCatalog> m_soilTypeCatalog;

    //! If the widget is in read-only mode
    bool m_readOnly;
};
#endif
