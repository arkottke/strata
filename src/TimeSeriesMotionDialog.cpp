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

#include "TimeSeriesMotionDialog.h"

#include "MyQwtCompatibility.h"
#include "ResponseSpectrum.h"
#include "TimeSeriesMotion.h"

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextStream>
#include <QWhatsThis>

#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

#include <climits>

TimeSeriesMotionDialog::TimeSeriesMotionDialog(TimeSeriesMotion * motion,  bool readOnly, QWidget *parent, Qt::WindowFlags f)
        : QDialog(parent, f), m_motion(motion)
{
    QVBoxLayout *layout = new QVBoxLayout;

    m_tabWidget = new QTabWidget;

    m_tabWidget->addTab(createInputFrame(readOnly), tr("Input"));
    m_tabWidget->addTab(createPlotsFrame(), tr("Plots"));    

    // Plot the motion if available
    if (m_motion->accel().size()) {
        plot();
    } else {
        m_tabWidget->setTabEnabled(1, false);
    };

    layout->addWidget(m_tabWidget);

    // Button Row
    QDialogButtonBox * buttonBox = 0;

    if (readOnly) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    } else {
        buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel
                | QDialogButtonBox::Apply | QDialogButtonBox::Help, Qt::Horizontal, this);
        connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
        connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
                this, SLOT(apply()));
    }
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttonBox);

    setLayout(layout);

    if (!m_motion->fileName().isEmpty()) {
        loadPreview(m_motion->fileName());
        updateTextEdit();
    }
}


void TimeSeriesMotionDialog::openFile()
{
    QSettings settings;
    // Prompt for a fileName
    QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Select a time series..."),
            settings.value("motionDirectory",
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                           ).toString());

    if (!fileName.isEmpty()) {
        QFileInfo fileInfo = QFileInfo(fileName);
        settings.setValue("motionDirectory", fileInfo.absoluteFilePath());

        m_fileNameLineEdit->setText(fileInfo.absoluteFilePath());
        loadPreview(fileName);

        // Try to load the file
        bool success = m_motion->load(fileName, true);
        if (success) {
            updateTextEdit();
            plot();
        } else {
            // Clear input boxes
            m_descripLineEdit->clear();
            m_pointCountSpinBox->clear();
            m_timeStepSpinBox->clear();
        }

        // Update the dialog
        m_tabWidget->setTabEnabled(1, success);
    }
}

void TimeSeriesMotionDialog::updateTextEdit()
{
    m_textEdit->blockSignals(true);
    int startLine = m_startLineSpinBox->value() - 1;
    int stopLine = m_stopLineSpinBox->value();
    // If a stop line is set correct the line number
    if (stopLine) {
        --stopLine;
    }
    // Original format
    QTextCharFormat format = m_textEdit->currentCharFormat();
    // Grab the cursor which is used to process the textedit
    QTextCursor cursor = m_textEdit->textCursor();
    // Move the cursor to the start of the document
    cursor.movePosition( QTextCursor::Start );
    //
    // Header lines
    //
    // Move down the old number of header lines and colorize the selection
    cursor.movePosition( QTextCursor::Down, QTextCursor::KeepAnchor, startLine);
    // Header format is green
    format.setForeground(QBrush(Qt::darkGreen));
    // Set the color
    cursor.setCharFormat(format);
    //
    // Data lines
    //
    // Reset the anchor
    cursor.movePosition( QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    //cursor.movePosition( QTextCursor::Up, QTextCursor::MoveAnchor);
    // Move the cursor down the number of data lines
    if ( !stopLine )
        cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor);
    else {
        cursor.movePosition( QTextCursor::Down, QTextCursor::KeepAnchor, stopLine - startLine);
        cursor.movePosition( QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    }
    // Data format is a blue
    format.setForeground(QBrush(Qt::blue));
    // Set the color
    cursor.setCharFormat(format);
    //
    // Extra lines
    //
    if ( m_stopLineSpinBox->value() && stopLine < m_lineCount) {
        // Move the cursor to start of the document
        cursor.movePosition( QTextCursor::Start );
        // Reset the anchor at the beginning of the next line
        cursor.movePosition( QTextCursor::Down, QTextCursor::MoveAnchor, stopLine+1);
        // Move the cursor down to the end of the document
        cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor);
        // Extra format is a red
        format.setForeground(QBrush(Qt::darkRed));
        // Set the color
        cursor.setCharFormat(format);
    }

    m_textEdit->blockSignals(false);
}

void TimeSeriesMotionDialog::updatePosition()
{
    m_positionSpinBox->setValue(m_textEdit->textCursor().blockNumber()+1);
}

void TimeSeriesMotionDialog::updateDataColumn(int index)
{
    if (index == 1) {
        m_dataColSpinBox->setEnabled(true);
    } else {
        m_dataColSpinBox->setEnabled(false);
    }
}

void TimeSeriesMotionDialog::help()
{
    QWhatsThis::enterWhatsThisMode();
}

void TimeSeriesMotionDialog::tryAccept()
{
    // FIXME the apply button should disable if it has been clicked once and then re-enable if changes occur
    if (tryApply()) {
        accept();
    }
}

void TimeSeriesMotionDialog::apply()
{
    // Clear the plots
    foreach (QwtPlotCurve *curve, QList<QwtPlotCurve *>() << m_atsCurve << m_saCurve << m_fasCurve) {
        curve->setSamples(QVector<double>(), QVector<double>());
    }

    if (tryApply()) {
        plot();
    }

    updateTextEdit();
}

QFrame* TimeSeriesMotionDialog::createInputFrame(bool readOnly)
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 8, 1 );

    // Create the file row
    QPushButton * pushButton = new QPushButton(tr("&File..."));
    pushButton->setWhatsThis(tr("Opens a dialog to select the file."));
    pushButton->setDisabled(readOnly);
    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(openFile()));
    layout->addWidget( pushButton, 0, 0 );

    m_fileNameLineEdit = new QLineEdit;
    m_fileNameLineEdit->setWhatsThis(tr("Filename of acceleration time series."));
    m_fileNameLineEdit->setReadOnly(true);
    m_fileNameLineEdit->setText(m_motion->fileName());

    layout->addWidget(m_fileNameLineEdit, 0, 1, 1, 8);

    // Create the description row
    m_descripLineEdit = new QLineEdit;
    m_descripLineEdit->setWhatsThis(tr("A description of the time series file (optional)."));
    m_descripLineEdit->setText(m_motion->description());
    m_descripLineEdit->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(descriptionChanged(QString)),
            m_descripLineEdit, SLOT(setText(QString)));
    connect(m_descripLineEdit, SIGNAL(textChanged(QString)),
            m_motion, SLOT(setDescription(QString)));

    layout->addWidget(new QLabel(tr("Description:")), 1, 0); 
    layout->addWidget(m_descripLineEdit, 1, 1, 1, 8);

    // Create the time step, point count, and  scale row
    m_pointCountSpinBox = new QSpinBox;
    m_pointCountSpinBox->setWhatsThis(tr("The number of data points in the time series."));
    m_pointCountSpinBox->setRange(0, INT_MAX);
    m_pointCountSpinBox->setValue(m_motion->pointCount());
    m_pointCountSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(pointCountChanged(int)),
            m_pointCountSpinBox, SLOT(setValue(int)));
    connect(m_pointCountSpinBox, SIGNAL(valueChanged(int)),
            m_motion, SLOT(setPointCount(int)));

    layout->addWidget(new QLabel(tr("Point count:")), 2, 0);
    layout->addWidget(m_pointCountSpinBox, 2, 1);

    m_timeStepSpinBox = new QDoubleSpinBox;
    m_timeStepSpinBox->setWhatsThis(tr("The time step between data points."));
    m_timeStepSpinBox->setRange(0.0001, 0.05);
    m_timeStepSpinBox->setSingleStep(0.001);
    m_timeStepSpinBox->setDecimals(4);
    m_timeStepSpinBox->setSuffix(" s");
    m_timeStepSpinBox->setValue(m_motion->timeStep());
    m_timeStepSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(timeStepChanged(double)),
            m_timeStepSpinBox, SLOT(setValue(double)));
    connect(m_timeStepSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setTimeStep(double)));

    layout->addWidget(new QLabel(tr("Time step:")), 2, 2);
    layout->addWidget(m_timeStepSpinBox, 2, 3 );

    m_scaleSpinBox = new QDoubleSpinBox;
    m_scaleSpinBox->setWhatsThis(tr("The scale factor that is applied to the motion."));
    m_scaleSpinBox->setRange(0.001, 20);
    m_scaleSpinBox->setDecimals(2);
    m_scaleSpinBox->setSingleStep(0.01);
    m_scaleSpinBox->setValue(m_motion->scale());
    m_scaleSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(scaleChanged(double)),
            m_scaleSpinBox, SLOT(setValue(double)));
    connect(m_scaleSpinBox, SIGNAL(valueChanged(double)),
            m_motion, SLOT(setScale(double)));

    layout->addWidget(new QLabel(tr("Scale factor:")), 2, 6);
    layout->addWidget(m_scaleSpinBox, 2, 7);

    // Data format row
    m_formatComboBox = new QComboBox;
    m_formatComboBox->setWhatsThis(tr("The format of the data file. <i>Rows</i> "
                "reads every number on every row.  <i>Columns</i> reads data "
                "from a specific column."));
    m_formatComboBox->addItems(TimeSeriesMotion::formatList());
    m_formatComboBox->setCurrentIndex(m_motion->format());
    m_formatComboBox->setEnabled(!readOnly);

    connect(m_formatComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateDataColumn(int)));
    connect(m_motion, SIGNAL(formatChanged(int)),
            m_formatComboBox, SLOT(setCurrentIndex(int)));
    connect(m_formatComboBox, SIGNAL(currentIndexChanged(int)),
            m_motion, SLOT(setFormat(int)));

    layout->addWidget( new QLabel(tr("Format:")), 3, 0);
    layout->addWidget(m_formatComboBox, 3, 1);
    
    m_dataColSpinBox = new QSpinBox;
    m_dataColSpinBox->setWhatsThis(tr("The column of the acceleration data."));
    m_dataColSpinBox->setRange(1, 100);
    m_dataColSpinBox->setEnabled(m_motion->format() == TimeSeriesMotion::Columns);
    m_dataColSpinBox->setValue(m_motion->dataColumn());
    m_dataColSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(dataColumnChanged(int)),
            m_dataColSpinBox, SLOT(setValue(int)));
    connect(m_dataColSpinBox, SIGNAL(valueChanged(int)),
            m_motion, SLOT(setDataColumn(int)));

    layout->addWidget(new QLabel(tr("Data column:")), 3, 2);
    layout->addWidget(m_dataColSpinBox, 3, 3);

    // Units specification
    m_unitsComboBox = new QComboBox;
    m_unitsComboBox->setWhatsThis(tr("Units of the time series."));
    m_unitsComboBox->addItems(TimeSeriesMotion::inputUnitsList());
    m_unitsComboBox->setCurrentIndex(m_motion->inputUnits());
    connect(m_motion, SIGNAL(inputUnitsChanged(int)),
            m_unitsComboBox, SLOT(setCurrentIndex(int)));
    connect(m_unitsComboBox, SIGNAL(currentIndexChanged(int)),
            m_motion, SLOT(setInputUnits(int)));

    layout->addWidget(new QLabel(tr("Units:")), 3, 4);
    layout->addWidget(m_unitsComboBox, 3, 5);

    // PGA of the motion
    m_pgaSpinBox = new QDoubleSpinBox;
    m_pgaSpinBox->setWhatsThis(tr("The peak ground acceleration of the motion."));
    m_pgaSpinBox->setRange(0.001, 10.0);
    m_pgaSpinBox->setDecimals(3);
    m_pgaSpinBox->setSingleStep(0.01);
    m_pgaSpinBox->setSuffix(" g");
    m_pgaSpinBox->setReadOnly(true);
    m_pgaSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_pgaSpinBox->setValue(m_motion->pga());

    connect(m_motion, SIGNAL(pgaChanged(double)),
            m_pgaSpinBox, SLOT(setValue(double)));

    layout->addWidget(new QLabel(tr("PGA:")), 3, 6);
    layout->addWidget(m_pgaSpinBox, 3, 7);

    // Lines row
    m_startLineSpinBox = new QSpinBox;
    m_startLineSpinBox->setMinimum(1);
    m_startLineSpinBox->setWhatsThis(tr("The line number at which to start reading the data"));
    m_startLineSpinBox->setValue(m_motion->startLine());
    m_startLineSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(startLineChanged(int)),
            m_startLineSpinBox, SLOT(setValue(int)));
    connect(m_startLineSpinBox, SIGNAL(valueChanged(int)),
            m_motion, SLOT(setStartLine(int)));

    layout->addWidget(new QLabel(tr("Start line:")), 4, 0);
    layout->addWidget(m_startLineSpinBox, 4, 1);

    m_stopLineSpinBox = new QSpinBox;
    m_stopLineSpinBox->setWhatsThis(tr("The line number to stop reading the data. If set to 0 the entire file is processed."));
    m_stopLineSpinBox->setValue(m_motion->stopLine());
    m_stopLineSpinBox->setReadOnly(readOnly);

    connect(m_motion, SIGNAL(stopLineChanged(int)),
            m_stopLineSpinBox, SLOT(setValue(int)));
    connect(m_stopLineSpinBox, SIGNAL(valueChanged(int)),
            m_motion, SLOT(setStopLine(int)));

    layout->addWidget(new QLabel(tr("Stop line:")), 4, 2);
    layout->addWidget( m_stopLineSpinBox, 4, 3 );

    m_positionSpinBox = new QSpinBox;
    m_positionSpinBox->setWhatsThis(tr("Current line number"));
    m_positionSpinBox->setReadOnly(true);
    m_positionSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_positionSpinBox->setReadOnly(readOnly);

    layout->addWidget(new QLabel(tr("Current Line:")), 4, 4);
    layout->addWidget( m_positionSpinBox, 4, 5);

    // Text edit
    m_textEdit = new QTextEdit;
    m_textEdit->setWhatsThis(tr("A preview of the file.  Green are header"
                "lines, blue are data lines, and red are extraneous lines."));
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFontFamily("Courier");

    connect(m_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updatePosition()));

    layout->addWidget(m_textEdit, 5, 0, 1, 9);


    QFrame *frame = new QFrame;
    frame->setLayout(layout);

    return frame;
}

QTabWidget* TimeSeriesMotionDialog::createPlotsFrame()
{
    QTabWidget *tabWidget = new QTabWidget;

    // Acceleration time series
    QwtPlot *plot = new QwtPlot;
    plot->setAutoReplot(true);
    QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    QFont font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    QwtText text = QwtText(tr("Time (s)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Acceleration (g)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    m_atsCurve = new QwtPlotCurve;
    m_atsCurve->setPen(QPen(Qt::blue));
    m_atsCurve->attach(plot);

    tabWidget->addTab(plot, tr("Accel. Time Series"));

    // Response spectrum plot
    plot = new QwtPlot;
    plot->setAutoReplot(true);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    plot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());
    font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    text = QwtText(tr("Period (s)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Spectral Accel. (g)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    m_saCurve = new QwtPlotCurve;
    m_saCurve->setPen(QPen(Qt::blue));
    m_saCurve->attach(plot);

    tabWidget->addTab(plot, tr("Response Spectrum"));

    // Fourier amplitude spectrum plot
    plot = new QwtPlot;
    plot->setAutoReplot(true);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                              QwtPicker::CrossRubberBand,
                                              QwtPicker::ActiveOnly, plot->canvas());
    picker->setStateMachine(new QwtPickerDragPointMachine());

    plot->setAxisScaleEngine(QwtPlot::xBottom, logScaleEngine());
    font = QApplication::font();
    plot->setAxisFont(QwtPlot::xBottom, font);

    plot->setAxisScaleEngine(QwtPlot::yLeft, logScaleEngine());
    plot->setAxisFont(QwtPlot::yLeft, font);

    font.setBold(true);
    text = QwtText(tr("Frequency (Hz)"));
    text.setFont(font);
    plot->setAxisTitle(QwtPlot::xBottom, text);

    text.setText(tr("Fourier Amplitude (g-s)"));
    plot->setAxisTitle(QwtPlot::yLeft, text);

    m_fasCurve = new QwtPlotCurve;
    m_fasCurve->setPen(QPen(Qt::blue));
    m_fasCurve->attach(plot);

    tabWidget->addTab(plot, tr("Fourier Amp. Spectrum"));

    return tabWidget;
}

void TimeSeriesMotionDialog::loadPreview(const QString &fileName)
{    
    // Create a preview of the file
    QFile data(fileName);
    if (data.open(QFile::ReadOnly)) {
        QTextStream stream(&data);
        m_textEdit->setText(stream.readAll());

        m_lineCount = m_textEdit->document()->blockCount();
        m_startLineSpinBox->setMaximum(m_lineCount);
        m_stopLineSpinBox->setMaximum(m_lineCount);

        m_positionSpinBox->setRange(0, m_lineCount);
    } else {
        qCritical() << "Error opening file: " << qPrintable(fileName);
        return;
    }
}

void TimeSeriesMotionDialog::plot()
{
    m_atsCurve->setSamples(m_motion->time(), m_motion->timeSeries(TimeSeriesMotion::Acceleration));
    m_saCurve->setSamples(m_motion->respSpec()->period(), m_motion->respSpec()->sa());

    // Don't plot the first point (0 Hz)
    m_fasCurve->setSamples(
            m_motion->freq().data() + 1,
            m_motion->absFourierAcc().data() + 1,
            m_motion->freq().size() - 1);
}

bool TimeSeriesMotionDialog::tryApply()
{
    bool success = true;

    if (!m_motion->isLoaded()) {
        // Need to read the motion from the file        
        success = m_motion->load(m_fileNameLineEdit->text(), false);
    } else {
        // Just apply the new settings
        m_motion->calculate();
    }

    m_tabWidget->setTabEnabled(1, success);

    return success;
}
