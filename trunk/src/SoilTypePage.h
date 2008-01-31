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
#include "NonLinearProperty.h"
#include "TableGroupBox.h"

#include <QWidget>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>

class SoilTypePage : public QWidget
{
	Q_OBJECT

	public:
		SoilTypePage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );
        
        void setModel( SiteResponseModel * model );

	public slots:
        void setIsVaried(bool);
        
        void unselectSoil(); 
        void setSelectedSoil( const QModelIndex & current, const QModelIndex & previous );
        void refreshSoilDetails();

        void save();
        void load();

    protected slots: 
        void updateUnits();
    
        void loadStdevModels();

        void saveSoilProperties();
        void loadSoilProperties();
    signals:
        void hasChanged();
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
		QDoubleSpinBox * m_dampingMinSpinBox;
		QDoubleSpinBox * m_correlSpinBox;

        QGroupBox * m_soilPropsGroupBox;
	    QLineEdit * m_stressLineEdit;
	    QLineEdit * m_piLineEdit;
	    QLineEdit * m_ocrLineEdit;
	    QLineEdit * m_freqLineEdit;
	    QLineEdit * m_nCyclesLineEdit;

		TableGroupBox * m_nlPropTableBox;
    
        QModelIndex m_selectedIndex;
        SoilType * m_selectedSoilType;
        NonLinearProperty * m_selectedNonLinearProperty;

		// Functions to set up the various group boxes
		void createLayersGroupBox();
        void createBedrockGroupBox();
		void createVariationGroupBox();
        void createSoilPropsGroupBox();
		void createNlPropTableBox();
};
#endif
