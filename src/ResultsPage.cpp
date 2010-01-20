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

#include "ResultsPage.h"
#include "OutputExportDialog.h"
#include "ConfigurePlotDialog.h"

#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QDebug>
#include <QImage>
#include <QApplication>
#include <QClipboard>
#include <QPainter>

#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>

ResultsPage::ResultsPage( SiteResponseOutput * model, QWidget * parent)
    : QSplitter(parent), m_model(model)
{
    reset();

    createTableGroup();
    createPlot();
    createContextMenu();

    // Set the default sizes of the QSplitter
    setStretchFactor( 0, 1 );
    setStretchFactor( 1, 4 );
}

ResultsPage::~ResultsPage()
{
    for (int i = 0; i < m_quantileCurves.size(); ++i )
        delete m_quantileCurves.takeFirst();

    for (int i = 0; i < m_dataCurves.size(); ++i )
        delete m_dataCurves.takeFirst();
}
    
void ResultsPage::reset()
{
    m_selectedOutput = 0;
    m_selectedRow = -1;
    m_statsNeedUpdate = false;
    m_zOrder = 20;
}

void ResultsPage::setStatsNeedUpdate()
{
    m_statsNeedUpdate = true;
    m_recomputePushButton->setEnabled(true);
}

void ResultsPage::exportData()
{
    OutputExportDialog dialog(m_model,this);
    dialog.exec();
}

void ResultsPage::print(QPrinter * printer)
{
    uncolorSelectedCurve();

    if ( m_selectedOutput->orientation() == Qt::Horizontal )
        printer->setOrientation(QPrinter::Landscape);
    else if ( m_selectedOutput->orientation() == Qt::Vertical )
        printer->setOrientation(QPrinter::Portrait);

    m_plot->print(*printer);

    colorSelectedCurve();
}

void ResultsPage::refreshWidget()
{
    // Set the table model
    m_tableView->resizeColumnsToContents();
    m_tableView->resizeRowsToContents();

    // Delete the old data curves
    while ( 0 < m_dataCurves.size())
        delete m_dataCurves.takeFirst();
   
    // Create data curves
    for (int i = 0; i < m_model->motionCount(); ++i) {
        for (int j = 0; j < m_model->siteCount(); ++j) {
            // Create a new curve
            m_dataCurves << new QwtPlotCurve;
            m_dataCurves.last()->setZ(m_zOrder);
            // Attach the curve to the plot
            m_dataCurves.last()->attach(m_plot);
        }
    }
    
    // Update the combobox
    m_outputComboBox->clear();
    m_outputComboBox->addItems(m_model->outputNames());

    if ( m_outputComboBox->count() > 0 ) {
        // Select the first item in the combo list
        m_outputComboBox->setCurrentIndex(0);
        plotOutput(0);
    }
}

void ResultsPage::showPlotContextMenu(const QPoint & point)
{
    m_plotContextMenu->popup( m_plot->mapToGlobal(point) );
}

void ResultsPage::copyPlot()
{
    uncolorSelectedCurve();
   
    // Set the clilpboard image
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setPixmap(QPixmap::grabWidget(m_plot));
    
    colorSelectedCurve();
}

void ResultsPage::configurePlot()
{
    ConfigurePlotDialog * dialog = new ConfigurePlotDialog(m_plot);

    if ( dialog->exec() )
        m_plot->replot();
}

void ResultsPage::selectedChanged( const QModelIndex & current, const QModelIndex & /*previous*/ )
{
    // Set the old record to gray
    uncolorSelectedCurve();
    
    m_selectedRow = current.row();

    // Colorized newly selected row
    colorSelectedCurve();

    m_plot->replot();

    // Update the buttons
    bool enabled = m_model->siteEnabledAt(m_selectedRow);
    m_enableSitePushButton->setChecked( enabled );
    m_enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg( enabled ? tr("Disable") : tr("Enable") )
            .arg( m_model->siteNameAt(m_selectedRow) ) );
    

    enabled = m_model->motionEnabledAt(m_selectedRow);
    m_enableMotionPushButton->setChecked( enabled );
    m_enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg( enabled ? tr("Disable") : tr("Enable") )
            .arg( m_model->motionNameAt(m_selectedRow) ) );
}

void ResultsPage::plotOutput(int index)
{
    m_selectedOutput = m_model->output().at(index);

    m_tableModel->setSelectedOutput(m_selectedOutput);
    m_tableView->selectRow(m_selectedRow);

    // Set the labels of the axis
    m_selectedOutput->configurePlot(m_plot);

    // Create the curves of each of the objects
    int idx = 0;
    for (int i = 0; i < m_model->motionCount(); ++i) {
        for (int j = 0; j < m_model->siteCount(); ++j) {
            if ( ( m_selectedOutput->isMotionIndependent() && i != 0 ) 
                    || ( m_selectedOutput->isSiteIndependent() && j != 0 ) )
                // Clear the data from the curves
                m_dataCurves[idx]->detach();
            else  {
                // Set the data
                m_selectedOutput->dataToCurve( i, j, m_dataCurves[idx]);
                m_dataCurves[idx]->attach(m_plot);
            }

            ++idx;
        }
    }

    // Hide the appropriate buttons if the output is site or motion independent
    if ( m_selectedOutput->isSiteIndependent() ) {
        m_tableView->hideColumn(0);
        m_enableSitePushButton->hide();

        m_tableView->showColumn(1);
        m_enableMotionPushButton->show();
    } else if ( m_selectedOutput->isMotionIndependent() ) {
        m_tableView->hideColumn(1);
        m_enableMotionPushButton->hide();

        m_tableView->showColumn(0);
        m_enableSitePushButton->show();
    } else {
        m_tableView->showColumn(0);
        m_tableView->showColumn(1);

        m_enableSitePushButton->show();
        m_enableMotionPushButton->show();
    }


    populateQuantiles();

    // Highlight selected row
    if ( 0 <= m_selectedRow )
        colorSelectedCurve();

    // Refresh the plot
    m_plot->replot();
}

void ResultsPage::pointSelected(const QPoint & point)
{
    // Find the index oshowf the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;

    for ( int i = 0; i < m_dataCurves.size(); ++i ) {
        if ( !m_dataCurves.at(i)->plot() )
            // Continue if not attached to the plot
            continue;

        m_dataCurves.at(i)->closestPoint( point, &distance);
    
        if ( minDistance < 0 || distance < minDistance ) {
            minDistance = distance;
            minIndex = i;
        }
    }
    // Select this item from the table
    m_tableView->selectRow(minIndex);
}

void ResultsPage::enableSelectedCurve()
{
    colorSelectedCurve();

    m_plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::colorSelectedCurve()
{
    if ( !m_selectedOutput || m_selectedRow < 0 )
        return;

    bool enabled;
    
    if (m_selectedOutput->isMotionIndependent() )
        enabled = m_model->siteEnabledAt(m_selectedRow);
    else if ( m_selectedOutput->isSiteIndependent() )
        enabled = m_model->motionEnabledAt(m_selectedRow);
    else 
        enabled = m_model->seriesEnabledAt(m_selectedRow);

    if ( enabled )
        m_dataCurves[m_selectedRow]->setPen(QPen(QBrush(Qt::darkGreen), 2));
    else
        m_dataCurves[m_selectedRow]->setPen(QPen(QBrush(Qt::darkRed), 2));

    m_dataCurves[m_selectedRow]->setZ(m_zOrder+1);
}

void ResultsPage::uncolorSelectedCurve()
{
    if ( !m_selectedOutput || m_selectedRow < 0 )
        return;

    // Turn the selected curve gray and set it to the regular zOrder
    m_dataCurves[m_selectedRow]->setPen(QPen(Qt::darkGray));
    m_dataCurves[m_selectedRow]->setZ(m_zOrder);
}

void ResultsPage::setMotionEnabled(bool enabled)
{
    if (  0 <= m_selectedRow )
        m_model->setMotionEnabledAt( m_selectedRow, enabled);
    
    m_enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg( enabled ? tr("Disable") : tr("Enable") )
            .arg( m_model->motionNameAt(m_selectedRow) ) );

    colorSelectedCurve();
    m_plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::setSiteEnabled(bool enabled)
{
    if (  0 <= m_selectedRow )
        m_model->setSiteEnabledAt( m_selectedRow, enabled);
    
    m_enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg( enabled ? tr("Disable") : tr("Enable") )
            .arg( m_model->siteNameAt(m_selectedRow) ) );

    // Re-color and re-plot selected curve incase the enable/disabled flag changed
    colorSelectedCurve();
    m_plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::recomputeStats()
{
    m_model->computeStats();

    populateQuantiles();

    m_plot->replot();
   
    m_statsNeedUpdate = false;
    m_recomputePushButton->setEnabled(false);
}

void ResultsPage::populateQuantiles()
{
    if ( m_selectedOutput->hasStats() ) {
        // Update the quantiles
        m_selectedOutput->quantilesToCurves(m_quantileCurves);  

        for (int i = 0; i < m_quantileCurves.size(); ++i ) {
            m_quantileCurves[i]->attach(m_plot);
            // Set the curves on top of the data
            m_quantileCurves[i]->setZ(m_zOrder+10);
        }
    } else {
        for (int i = 0; i < m_quantileCurves.size(); ++i )
            m_quantileCurves[i]->detach();
    }
}

void ResultsPage::createTableGroup() 
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(2,1);

    // Type combo box
    m_outputComboBox = new QComboBox;

    connect( m_outputComboBox, SIGNAL(activated(int)), this, SLOT(plotOutput(int)));

    layout->addWidget(new QLabel(tr("Output:")), 0, 0);
    layout->addWidget(m_outputComboBox, 0, 1, 1, 3);
    
    // Create the table view
    m_tableView = new QTableView;
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_tableModel = new OutputTableModel(m_model);
    m_tableView->setModel(m_tableModel);
    
    connect( m_tableView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedChanged(QModelIndex,QModelIndex)));

    layout->addWidget( m_tableView, 1, 0, 1, 4);

    // Create Push buttons
    m_enableSitePushButton = new QPushButton;
    m_enableSitePushButton->setCheckable(true);
    
    connect( m_enableSitePushButton, SIGNAL(clicked(bool)), this, SLOT(setSiteEnabled(bool)));

    layout->addWidget( m_enableSitePushButton, 2, 0 );
    
    m_enableMotionPushButton = new QPushButton;
    m_enableMotionPushButton->setCheckable(true);

    connect( m_enableMotionPushButton, SIGNAL(clicked(bool)), this, SLOT(setMotionEnabled(bool)));

    layout->addWidget( m_enableMotionPushButton, 2, 1 );

    m_recomputePushButton = new QPushButton(tr("Recompute Statistics"));
    m_recomputePushButton->setEnabled(false);

    connect( m_recomputePushButton, SIGNAL(clicked()), this, SLOT(recomputeStats()));

    layout->addWidget( m_recomputePushButton, 2, 3 );

    // Create the group box
    QGroupBox * group = new QGroupBox(tr("Data Selection"));

    group->setLayout(layout);

    addWidget( group );
    
    connect( m_tableModel, SIGNAL(enabledChanged()), this, SLOT(enableSelectedCurve()));
}

void ResultsPage::createPlot()
{
    // Plot
    m_plot = new QwtPlot;
    m_plot->setCanvasBackground(Qt::white);
    m_plot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect( m_plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPlotContextMenu(QPoint)));
    
    // Picker to allow for selection of the closest curve and displays curve
    // coordinates with a cross rubber band.
    QwtPlotPicker * picker = new QwtPlotPicker( 
            QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection,
            QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly, m_plot->canvas());

    connect( picker, SIGNAL(appended(QPoint)), this, SLOT(pointSelected(QPoint)));

    /*
    // Legend
    QwtLegend * legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_plot->insertLegend(legend, QwtPlot::BottomLegend);
    */

    addWidget(m_plot);
    
    // Create quantile curves
    for (int i = 0; i < 3; ++i ) {
        m_quantileCurves << new QwtPlotCurve;
        m_quantileCurves.last()->attach(m_plot);
    }
}

void ResultsPage::createContextMenu() 
{
    // Create the context menu
    m_plotContextMenu = new QMenu;
    m_plotContextMenu->addAction(QIcon(":/images/edit-copy.svg"), tr("Copy"), this, SLOT(copyPlot()));
    m_plotContextMenu->addSeparator();
    m_plotContextMenu->addAction(tr("Plot Options"), this, SLOT(configurePlot()));
}

