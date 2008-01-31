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

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QPointer>

class MotionPage : public QWidget
{
    Q_OBJECT

    public:
        MotionPage( SiteResponseModel * model, QWidget * parent = 0, Qt::WindowFlags f = 0 );

        void setModel( SiteResponseModel * model );

    public slots:
        void setMethod(int);

        void load();
        void save();

    signals:
        void updateFas();

        void hasChanged();

    protected slots:
        void setSource(int);

        void previewData();
        void previewPlot();

    private:
        SiteResponseModel * m_model;

        QStackedWidget * m_stackedWidget;
        
        QGroupBox * m_inputLocationGroupBox;
        DepthComboBox * m_depthComboBox;

        TableGroupBox * m_recordedMotionTableBox;
        
        QComboBox * m_typeComboBox;
        QDoubleSpinBox * m_durationSpinBox;
        QDoubleSpinBox * m_strainFactorSpinBox;
        QDoubleSpinBox * m_soilFactorSpinBox;
        QComboBox * m_sourceComboBox;
        QDoubleSpinBox * m_dampingSpinBox;

        QStackedWidget * m_spectraStackedWidget;
        TableGroupBox * m_fourierSpecTableBox;
        TableGroupBox * m_respSpecTableBox;

        QPointer<QDialog> m_dataDialog;
        QPointer<QDialog> m_plotDialog;
        
        // Create pages
        void createInputLayer();
        void createRecordedPage();
        void createRvtPage();

        //! Compute the Reponse Spectrum and Fourier Amplitude spectrum of the RvtMotion
        bool computeRvtResponse();
};
#endif
