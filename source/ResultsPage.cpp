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

#include "AbstractOutput.h"
#include "ConfigurePlotDialog.h"
#include "EditActions.h"
#include "MyTableView.h"
#include "OutputCatalog.h"
#include "OutputExportDialog.h"
#include "SiteResponseModel.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QClipboard>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QSplitter>
#include <QScrollArea>

#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>

#if QWT_VERSION < 0x060100
#include <qwt_legend_item.h>
#else
#include <qwt_plot_legenditem.h>
#endif

ResultsPage::ResultsPage(QWidget * parent)
    : AbstractPage(parent), m_outputCatalog(0), m_selectedOutput(0)
{
    m_statsNeedUpdate = false;

    QSplitter* splitter = new QSplitter;

    splitter->addWidget(createOutputGroup());
    splitter->setStretchFactor(0, 1);

    splitter->addWidget(createDataTabWidget());
    splitter->setStretchFactor(1, 4);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(splitter);
    setLayout(layout);

    createContextMenu();
}

void ResultsPage::setModel(SiteResponseModel* model) {
    m_selectedOutput = 0;
    m_selectedRow = -1;

    // Remove the previous model and delete the selectionModel
    if (QItemSelectionModel* m = m_catalogTableView->selectionModel()) {
        m_catalogTableView->setModel(0);
        delete m;
    }

    // Need to check if the model has results -- otherwise there is a list of
    // outputs with no data associated with them.
    if (!model->hasResults())
        return;

    m_outputCatalog = model->outputCatalog();
    m_catalogTableView->setModel(m_outputCatalog);

    connect(m_outputCatalog, SIGNAL(enabledChanged(int)),
            this, SLOT(enableSelectedCurve(int)));
    connect(m_catalogTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(setSelectedSeries(QModelIndex,QModelIndex)));

    m_outputComboBox->clear();
    m_outputComboBox->addItems(m_outputCatalog->outputNames());

    if (m_outputComboBox->count())
        setSelectedOutput(0);
}

void ResultsPage::setStatsNeedUpdate()
{
    m_statsNeedUpdate = true;
    m_recomputePushButton->setEnabled(true);
}

void ResultsPage::exportData()
{
    OutputExportDialog dialog(m_outputCatalog, this);
    dialog.exec();
}

void ResultsPage::print(QPrinter* printer)
{
    uncolorCurve(m_selectedRow);

//    if (m_selectedOutput->orientation() == Qt::Horizontal )
//        printer->setOrientation(QPrinter::Landscape);
//    else if ( m_selectedOutput->orientation() == Qt::Vertical )
//        printer->setOrientation(QPrinter::Portrait);

    m_plot->render(printer);
    colorCurve(m_selectedRow);
}

void ResultsPage::showPlotContextMenu(const QPoint & point)
{
    m_plotContextMenu->popup( m_plot->mapToGlobal(point) );
}

void ResultsPage::copyPlot()
{
    uncolorCurve(m_selectedRow);
   
    // Set the clilpboard image
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setPixmap(QPixmap::grabWidget(m_plot));
    
    colorCurve(m_selectedRow);
}

void ResultsPage::configurePlot()
{
    ConfigurePlotDialog * dialog = new ConfigurePlotDialog(m_plot);

    if (dialog->exec())
        m_plot->replot();

    // FIXME save properties to selected output
}

void ResultsPage::setSelectedSeries(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    if (m_selectedRow >= 0)
        // Set the old record to gray
        uncolorCurve(m_selectedRow);
    
    m_selectedRow = current.row();

    Q_ASSERT(m_selectedOutput);
    m_selectedOutput->setMotionIndex(
            m_selectedOutput->intToMotion(m_selectedRow));

    // Colorized newly selected row
    colorCurve(m_selectedRow);

    m_plot->replot();

    // Update the buttons
    bool enabled = m_outputCatalog->siteEnabled(m_selectedRow);
    m_enableSitePushButton->setChecked(enabled);
    m_enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(m_outputCatalog->siteNumberAt(m_selectedRow)));
    
    enabled = m_outputCatalog->motionEnabled(m_selectedRow);
    m_enableMotionPushButton->setChecked( enabled );
    m_enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(m_outputCatalog->motionNameAt(m_selectedRow)));

    // Select the appropriate column
    if (m_selectedOutput->needsTime()) {
        m_outputTableView->selectColumn(m_selectedRow / m_selectedOutput->motionCount() + 1);
    } else {
        m_outputTableView->selectColumn(m_selectedRow + 1);
    }
}

void ResultsPage::selectedDataChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    int row = current.column() - 1;

    if (row >= 0) {
        if (m_selectedOutput->needsTime())
            // Need to modify the row because only one time series is shown at a time
            row = row * m_selectedOutput->motionCount() + m_selectedOutput->motionIndex();

        m_catalogTableView->selectRow(row);
    }
}

void ResultsPage::setSelectedOutput(int index)
{
    if (m_selectedOutput)
        disconnect(m_selectedOutput, 0, this, 0);

    m_selectedOutput = m_outputCatalog->setSelectedOutput(index);

    if (m_selectedOutput->rowCount() <= m_selectedRow)
        m_selectedRow = -1;

    m_selectedOutput->plot(m_plot, m_curves);

#if QWT_VERSION >= 0x060100
    m_plot->updateLegend();
#endif

    m_catalogTableView->resizeColumnsToContents();
    m_catalogTableView->resizeRowsToContents();

    // Remove the previous model and delete the selectionModel
    if (QItemSelectionModel *m = m_outputTableView->selectionModel()) {
        m_outputTableView->setModel(0);
        delete m;
    }
    m_outputTableView->setModel(m_selectedOutput);
    m_outputTableView->resizeColumnsToContents();
    m_outputTableView->resizeRowsToContents();

    // Need to remove the previous connections because otherwise it signals multiple times
    disconnect(m_outputTableView->selectionModel(), 0, this, 0);
    connect(m_outputTableView->selectionModel(), SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)),
               this, SLOT(selectedDataChanged(QModelIndex,QModelIndex)));

    // Site indepedent
    m_catalogTableView->setColumnHidden(
            OutputCatalog::SiteColumn,
            m_selectedOutput->siteIndependent());
    m_enableSitePushButton->setHidden(
            m_selectedOutput->siteIndependent());

    // Motion independent
    m_catalogTableView->setColumnHidden(
            OutputCatalog::MotionColumn,
            m_selectedOutput->motionIndependent());
    m_enableMotionPushButton->setHidden(
            m_selectedOutput->motionIndependent());

    if (m_selectedOutput && m_selectedRow >= 0)
        setSelectedSeries(m_outputCatalog->index(m_selectedRow, 0));
}

void ResultsPage::pointSelected(const QPoint & point)
{
    // Find the index oshowf the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;

    for (int i = 0; i < m_curves.size(); ++i) {
        m_curves.at(i)->closestPoint(point, &distance);
    
        if (i == 0 || distance < minDistance) {
            minDistance = distance;
            minIndex = i;
        }
    }

    // Select this item from the table
    m_catalogTableView->selectRow(minIndex);
}

void ResultsPage::enableSelectedCurve(int index)
{
    colorCurve(index);
    m_plot->replot();
    setStatsNeedUpdate();
}

void ResultsPage::colorCurve(int row)
{
    if (!m_selectedOutput || m_selectedRow < 0)
        return;

    QPen pen = m_outputCatalog->enabledAt(row) ?
               QPen(QBrush(Qt::darkGreen), 2) : QPen(QBrush(Qt::darkRed), 2);

    // Turn the selected curve gray and set it to the regular zOrder
    m_curves[row]->setPen(pen);
    m_curves[row]->setZ(AbstractOutput::zOrder()+2);    
}

void ResultsPage::uncolorCurve(int row)
{
    if (0 <= row && row < m_curves.size()) {
        // Turn the selected curve gray and set it to the regular zOrder
        m_curves[row]->setPen(QPen(Qt::darkGray));
        m_curves[row]->setZ(AbstractOutput::zOrder());
    }
}

void ResultsPage::setMotionEnabled(bool enabled)
{
    m_outputCatalog->setMotionEnabled(
            m_selectedOutput->intToMotion(m_selectedRow), enabled);

    m_enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(m_outputCatalog->motionNameAt(m_selectedRow)));

    colorCurve(m_selectedRow);
    m_plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::setSiteEnabled(bool enabled)
{
    m_outputCatalog->setSiteEnabled(
            m_selectedOutput->intToSite(m_selectedRow), enabled);

    m_enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(m_outputCatalog->siteNumberAt(m_selectedRow)));

    // Re-color and re-plot selected curve incase the enable/disabled flag changed
    colorCurve(m_selectedRow);
    m_plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::recomputeStats()
{
    m_outputCatalog->finalize();

    // FIXME -- this is sloppy because only the curves from the statistics change
    m_selectedOutput->plot(m_plot, m_curves);
   
    m_statsNeedUpdate = false;
    m_recomputePushButton->setEnabled(false);

}

QGroupBox* ResultsPage::createOutputGroup()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(2,1);

    // Type combo box
    m_outputComboBox = new QComboBox;

    connect(m_outputComboBox, SIGNAL(activated(int)),
            this, SLOT(setSelectedOutput(int)));

    layout->addWidget(new QLabel(tr("Output:")), 0, 0);
    layout->addWidget(m_outputComboBox, 0, 1, 1, 3);
    
    // Create the table view
    m_catalogTableView = new QTableView;
    m_catalogTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_catalogTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    layout->addWidget(m_catalogTableView, 1, 0, 1, 4);

    // Create Push buttons
    m_enableSitePushButton = new QPushButton;
    m_enableSitePushButton->setCheckable(true);
    
    connect(m_enableSitePushButton, SIGNAL(clicked(bool)),
             this, SLOT(setSiteEnabled(bool)));

    layout->addWidget( m_enableSitePushButton, 2, 0 );
    
    m_enableMotionPushButton = new QPushButton;
    m_enableMotionPushButton->setCheckable(true);

    connect(m_enableMotionPushButton, SIGNAL(clicked(bool)),
             this, SLOT(setMotionEnabled(bool)));

    layout->addWidget( m_enableMotionPushButton, 2, 1 );

    m_recomputePushButton = new QPushButton(tr("Recompute Statistics"));
    m_recomputePushButton->setEnabled(false);

    connect(m_recomputePushButton, SIGNAL(clicked()),
             this, SLOT(recomputeStats()));

    layout->addWidget(m_recomputePushButton, 2, 3);

    // Create the group box
    QGroupBox* groupBox = new QGroupBox(tr("Data Selection"));
    groupBox->setLayout(layout);

    return groupBox;
}

QTabWidget* ResultsPage::createDataTabWidget()
{
    QTabWidget *tabWidget = new QTabWidget;

    // Plot
    m_plot = new QwtPlot;
    m_plot->setCanvasBackground(Qt::white);
    m_plot->setContextMenuPolicy(Qt::CustomContextMenu);
    m_plot->setAutoReplot(false);

    connect(m_plot, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlotContextMenu(QPoint)));
    
    // Picker to allow for selection of the closest curve and displays curve
    // coordinates with a cross rubber band.
    QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                                              m_plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    connect(picker, SIGNAL(appended(QPoint)), this, SLOT(pointSelected(QPoint)));

    // Legend
    QwtLegend* legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_plot->insertLegend(legend, QwtPlot::BottomLegend);
#if QWT_VERSION >= 0x060100
    connect(m_plot,
            SIGNAL(legendDataChanged(QVariant,QList<QwtLegendData>)),
            legend,
            SLOT(updateLegend(QVariant,QList<QwtLegendData>)));
#endif

    // Add the generic curves to the legend
    QList<QPair<QString, Qt::GlobalColor> > pairs;
    pairs << qMakePair(tr("Unselected Realization"), Qt::darkGray)
          << qMakePair(tr("Selected and Enabled Realization"), Qt::darkGreen)
          << qMakePair(tr("Selected and Disabled Realization"), Qt::darkRed);

    QPair<QString, Qt::GlobalColor> pair;
    foreach (pair, pairs) {
        QwtPlotCurve *curve = new QwtPlotCurve(pair.first);

        curve->setLegendIconSize(QSize(32, 8));
        curve->setPen(QPen(QBrush(pair.second), 2));
        curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
#if QWT_VERSION < 0x060100
        curve->updateLegend(legend);
#else
        m_plot->updateLegend(curve);
#endif
    }
	
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_plot);

    tabWidget->addTab(scrollArea, tr("Plot"));

    m_outputTableView = new MyTableView;
    m_outputTableView->setReadOnly(true);

    tabWidget->addTab(m_outputTableView, tr("Data Table"));

    return tabWidget;
}

void ResultsPage::createContextMenu() 
{
    // Create the context menu
    m_plotContextMenu = new QMenu;
    m_plotContextMenu->addAction(QIcon(":/images/edit-copy.svg"), tr("Copy"), this, SLOT(copyPlot()));
    m_plotContextMenu->addSeparator();
    m_plotContextMenu->addAction(tr("Plot Options"), this, SLOT(configurePlot()));
}
