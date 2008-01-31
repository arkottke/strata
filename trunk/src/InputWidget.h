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

#ifndef INPUT_WIDGET_H_
#define INPUT_WIDGET_H_

#include <QScrollArea>
#include <QTabWidget>

#include "SiteResponseModel.h"
#include "GeneralPage.h"
#include "SoilTypePage.h"
#include "SoilProfilePage.h"
#include "MotionPage.h"
#include "OutputPage.h"
#include "ComputePage.h"

#include <QPrinter>

class InputWidget : public QScrollArea
{
    Q_OBJECT
	
    public:
        InputWidget( QWidget * parent = 0);
       
        SiteResponseModel * model() const;

        SiteResponseOutput * outputModel();

    public slots:
        void reset();
        
        void open(const QString fileName);
        void save(const QString fileName);

        void exportData();
        void print(QPrinter * printer);

	protected slots:
        void disableTabs(bool);
        void refreshTab(int tab);

	signals:
        void save();
        void modified();
        void soilIsVaried(int state);
        void velocityIsVaried(int state);

        void linkActivated(QString link);

        void finishedComputing();
    private:
        //! Model that holds all of the information
        SiteResponseModel * m_model;

        QTabWidget * m_tabWidget;

        /*! @name Tabs
         */
        //@{
        GeneralPage * m_generalPage;
        SoilTypePage * m_soilTypePage;
        SoilProfilePage * m_soilProfilePage;
        MotionPage * m_motionPage;
        OutputPage * m_outputPage;
        ComputePage * m_computePage;
        //@}

        //! Create the widgets in the page
        void createPage();
};
#endif
