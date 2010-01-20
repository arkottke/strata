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

#include "SiteResponseModel.h"
#include "NonlinearProperty.h"
#include "TableGroupBox.h"

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QPointer>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>

//! Widget for the Soil Type Page.

class SoilTypePage : public QWidget
{
	Q_OBJECT

	public:
		SoilTypePage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );
        
	public slots:
        void setIsVaried(bool b);
        void setReadOnly(bool b);
        
        void unselectSoil(); 
        void setSelectedSoil( const QModelIndex & current, const QModelIndex & previous );
        void refreshSoilDetails();

        void load();

    protected slots: 
        void updateUnits();
    
        void loadStdevModels();

        void loadSoilProperties();

    signals:
        void soilTypesChanged();
        void linkActivated(const QString & link);

	private:
		SiteResponseModel * m_model;

		TableGroupBox * m_layersTableBox;

        QGroupBox * m_bedrockGroupBox;
		QDoubleSpinBox * m_bedrockUntWtSpinBox;
		QDoubleSpinBox * m_bedrockDampingSpinBox;
        QCheckBox * m_varyBedrockDampingCheckBox;
		
		QGroupBox * m_variationGroupBox;
        QComboBox * m_dynPropModelComboBox;
        QLineEdit * m_shearModStdevLineEdit;
		QDoubleSpinBox * m_shearModMaxSpinBox;
		QDoubleSpinBox * m_shearModMinSpinBox;
        QLineEdit * m_dampingStdevLineEdit;
		QDoubleSpinBox * m_dampingMaxSpinBox;
		QDoubleSpinBox * m_dampingMinSpinBox;
		QDoubleSpinBox * m_correlSpinBox;

        QGroupBox * m_soilPropsGroupBox;
	    QDoubleSpinBox * m_stressSpinBox;
	    QDoubleSpinBox * m_piSpinBox;
	    QDoubleSpinBox * m_ocrSpinBox;
	    QDoubleSpinBox * m_freqSpinBox;
	    QSpinBox * m_nCyclesSpinBox;

		TableGroupBox * m_nlPropTableBox;
    
        QModelIndex m_selectedIndex;
        QPointer<SoilType> m_selectedSoilType;
        QPointer<NonlinearProperty> m_selectedNonlinearProperty;

		// Functions to set up the various group boxes
		void createLayersGroupBox();
        void createBedrockGroupBox();
		void createVariationGroupBox();
        void createSoilPropsGroupBox();
		void createNlPropTableBox();
};
#endif
