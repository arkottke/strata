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

#include "RecordedMotionDialog.h"

#include <QFileInfo>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QTextCursor>
#include <QTextBlock>
#include <QFileDialog>
#include <QTextStream>
#include <QWhatsThis>
#include <QDebug>
#include <QSettings>
#include <qwt_plot_curve.h>


RecordedMotionDialog::RecordedMotionDialog(RecordedMotion * motion, QString & workingDir, QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f), m_motion(motion), m_workingDir(workingDir)
{
    setup();
}

void RecordedMotionDialog::setup()
{
    int space = 20;
    // Create the file row
    QPushButton * filePushButton = new QPushButton(tr("&File..."));
    filePushButton->setWhatsThis(tr("Opens a dialog to select the file."));

    m_fileLineEdit = new QLineEdit;
    m_fileLineEdit->setWhatsThis(tr("Currently selected file"));
    m_fileLineEdit->setReadOnly(true);

    QHBoxLayout * row_1 = new QHBoxLayout;
    row_1->addWidget(filePushButton);
    row_1->addWidget(m_fileLineEdit);

    // Create the description row
    m_descripLineEdit = new QLineEdit;
    m_descripLineEdit->setWhatsThis(tr("A description of the time series file (optional)."));
    QHBoxLayout * row_2 = new QHBoxLayout;
    row_2->addWidget(new QLabel(tr("Description:")));
    row_2->addWidget(m_descripLineEdit);

    // Create the time step, point count, and scale row
    m_timeStepLineEdit = new QLineEdit;
    m_timeStepLineEdit->setWhatsThis(tr("The time step between data points."));
    m_timeStepLineEdit->setValidator(new QDoubleValidator(m_timeStepLineEdit));
    
    m_countLineEdit = new QLineEdit;
    m_countLineEdit->setWhatsThis(tr("The number of data points in the file."));
    m_countLineEdit->setValidator(new QIntValidator(m_countLineEdit));
   
    m_scaleLineEdit = new QLineEdit;
    m_scaleLineEdit->setWhatsThis(tr("The scale factor that is applied to the motion."));
    m_scaleLineEdit->setValidator(new QDoubleValidator(m_scaleLineEdit));
    m_scaleLineEdit->setText("1");

    QHBoxLayout * row_3  = new QHBoxLayout;
    row_3->addWidget(new QLabel(tr("Number of points:")));
    row_3->addWidget(m_countLineEdit);
    row_3->addSpacing(space);
    row_3->addWidget(new QLabel(tr("Time step (s):")));
    row_3->addWidget(m_timeStepLineEdit);
    row_3->addSpacing(space);
    row_3->addWidget(new QLabel(tr("Scale factor:")));
    row_3->addWidget(m_scaleLineEdit);
    row_3->addStretch(1);

    // Data format row
    m_formatComboBox = new QComboBox;
    m_formatComboBox->setWhatsThis(tr("The format of the data file. <i>Rows</i> "
                "reads every number on every row.  <i>Columns</i> reads data "
                "from a specific column."));
    m_formatComboBox->addItems(RecordedMotion::formatList());
    
    m_dataColumnSpinBox = new QSpinBox;
    m_dataColumnSpinBox->setWhatsThis(tr("The column of the acceleration data."));
    m_dataColumnSpinBox->setEnabled(false); 
    m_dataColumnSpinBox->setMinimum(1);
    m_dataColumnSpinBox->setValue(1);

    QHBoxLayout * row_4 = new QHBoxLayout;
    row_4->addWidget(new QLabel(tr("Format:")));
    row_4->addWidget(m_formatComboBox);
    row_4->addSpacing(space);
    row_4->addWidget(new QLabel(tr("Data column:")));
    row_4->addWidget(m_dataColumnSpinBox);
    row_4->addSpacing(space);
    row_4->addStretch(1);

    // Lines row
    m_startLineSpinBox = new QSpinBox;
    m_startLineSpinBox->setMinimum(1);
    m_startLineSpinBox->setWhatsThis(tr("The line number at which to start reading the data"));

    m_stopLineSpinBox = new QSpinBox;
    m_stopLineSpinBox->setWhatsThis(tr("The line number to stop reading the data. If set to 0 the entire file is processed."));

    m_positionLineEdit = new QLineEdit;
    m_positionLineEdit->setFixedWidth(40);
    m_positionLineEdit->setWhatsThis(tr("Current line number"));

    QPushButton * refreshButton = new QPushButton(tr("Refresh"));
    refreshButton->setWhatsThis(tr("Refresh the colorization of the file preview."));

    QHBoxLayout * row_5 = new QHBoxLayout;
    row_5->addWidget(new QLabel(tr("Data:   Start line:")));
    row_5->addWidget(m_startLineSpinBox);
    row_5->addSpacing(space);
    row_5->addWidget(new QLabel(tr("Stop line:")));
    row_5->addWidget(m_stopLineSpinBox);
    row_5->addSpacing(space);
    row_5->addWidget(new QLabel(tr("Current Line:")));
    row_5->addWidget(m_positionLineEdit);
    row_5->addStretch(1);
    row_5->addWidget(refreshButton);

    // Text edit
    m_textEdit = new QTextEdit;
    m_textEdit->setWhatsThis(tr("A preview of the file.  Green are header"
                "lines, blue are data lines, and red are extraneous lines."));
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFontFamily("Courier New");

    // Button Row
    QPushButton * helpPushButton = new QPushButton(tr("&Help"));
    QPushButton * plotPushButton = new QPushButton(tr("&Plot"));
    QPushButton * cancelPushButton = new QPushButton(tr("&Cancel"));
    QPushButton * okayPushButton = new QPushButton(tr("&Ok"));
    okayPushButton->setDefault(true);

    QHBoxLayout * buttonRow = new QHBoxLayout;
    buttonRow->addWidget(helpPushButton);
    buttonRow->addStretch(1);
    buttonRow->addWidget(plotPushButton);
    buttonRow->addSpacing(space);
    buttonRow->addWidget(cancelPushButton);
    buttonRow->addWidget(okayPushButton);

    // For the layout
    QVBoxLayout * layout = new QVBoxLayout;

    layout->addItem(row_1);
    layout->addItem(row_2);
    layout->addItem(row_3);
    layout->addItem(row_4);
    layout->addItem(row_5);
    layout->addWidget(m_textEdit);
    layout->addItem(buttonRow);
    setLayout(layout);

    // Connections
    connect(filePushButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateTextEdit()));
   
    connect(m_formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDataColumn(int)));

    connect(m_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updatePosition()));

    connect(helpPushButton, SIGNAL(clicked()), this, SLOT(help()));
    connect(plotPushButton, SIGNAL(clicked()), this, SLOT(plot()));
    connect(okayPushButton, SIGNAL(clicked()), this, SLOT(tryAccept()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void RecordedMotionDialog::openFile()
{
    QSettings settings;
    // Prompt for a fileName
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);

    if (settings.contains("mainwindow-motion-dialog"))
        dialog.restoreState( settings.value("mainwindow-motion-dialog").toByteArray() );

    if (dialog.exec()) {
        QString fileName = dialog.selectedFiles().at(0);
        // Save the state
        settings.setValue( "mainwindow-motion-dialog", dialog.saveState() );

        // Set the starting line for AT2 files if the start line hasn't been changed.
        if (fileName.endsWith(".AT2", Qt::CaseInsensitive) && m_startLineSpinBox->value() == 1)
            m_startLineSpinBox->setValue(5);

        // Save the working directory
        QFileInfo fileInfo = QFileInfo(fileName);
        m_workingDir = fileInfo.filePath();

        // Read the file
        QFile data(fileName);

        if (data.open(QFile::ReadOnly)) {
            QTextStream stream(&data);
            m_textEdit->setText(stream.readAll());
        } else {
            qCritical("Error opening file: %s", qPrintable(fileName));
            return;
        }
    
        // Update the text field
        m_fileLineEdit->setText(fileName);
        // Count the total number of lines 
        m_totalLines = m_textEdit->document()->blockCount();
        // Set the maximum values of the lines spinbox
        m_startLineSpinBox->setMaximum(m_totalLines);
        m_stopLineSpinBox->setMaximum(m_totalLines);
    }
}

void RecordedMotionDialog::updateTextEdit()
{
    int startLine = m_startLineSpinBox->value() - 1;
    int stopLine = m_stopLineSpinBox->value();
    // If a stop line is set correct the line number 
    if (stopLine) --stopLine;
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
    if ( m_stopLineSpinBox->value() && stopLine < m_totalLines) {
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
}

void RecordedMotionDialog::updatePosition()
{
    m_positionLineEdit->setText(QString::number(m_textEdit->textCursor().blockNumber()+1));
}

void RecordedMotionDialog::updateDataColumn(int index)
{
    if ( index == 1 )
        m_dataColumnSpinBox->setEnabled(true);
    else
        m_dataColumnSpinBox->setEnabled(false);
}

void RecordedMotionDialog::help()
{
    QWhatsThis::enterWhatsThisMode();
}

void RecordedMotionDialog::plot()
{
    // Close the plot if it exists
    if (!m_plot.isNull())
        delete m_plot;

    // Clear old acceleration
    m_motion->clear();
    // Save the values to m_motion
    save();

    if (m_motion->load() && m_plot.isNull())
    {
        m_plot = new QwtPlot;
        m_plot->setWindowModality(Qt::WindowModal);
        m_plot->setAttribute(Qt::WA_DeleteOnClose, true);
        m_plot->setAxisTitle( QwtPlot::xBottom, tr("Time (s)"));
        m_plot->setAxisTitle( QwtPlot::yLeft, tr("Acceleration (g)"));
        m_plot->resize( 800, 300 );
        
        QwtPlotCurve * curve = new QwtPlotCurve;
        curve->setPen(QPen(Qt::red));
        curve->setData( m_motion->time().data(), m_motion->accel().data(), m_motion->accel().size() );
        curve->attach(m_plot);

        // finally, refresh the plot
        m_plot->replot();
        // Show the plot
        m_plot->show();
    } 
}

void RecordedMotionDialog::tryAccept()
{
    m_motion->clear();
    // Save the values to m_motion
    save();
    // Attempt to load m_motion
    if(m_motion->load())
        accept();
    // Close the plot
    if(m_plot)
        m_plot->close();
}

void RecordedMotionDialog::save()
{
    m_motion->setFileName(m_fileLineEdit->text());
    m_motion->setDescription(m_descripLineEdit->text());

    m_motion->setTimeStep(m_timeStepLineEdit->text().toDouble());
    m_motion->setPointCount(m_countLineEdit->text().toInt());
    m_motion->setScale(m_scaleLineEdit->text().toDouble());
    
    m_motion->setFormat((RecordedMotion::Format)m_formatComboBox->currentIndex());
    m_motion->setDataColumn(m_dataColumnSpinBox->value());

    m_motion->setStartLine(m_startLineSpinBox->value()-1);
    m_motion->setStopLine(m_stopLineSpinBox->value()-1);
}
