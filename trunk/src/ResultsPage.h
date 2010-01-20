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

#ifndef RESULTS_PAGE_H_
#define RESULTS_PAGE_H_

#include "SiteResponseOutput.h"
#include "OutputTableModel.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QPrinter>
#include <QMenu>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

//! Widget for the Results page.

class ResultsPage : public QSplitter
{
    Q_OBJECT
	
    public:
        ResultsPage( SiteResponseOutput * model = 0, QWidget * parent = 0);
        ~ResultsPage();

    signals:

    public slots:
        void exportData();
        void print(QPrinter * printer);

        void refreshWidget();

    protected slots:
        //! Show the context menu
        void showPlotContextMenu(const QPoint & point);

        //! Copy the plot to the clipboard
        void copyPlot();

        void configurePlot();

        //! Update the plot when a specific output is selected
        void selectedChanged( const QModelIndex & current, const QModelIndex & previous);

        void plotOutput(int index);

        void pointSelected(const QPoint & point);

        void enableSelectedCurve();
        
        void colorSelectedCurve();
        void uncolorSelectedCurve();

        void setMotionEnabled(bool enabled);
        void setSiteEnabled(bool enabled);

        void recomputeStats();

        void setStatsNeedUpdate();

    protected:
        void populateQuantiles();
        void reset();

    private:
        //! Create the table goup box
        void createTableGroup();

        //! Create the plot
        void createPlot();
        
        //! Create context menu for the chart
        void createContextMenu();

        //! Data for the widget
        SiteResponseOutput * m_model;

        QComboBox * m_outputComboBox;

        QTableView * m_tableView;
        OutputTableModel * m_tableModel;

        QPushButton * m_enableMotionPushButton;
        QPushButton * m_enableSitePushButton;
        QPushButton * m_recomputePushButton;

        QwtPlot * m_plot; 
        QList<QwtPlotCurve*> m_dataCurves;
        QList<QwtPlotCurve*> m_quantileCurves;

        QMenu * m_plotContextMenu;

        //! Currently selected output
        const Output * m_selectedOutput;

        //! Currently selected row
        int m_selectedRow;

        //! If the statistics need to be re-computed
        bool m_statsNeedUpdate;

        //! Default zOrder of all of the data curves
        double m_zOrder;
};
#endif

