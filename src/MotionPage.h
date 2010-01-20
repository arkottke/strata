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

#ifndef MOTION_PAGE_H_
#define MOTION_PAGE_H_

#include "SiteResponseModel.h"
#include "TableGroupBox.h"
#include "DepthComboBox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QStackedWidget>
#include <QTabWidget>
#include <QWidget>

//! Widget for the Motion Page.

class MotionPage : public QWidget
{
    Q_OBJECT

    public:
        MotionPage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

    public slots:
        void setMethod(int);
        
        void setReadOnly(bool b);

        void load();

    signals:
        void updateFas();

    protected slots:
        void setSource(int);
        void setLocation(int);


        void previewData();
        void previewPlot();
        void previewCrustalAmp();

        void loadSuite();

        void setEnableCrustalTab(bool enabled = false );

    private:
        //! Load point source paramters
        void loadPointSourceParams();


        SiteResponseModel * m_model;

        QStackedWidget * m_stackedWidget;
        
        QGroupBox * m_inputLocationGroupBox;
        DepthComboBox * m_depthComboBox;

        TableGroupBox * m_recordedMotionTableBox;
        
        QComboBox * m_typeComboBox;
        QDoubleSpinBox * m_durationSpinBox;
        QCheckBox * m_limitFasCheckBox;
        QComboBox * m_sourceComboBox;
        QDoubleSpinBox * m_dampingSpinBox;
        QDoubleSpinBox * m_maxEngFreqSpinBox;

        QStackedWidget * m_rvtSourceStackedWidget;
        TableGroupBox * m_fourierSpecTableBox;
        TableGroupBox * m_respSpecTableBox;

        QGroupBox * m_rvtPropsGroupBox;

        QGroupBox * m_pointSourceGroupBox;
        QDoubleSpinBox * m_momentMagSpinBox;
        QDoubleSpinBox * m_distanceSpinBox;
        QDoubleSpinBox * m_depthSpinBox;
        QComboBox * m_locationComboBox;
        QDoubleSpinBox * m_stressDropSpinBox;
        QDoubleSpinBox * m_geoAttenSpinBox;
        QDoubleSpinBox * m_pathDurSpinBox;
        QDoubleSpinBox * m_pathAttenCoeffSpinBox;
        QDoubleSpinBox * m_pathAttenPowerSpinBox;
        QDoubleSpinBox * m_shearVelSpinBox;
        QDoubleSpinBox * m_densitySpinBox;
        QDoubleSpinBox * m_siteAttenSpinBox;
        QCheckBox * m_specificCrustalAmpCheckBox;
        QPushButton * m_computeCrustalAmpButton;
        QTabWidget * m_siteTabWidget;
        TableGroupBox * m_siteAmpTableGroupBox; 
        TableGroupBox * m_crustalTableGroupBox; 

        QPointer<QDialog> m_dataDialog;
        QPointer<QDialog> m_plotDialog;
        
        // Create pages
        void createInputLayer();
        void createRecordedPage();
        void createRvtPage();
        void createPointSourceGroupBox();

        //! Compute the Reponse Spectrum and Fourier Amplitude spectrum of the RvtMotion
        bool computeRvtResponse();
};
#endif
