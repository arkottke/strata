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
#include <QTextBlock>
#include <QTextCursor>
#include <QTextStream>
#include <QWhatsThis>

#include <qwt_picker_machine.h>
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <qwt_text.h>

#include <climits>

TimeSeriesMotionDialog::TimeSeriesMotionDialog(TimeSeriesMotion *motion,
                                               bool readOnly, QWidget *parent,
                                               Qt::WindowFlags f)
    : QDialog(parent, f), _motion(motion) {
  auto *layout = new QVBoxLayout;

  _tabWidget = new QTabWidget;

  _tabWidget->addTab(createInputFrame(readOnly), tr("Input"));
  _tabWidget->addTab(createPlotsFrame(), tr("Plots"));

  // Plot the motion if available
  if (_motion->accel().size()) {
    plot();
  } else {
    _tabWidget->setTabEnabled(1, false);
  };

  layout->addWidget(_tabWidget);

  // Button Row
  QDialogButtonBox *buttonBox = 0;

  if (readOnly) {
    buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
  } else {
    buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel |
            QDialogButtonBox::Apply | QDialogButtonBox::Help,
        Qt::Horizontal, this);
    connect(buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this,
            SLOT(apply()));
  }
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  layout->addWidget(buttonBox);

  setLayout(layout);

  if (!_motion->fileName().isEmpty()) {
    loadPreview(_motion->fileName());
    updateTextEdit();
  }
}

void TimeSeriesMotionDialog::openFile() {
  QSettings settings;
  // Prompt for a fileName
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Select a time series..."),
      settings
          .value("motionDirectory", QStandardPaths::writableLocation(
                                        QStandardPaths::DocumentsLocation))
          .toString());

  if (!fileName.isEmpty()) {
    QFileInfo fileInfo = QFileInfo(fileName);
    settings.setValue("motionDirectory", fileInfo.absoluteFilePath());

    _fileNameLineEdit->setText(fileInfo.absoluteFilePath());
    loadPreview(fileName);

    // Try to load the file
    bool success = _motion->load(fileName, true);
    if (success) {
      updateTextEdit();
      plot();
    } else {
      // Clear input boxes
      _descripLineEdit->clear();
      _pointCountSpinBox->clear();
      _timeStepSpinBox->clear();
    }

    // Update the dialog
    _tabWidget->setTabEnabled(1, success);
  }
}

void TimeSeriesMotionDialog::updateTextEdit() {
  _textEdit->blockSignals(true);
  int startLine = _startLineSpinBox->value() - 1;
  int stopLine = _stopLineSpinBox->value();
  // If a stop line is set correct the line number
  if (stopLine) {
    --stopLine;
  }
  // Original format
  QTextCharFormat format = _textEdit->currentCharFormat();
  // Grab the cursor which is used to process the textedit
  QTextCursor cursor = _textEdit->textCursor();
  // Move the cursor to the start of the document
  cursor.movePosition(QTextCursor::Start);
  //
  // Header lines
  //
  // Move down the old number of header lines and colorize the selection
  cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, startLine);
  // Header format is green
  format.setForeground(QBrush(Qt::darkGreen));
  // Set the color
  cursor.setCharFormat(format);
  //
  // Data lines
  //
  // Reset the anchor
  cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
  // cursor.movePosition( QTextCursor::Up, QTextCursor::MoveAnchor);
  //  Move the cursor down the number of data lines
  if (!stopLine)
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
  else {
    cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,
                        stopLine - startLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
  }
  // Data format is a blue
  format.setForeground(QBrush(Qt::blue));
  // Set the color
  cursor.setCharFormat(format);
  //
  // Extra lines
  //
  if (_stopLineSpinBox->value() && stopLine < _lineCount) {
    // Move the cursor to start of the document
    cursor.movePosition(QTextCursor::Start);
    // Reset the anchor at the beginning of the next line
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor,
                        stopLine + 1);
    // Move the cursor down to the end of the document
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    // Extra format is a red
    format.setForeground(QBrush(Qt::darkRed));
    // Set the color
    cursor.setCharFormat(format);
  }

  _textEdit->blockSignals(false);
}

void TimeSeriesMotionDialog::updatePosition() {
  _positionSpinBox->setValue(_textEdit->textCursor().blockNumber() + 1);
}

void TimeSeriesMotionDialog::updateDataColumn(int index) {
  if (index == 1) {
    _dataColSpinBox->setEnabled(true);
  } else {
    _dataColSpinBox->setEnabled(false);
  }
}

void TimeSeriesMotionDialog::help() { QWhatsThis::enterWhatsThisMode(); }

void TimeSeriesMotionDialog::tryAccept() {
  // FIXME the apply button should disable if it has been clicked once and then
  // re-enable if changes occur
  if (tryApply()) {
    accept();
  }
}

void TimeSeriesMotionDialog::apply() {
  // Clear the plots
  for (auto *curve : {_atsCurve, _saCurve, _fasCurve}) {
    curve->setSamples(QVector<double>(), QVector<double>());
  }

  if (tryApply()) {
    plot();
  }

  updateTextEdit();
}

auto TimeSeriesMotionDialog::createInputFrame(bool readOnly) -> QFrame * {
  auto *layout = new QGridLayout;
  layout->setColumnStretch(8, 1);

  // Create the file row
  auto *pushButton = new QPushButton(tr("&File..."));
  pushButton->setWhatsThis(tr("Opens a dialog to select the file."));
  pushButton->setDisabled(readOnly);
  connect(pushButton, SIGNAL(clicked()), this, SLOT(openFile()));
  layout->addWidget(pushButton, 0, 0);

  _fileNameLineEdit = new QLineEdit;
  _fileNameLineEdit->setWhatsThis(tr("Filename of acceleration time series."));
  _fileNameLineEdit->setReadOnly(true);
  _fileNameLineEdit->setText(_motion->fileName());

  layout->addWidget(_fileNameLineEdit, 0, 1, 1, 8);

  // Create the description row
  _descripLineEdit = new QLineEdit;
  _descripLineEdit->setWhatsThis(
      tr("A description of the time series file (optional)."));
  _descripLineEdit->setText(_motion->description());
  _descripLineEdit->setReadOnly(readOnly);

  connect(_motion, SIGNAL(descriptionChanged(QString)), _descripLineEdit,
          SLOT(setText(QString)));
  connect(_descripLineEdit, SIGNAL(textChanged(QString)), _motion,
          SLOT(setDescription(QString)));

  layout->addWidget(new QLabel(tr("Description:")), 1, 0);
  layout->addWidget(_descripLineEdit, 1, 1, 1, 8);

  // Create the time step, point count, and  scale row
  _pointCountSpinBox = new QSpinBox;
  _pointCountSpinBox->setWhatsThis(
      tr("The number of data points in the time series."));
  _pointCountSpinBox->setRange(0, INT_MAX);
  _pointCountSpinBox->setValue(_motion->pointCount());
  _pointCountSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(pointCountChanged(int)), _pointCountSpinBox,
          SLOT(setValue(int)));
  connect(_pointCountSpinBox, SIGNAL(valueChanged(int)), _motion,
          SLOT(setPointCount(int)));

  layout->addWidget(new QLabel(tr("Point count:")), 2, 0);
  layout->addWidget(_pointCountSpinBox, 2, 1);

  _timeStepSpinBox = new QDoubleSpinBox;
  _timeStepSpinBox->setWhatsThis(tr("The time step between data points."));
  _timeStepSpinBox->setRange(0.0001, 0.05);
  _timeStepSpinBox->setSingleStep(0.001);
  _timeStepSpinBox->setDecimals(4);
  _timeStepSpinBox->setSuffix(" s");
  _timeStepSpinBox->setValue(_motion->timeStep());
  _timeStepSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(timeStepChanged(double)), _timeStepSpinBox,
          SLOT(setValue(double)));
  connect(_timeStepSpinBox, SIGNAL(valueChanged(double)), _motion,
          SLOT(setTimeStep(double)));

  layout->addWidget(new QLabel(tr("Time step:")), 2, 2);
  layout->addWidget(_timeStepSpinBox, 2, 3);

  _scaleSpinBox = new QDoubleSpinBox;
  _scaleSpinBox->setWhatsThis(
      tr("The scale factor that is applied to the motion."));
  _scaleSpinBox->setRange(0.001, 20);
  _scaleSpinBox->setDecimals(4);
  _scaleSpinBox->setSingleStep(0.01);
  _scaleSpinBox->setValue(_motion->scale());
  _scaleSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(scaleChanged(double)), _scaleSpinBox,
          SLOT(setValue(double)));
  connect(_scaleSpinBox, SIGNAL(valueChanged(double)), _motion,
          SLOT(setScale(double)));

  layout->addWidget(new QLabel(tr("Scale factor:")), 2, 6);
  layout->addWidget(_scaleSpinBox, 2, 7);

  // Data format row
  _formatComboBox = new QComboBox;
  _formatComboBox->setWhatsThis(
      tr("The format of the data file. <i>Rows</i> "
         "reads every number on every row.  <i>Columns</i> reads data "
         "from a specific column."));
  _formatComboBox->addItems(TimeSeriesMotion::formatList());
  _formatComboBox->setCurrentIndex(_motion->format());
  _formatComboBox->setEnabled(!readOnly);

  connect(_formatComboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT(updateDataColumn(int)));
  connect(_motion, SIGNAL(formatChanged(int)), _formatComboBox,
          SLOT(setCurrentIndex(int)));
  connect(_formatComboBox, SIGNAL(currentIndexChanged(int)), _motion,
          SLOT(setFormat(int)));

  layout->addWidget(new QLabel(tr("Format:")), 3, 0);
  layout->addWidget(_formatComboBox, 3, 1);

  _dataColSpinBox = new QSpinBox;
  _dataColSpinBox->setWhatsThis(tr("The column of the acceleration data."));
  _dataColSpinBox->setRange(1, 100);
  _dataColSpinBox->setEnabled(_motion->format() == TimeSeriesMotion::Columns);
  _dataColSpinBox->setValue(_motion->dataColumn());
  _dataColSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(dataColumnChanged(int)), _dataColSpinBox,
          SLOT(setValue(int)));
  connect(_dataColSpinBox, SIGNAL(valueChanged(int)), _motion,
          SLOT(setDataColumn(int)));

  layout->addWidget(new QLabel(tr("Data column:")), 3, 2);
  layout->addWidget(_dataColSpinBox, 3, 3);

  // Units specification
  _unitsComboBox = new QComboBox;
  _unitsComboBox->setWhatsThis(tr("Units of the time series."));
  _unitsComboBox->addItems(TimeSeriesMotion::inputUnitsList());
  _unitsComboBox->setCurrentIndex(_motion->inputUnits());
  connect(_motion, SIGNAL(inputUnitsChanged(int)), _unitsComboBox,
          SLOT(setCurrentIndex(int)));
  connect(_unitsComboBox, SIGNAL(currentIndexChanged(int)), _motion,
          SLOT(setInputUnits(int)));

  layout->addWidget(new QLabel(tr("Units:")), 3, 4);
  layout->addWidget(_unitsComboBox, 3, 5);

  // PGA of the motion
  _pgaSpinBox = new QDoubleSpinBox;
  _pgaSpinBox->setWhatsThis(tr("The peak ground acceleration of the motion."));
  _pgaSpinBox->setRange(0.001, 10.0);
  _pgaSpinBox->setDecimals(3);
  _pgaSpinBox->setSingleStep(0.01);
  _pgaSpinBox->setSuffix(" g");
  _pgaSpinBox->setReadOnly(true);
  _pgaSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
  _pgaSpinBox->setValue(_motion->pga());

  connect(_motion, SIGNAL(pgaChanged(double)), _pgaSpinBox,
          SLOT(setValue(double)));

  layout->addWidget(new QLabel(tr("PGA:")), 3, 6);
  layout->addWidget(_pgaSpinBox, 3, 7);

  // Lines row
  _startLineSpinBox = new QSpinBox;
  _startLineSpinBox->setRange(1, INT_MAX);
  _startLineSpinBox->setWhatsThis(
      tr("The line number at which to start reading the data"));
  _startLineSpinBox->setValue(_motion->startLine());
  _startLineSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(startLineChanged(int)), _startLineSpinBox,
          SLOT(setValue(int)));
  connect(_startLineSpinBox, SIGNAL(valueChanged(int)), _motion,
          SLOT(setStartLine(int)));

  layout->addWidget(new QLabel(tr("Start line:")), 4, 0);
  layout->addWidget(_startLineSpinBox, 4, 1);

  _stopLineSpinBox = new QSpinBox;
  _stopLineSpinBox->setRange(0, INT_MAX);
  _stopLineSpinBox->setWhatsThis(
      tr("The line number to stop reading the data. If set to 0 the entire "
         "file is processed."));
  _stopLineSpinBox->setValue(_motion->stopLine());
  _stopLineSpinBox->setReadOnly(readOnly);

  connect(_motion, SIGNAL(stopLineChanged(int)), _stopLineSpinBox,
          SLOT(setValue(int)));
  connect(_stopLineSpinBox, SIGNAL(valueChanged(int)), _motion,
          SLOT(setStopLine(int)));

  layout->addWidget(new QLabel(tr("Stop line:")), 4, 2);
  layout->addWidget(_stopLineSpinBox, 4, 3);

  _positionSpinBox = new QSpinBox;
  _positionSpinBox->setWhatsThis(tr("Current line number"));
  _positionSpinBox->setReadOnly(true);
  _positionSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
  _positionSpinBox->setReadOnly(readOnly);

  layout->addWidget(new QLabel(tr("Current Line:")), 4, 4);
  layout->addWidget(_positionSpinBox, 4, 5);

  // Text edit
  _textEdit = new QTextEdit;
  _textEdit->setWhatsThis(
      tr("A preview of the file.  Green are header"
         "lines, blue are data lines, and red are extraneous lines."));
  _textEdit->setLineWrapMode(QTextEdit::NoWrap);
  _textEdit->setReadOnly(true);
  _textEdit->setFontFamily("Courier");

  connect(_textEdit, SIGNAL(cursorPositionChanged()), this,
          SLOT(updatePosition()));

  layout->addWidget(_textEdit, 5, 0, 1, 9);

  auto *frame = new QFrame;
  frame->setLayout(layout);

  return frame;
}

auto TimeSeriesMotionDialog::createPlotsFrame() -> QTabWidget * {
  auto *tabWidget = new QTabWidget;

  // Acceleration time series
  auto *plot = new QwtPlot;
  plot->setAutoReplot(true);
  auto *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
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

  _atsCurve = new QwtPlotCurve;
  _atsCurve->setPen(QPen(Qt::blue));
  _atsCurve->attach(plot);

  tabWidget->addTab(plot, tr("Accel. Time Series"));

  // Response spectrum plot
  plot = new QwtPlot;
  plot->setAutoReplot(true);
  picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                             QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                             plot->canvas());
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

  _saCurve = new QwtPlotCurve;
  _saCurve->setPen(QPen(Qt::blue));
  _saCurve->attach(plot);

  tabWidget->addTab(plot, tr("Response Spectrum"));

  // Fourier amplitude spectrum plot
  plot = new QwtPlot;
  plot->setAutoReplot(true);
  picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                             QwtPicker::CrossRubberBand, QwtPicker::ActiveOnly,
                             plot->canvas());
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

  _fasCurve = new QwtPlotCurve;
  _fasCurve->setPen(QPen(Qt::blue));
  _fasCurve->attach(plot);

  tabWidget->addTab(plot, tr("Fourier Amp. Spectrum"));

  return tabWidget;
}

void TimeSeriesMotionDialog::loadPreview(const QString &fileName) {
  // Create a preview of the file
  QFile data(fileName);
  if (data.open(QFile::ReadOnly)) {
    QTextStream stream(&data);
    _textEdit->setText(stream.readAll());

    _lineCount = _textEdit->document()->blockCount();
    _startLineSpinBox->setMaximum(_lineCount);
    _stopLineSpinBox->setMaximum(_lineCount);

    _positionSpinBox->setRange(0, _lineCount);
  } else {
    qCritical() << "Error opening file: " << qPrintable(fileName);
    return;
  }
}

void TimeSeriesMotionDialog::plot() {
  _atsCurve->setSamples(_motion->time(),
                        _motion->timeSeries(TimeSeriesMotion::Acceleration));
  _saCurve->setSamples(_motion->respSpec()->period(),
                       _motion->respSpec()->sa());

  // Don't plot the first point (0 Hz)
  _fasCurve->setSamples(_motion->freq().data() + 1,
                        _motion->absFourierAcc().data() + 1,
                        _motion->freq().size() - 1);
}

auto TimeSeriesMotionDialog::tryApply() -> bool {
  bool success = true;

  if (!_motion->isLoaded()) {
    // Need to read the motion from the file
    success = _motion->load(_fileNameLineEdit->text(), false);
  } else {
    // Just apply the new settings
    _motion->calculate();
  }

  _tabWidget->setTabEnabled(1, success);

  return success;
}
