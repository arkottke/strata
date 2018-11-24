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
        : AbstractPage(parent), _outputCatalog(nullptr), _selectedOutput(nullptr)
{
    _statsNeedUpdate = false;

    auto *splitter = new QSplitter;

    splitter->addWidget(createOutputGroup());
    splitter->setStretchFactor(0, 1);

    splitter->addWidget(createDataTabWidget());
    splitter->setStretchFactor(1, 4);

    auto *layout = new QHBoxLayout;
    layout->addWidget(splitter);
    setLayout(layout);

    createContextMenu();
}

void ResultsPage::setModel(SiteResponseModel* model) {
    _selectedOutput = nullptr;
    _selectedRow = -1;

    // Remove the previous model and delete the selectionModel
    if (QItemSelectionModel* m = _catalogTableView->selectionModel()) {
        _catalogTableView->setModel(nullptr);
        delete m;
    }

    // Need to check if the model has results -- otherwise there is a list of
    // outputs with no data associated with them.
    if (!model->hasResults())
        return;

    _outputCatalog = model->outputCatalog();
    _catalogTableView->setModel(_outputCatalog);

    connect(_outputCatalog, SIGNAL(enabledChanged(int)),
            this, SLOT(enableSelectedCurve(int)));
    connect(_catalogTableView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(setSelectedSeries(QModelIndex,QModelIndex)));

    _outputComboBox->clear();
    _outputComboBox->addItems(_outputCatalog->outputNames());

    if (_outputComboBox->count())
        setSelectedOutput(0);
}

void ResultsPage::setStatsNeedUpdate()
{
    _statsNeedUpdate = true;
    _recomputePushButton->setEnabled(true);
}

void ResultsPage::exportData()
{
    OutputExportDialog dialog(_outputCatalog, this);
    dialog.exec();
}

void ResultsPage::print(QPrinter* printer)
{
    uncolorCurve(_selectedRow);

//    if (_selectedOutput->orientation() == Qt::Horizontal )
//        printer->setOrientation(QPrinter::Landscape);
//    else if ( _selectedOutput->orientation() == Qt::Vertical )
//        printer->setOrientation(QPrinter::Portrait);

    _plot->render(printer);
    colorCurve(_selectedRow);
}

void ResultsPage::showPlotContextMenu(const QPoint & point)
{
    _plotContextMenu->popup( _plot->mapToGlobal(point) );
}

void ResultsPage::copyPlot()
{
    uncolorCurve(_selectedRow);
   
    // Set the clilpboard image
    QClipboard * clipboard = QApplication::clipboard();
    clipboard->setPixmap(QPixmap::grabWidget(_plot));
    
    colorCurve(_selectedRow);
}

void ResultsPage::configurePlot()
{
    auto *dialog = new ConfigurePlotDialog(_plot);

    if (dialog->exec())
        _plot->replot();

    // FIXME save properties to selected output
}

void ResultsPage::setSelectedSeries(const QModelIndex & current, const QModelIndex & previous)
{
    Q_UNUSED(previous);

    if (_selectedRow >= 0)
        // Set the old record to gray
        uncolorCurve(_selectedRow);
    
    _selectedRow = current.row();

    Q_ASSERT(_selectedOutput);
    _selectedOutput->setMotionIndex(
            _selectedOutput->intToMotion(_selectedRow));

    // Colorized newly selected row
    colorCurve(_selectedRow);

    _plot->replot();

    // Update the buttons
    bool enabled = _outputCatalog->siteEnabled(_selectedRow);
    _enableSitePushButton->setChecked(enabled);
    _enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(_outputCatalog->siteNumberAt(_selectedRow)));
    
    enabled = _outputCatalog->motionEnabled(_selectedRow);
    _enableMotionPushButton->setChecked( enabled );
    _enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(_outputCatalog->motionNameAt(_selectedRow)));

    // Select the appropriate column
    if (_selectedOutput->needsTime()) {
        _outputTableView->selectColumn(_selectedRow / _selectedOutput->motionCount() + 1);
    } else {
        _outputTableView->selectColumn(_selectedRow + 1);
    }
}

void ResultsPage::selectedDataChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    int row = current.column() - 1;

    if (row >= 0) {
        if (_selectedOutput->needsTime())
            // Need to modify the row because only one time series is shown at a time
            row = row * _selectedOutput->motionCount() + _selectedOutput->motionIndex();

        _catalogTableView->selectRow(row);
    }
}

void ResultsPage::setSelectedOutput(int index)
{
    if (_selectedOutput)
        disconnect(_selectedOutput, nullptr, this, nullptr);

    _selectedOutput = _outputCatalog->setSelectedOutput(index);

    if (_selectedOutput->rowCount() <= _selectedRow)
        _selectedRow = -1;

    _selectedOutput->plot(_plot, _curves);

#if QWT_VERSION >= 0x060100
    _plot->updateLegend();
#endif

    _catalogTableView->resizeColumnsToContents();
    _catalogTableView->resizeRowsToContents();

    // Remove the previous model and delete the selectionModel
    if (QItemSelectionModel *m = _outputTableView->selectionModel()) {
        _outputTableView->setModel(nullptr);
        delete m;
    }
    _outputTableView->setModel(_selectedOutput);
    _outputTableView->resizeColumnsToContents();
    _outputTableView->resizeRowsToContents();

    // Need to remove the previous connections because otherwise it signals multiple times
    disconnect(_outputTableView->selectionModel(), nullptr, this, nullptr);
    connect(_outputTableView->selectionModel(), SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)),
               this, SLOT(selectedDataChanged(QModelIndex,QModelIndex)));

    // Site indepedent
    _catalogTableView->setColumnHidden(
            OutputCatalog::SiteColumn,
            _selectedOutput->siteIndependent());
    _enableSitePushButton->setHidden(
            _selectedOutput->siteIndependent());

    // Motion independent
    _catalogTableView->setColumnHidden(
            OutputCatalog::MotionColumn,
            _selectedOutput->motionIndependent());
    _enableMotionPushButton->setHidden(
            _selectedOutput->motionIndependent());

    if (_selectedOutput && _selectedRow >= 0)
        setSelectedSeries(_outputCatalog->index(_selectedRow, 0));
}

void ResultsPage::pointSelected(const QPoint & point)
{
    // Find the index oshowf the closest curve
    double distance;
    double minDistance = -1;
    int minIndex = 0;

    for (int i = 0; i < _curves.size(); ++i) {
        _curves.at(i)->closestPoint(point, &distance);
    
        if (i == 0 || distance < minDistance) {
            minDistance = distance;
            minIndex = i;
        }
    }

    // Select this item from the table
    _catalogTableView->selectRow(minIndex);
}

void ResultsPage::enableSelectedCurve(int index)
{
    colorCurve(index);
    _plot->replot();
    setStatsNeedUpdate();
}

void ResultsPage::colorCurve(int row)
{
    if (!_selectedOutput || _selectedRow < 0)
        return;

    QPen pen = _outputCatalog->enabledAt(row) ?
               QPen(QBrush(Qt::darkGreen), 2) : QPen(QBrush(Qt::darkRed), 2);

    // Turn the selected curve gray and set it to the regular zOrder
    _curves[row]->setPen(pen);
    _curves[row]->setZ(AbstractOutput::zOrder()+2);    
}

void ResultsPage::uncolorCurve(int row)
{
    if (0 <= row && row < _curves.size()) {
        // Turn the selected curve gray and set it to the regular zOrder
        _curves[row]->setPen(QPen(Qt::darkGray));
        _curves[row]->setZ(AbstractOutput::zOrder());
    }
}

void ResultsPage::setMotionEnabled(bool enabled)
{
    _outputCatalog->setMotionEnabled(
            _selectedOutput->intToMotion(_selectedRow), enabled);

    _enableMotionPushButton->setText( QString(tr("%1 Motion: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(_outputCatalog->motionNameAt(_selectedRow)));

    colorCurve(_selectedRow);
    _plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::setSiteEnabled(bool enabled)
{
    _outputCatalog->setSiteEnabled(
            _selectedOutput->intToSite(_selectedRow), enabled);

    _enableSitePushButton->setText( QString(tr("%1 Site: %2"))
            .arg(enabled ? tr("Disable") : tr("Enable"))
            .arg(_outputCatalog->siteNumberAt(_selectedRow)));

    // Re-color and re-plot selected curve incase the enable/disabled flag changed
    colorCurve(_selectedRow);
    _plot->replot();

    setStatsNeedUpdate();
}

void ResultsPage::recomputeStats()
{
    _outputCatalog->finalize();

    // FIXME -- this is sloppy because only the curves from the statistics change
    _selectedOutput->plot(_plot, _curves);
   
    _statsNeedUpdate = false;
    _recomputePushButton->setEnabled(false);

}

QGroupBox* ResultsPage::createOutputGroup()
{
    auto *layout = new QGridLayout;
    layout->setColumnStretch(2,1);

    // Type combo box
    _outputComboBox = new QComboBox;

    connect(_outputComboBox, SIGNAL(activated(int)),
            this, SLOT(setSelectedOutput(int)));

    layout->addWidget(new QLabel(tr("Output:")), 0, 0);
    layout->addWidget(_outputComboBox, 0, 1, 1, 3);
    
    // Create the table view
    _catalogTableView = new QTableView;
    _catalogTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    _catalogTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    layout->addWidget(_catalogTableView, 1, 0, 1, 4);

    // Create Push buttons
    _enableSitePushButton = new QPushButton;
    _enableSitePushButton->setCheckable(true);
    
    connect(_enableSitePushButton, SIGNAL(clicked(bool)),
             this, SLOT(setSiteEnabled(bool)));

    layout->addWidget( _enableSitePushButton, 2, 0 );
    
    _enableMotionPushButton = new QPushButton;
    _enableMotionPushButton->setCheckable(true);

    connect(_enableMotionPushButton, SIGNAL(clicked(bool)),
             this, SLOT(setMotionEnabled(bool)));

    layout->addWidget( _enableMotionPushButton, 2, 1 );

    _recomputePushButton = new QPushButton(tr("Recompute Statistics"));
    _recomputePushButton->setEnabled(false);

    connect(_recomputePushButton, SIGNAL(clicked()),
             this, SLOT(recomputeStats()));

    layout->addWidget(_recomputePushButton, 2, 3);

    // Create the group box
    auto *groupBox = new QGroupBox(tr("Data Selection"));
    groupBox->setLayout(layout);

    return groupBox;
}

QTabWidget* ResultsPage::createDataTabWidget()
{
    auto *tabWidget = new QTabWidget;

    // Plot
    _plot = new QwtPlot(tabWidget);
    _plot->setCanvasBackground(Qt::white);
    _plot->setContextMenuPolicy(Qt::CustomContextMenu);
    _plot->setAutoReplot(false);

    connect(_plot, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(showPlotContextMenu(QPoint)));
    
    // Picker to allow for selection of the closest curve and displays curve
    // coordinates with a cross rubber band.
    auto *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                     QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                                     _plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    connect(picker, SIGNAL(appended(QPoint)), this, SLOT(pointSelected(QPoint)));

    // Legend
    auto *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    _plot->insertLegend(legend, QwtPlot::BottomLegend);
#if QWT_VERSION >= 0x060100
    connect(_plot,
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
            auto *curve = new QwtPlotCurve(pair.first);

        curve->setLegendIconSize(QSize(32, 8));
        curve->setPen(QPen(QBrush(pair.second), 2));
        curve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
#if QWT_VERSION < 0x060100
        curve->updateLegend(legend);
#else
        _plot->updateLegend(curve);
#endif
    }

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(_plot);

    tabWidget->addTab(scrollArea, tr("Plot"));

    _outputTableView = new MyTableView;
    _outputTableView->setReadOnly(true);

    tabWidget->addTab(_outputTableView, tr("Data Table"));

    return tabWidget;
}

void ResultsPage::createContextMenu() 
{
    // Create the context menu
    _plotContextMenu = new QMenu;
    _plotContextMenu->addAction(QIcon(":/images/edit-copy.svg"), tr("Copy"), this, SLOT(copyPlot()));
    _plotContextMenu->addSeparator();
    _plotContextMenu->addAction(tr("Plot Options"), this, SLOT(configurePlot()));
}
