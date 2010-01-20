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

#include <QDebug>
#if QT_VERSION >= 0x040400
    #include <QDesktopServices>
#endif
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextStream>
#include <QWhatsThis>

#include <qwt_plot_curve.h>


RecordedMotionDialog::RecordedMotionDialog(RecordedMotion * motion, QString & workingDir, QWidget *parent, Qt::WindowFlags f)
        : QDialog(parent, f), m_motion(motion)
{
    setup();
}

void RecordedMotionDialog::setup()
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 6, 1 );

    // Create the file row
    QPushButton * filePushButton = new QPushButton(tr("&File..."));
    filePushButton->setWhatsThis(tr("Opens a dialog to select the file."));
    connect(filePushButton, SIGNAL(clicked()), this, SLOT(openFile()));
    layout->addWidget( filePushButton, 0, 0 );

    m_fileLineEdit = new QLineEdit;
    m_fileLineEdit->setWhatsThis(tr("Currently selected file"));
    m_fileLineEdit->setReadOnly(true);
    layout->addWidget( m_fileLineEdit, 0, 1, 1, 8);

    // Create the description row
    m_descripLineEdit = new QLineEdit;
    m_descripLineEdit->setWhatsThis(tr("A description of the time series file (optional)."));
    layout->addWidget(new QLabel(tr("Description:")), 1, 0); 
    layout->addWidget(m_descripLineEdit, 1, 1, 1, 8);

    // Create the time step, point count, and scale row
    m_countLineEdit = new QLineEdit;
    m_countLineEdit->setWhatsThis(tr("The number of data points in the file."));
    m_countLineEdit->setValidator(new QIntValidator(m_countLineEdit));
    layout->addWidget(new QLabel(tr("Number of points:")), 2, 0);
    layout->addWidget( m_countLineEdit, 2, 1 );


    m_timeStepLineEdit = new QLineEdit;
    m_timeStepLineEdit->setWhatsThis(tr("The time step between data points."));
    m_timeStepLineEdit->setValidator(new QDoubleValidator(m_timeStepLineEdit));
    layout->addWidget(new QLabel(tr("Time step (s):")), 2, 2);
    layout->addWidget(m_timeStepLineEdit, 2, 3 );
   
    m_scaleLineEdit = new QLineEdit;
    m_scaleLineEdit->setWhatsThis(tr("The scale factor that is applied to the motion."));
    m_scaleLineEdit->setValidator(new QDoubleValidator(m_scaleLineEdit));
    m_scaleLineEdit->setText("1");
    
    layout->addWidget(new QLabel(tr("Scale factor:")), 2, 4);
    layout->addWidget(m_scaleLineEdit, 2, 5 );

    // Data format row
    m_formatComboBox = new QComboBox;
    m_formatComboBox->setWhatsThis(tr("The format of the data file. <i>Rows</i> "
                "reads every number on every row.  <i>Columns</i> reads data "
                "from a specific column."));
    m_formatComboBox->addItems(RecordedMotion::formatList());
    connect(m_formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDataColumn(int)));

    layout->addWidget( new QLabel(tr("Format:")), 3, 0);
    layout->addWidget(m_formatComboBox, 3, 1);
    
    m_dataColumnSpinBox = new QSpinBox;
    m_dataColumnSpinBox->setWhatsThis(tr("The column of the acceleration data."));
    m_dataColumnSpinBox->setEnabled(false); 
    m_dataColumnSpinBox->setMinimum(1);
    m_dataColumnSpinBox->setValue(1);

    layout->addWidget(new QLabel(tr("Data column:")), 3, 2);
    layout->addWidget(m_dataColumnSpinBox, 3, 3);

    // Lines row
    m_startLineSpinBox = new QSpinBox;
    m_startLineSpinBox->setMinimum(1);
    m_startLineSpinBox->setWhatsThis(tr("The line number at which to start reading the data"));
    layout->addWidget(new QLabel(tr("Data:   Start line:")), 4, 0);
    layout->addWidget(m_startLineSpinBox, 4, 1);

    m_stopLineSpinBox = new QSpinBox;
    m_stopLineSpinBox->setWhatsThis(tr("The line number to stop reading the data. If set to 0 the entire file is processed."));
    layout->addWidget(new QLabel(tr("Stop line:")), 4, 2);
    layout->addWidget( m_stopLineSpinBox, 4, 3 );

    m_positionLineEdit = new QLineEdit;
    m_positionLineEdit->setFixedWidth(40);
    m_positionLineEdit->setWhatsThis(tr("Current line number"));
    layout->addWidget(new QLabel(tr("Current Line:")), 4, 4);
    layout->addWidget( m_positionLineEdit, 4, 5);

    // Text edit
    m_textEdit = new QTextEdit;
    m_textEdit->setWhatsThis(tr("A preview of the file.  Green are header"
                "lines, blue are data lines, and red are extraneous lines."));
    m_textEdit->setLineWrapMode(QTextEdit::NoWrap);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFontFamily("Courier New");
    connect(m_textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(updatePosition()));

    layout->addWidget( m_textEdit, 5, 0, 1, 7 );

    // Button Row
    QDialogButtonBox * buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this);
    connect( buttonBox, SIGNAL(helpRequested()), this, SLOT(help()));
    connect( buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QPushButton * plotPushButton = new QPushButton(tr("&Plot"));
    plotPushButton->setWhatsThis(tr("Plot the acceleration time series."));
    connect(plotPushButton, SIGNAL(clicked()), this, SLOT(plot()));
    buttonBox->addButton( plotPushButton, QDialogButtonBox::ActionRole );
    
    QPushButton * refreshButton = new QPushButton(tr("Refresh"));
    refreshButton->setWhatsThis(tr("Refresh the colorization of the file preview."));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(updateTextEdit()));
    buttonBox->addButton( refreshButton, QDialogButtonBox::ActionRole );

    layout->addWidget( buttonBox, 6, 0, 1, 7 );

    setLayout(layout);
}

void RecordedMotionDialog::openFile()
{
    QSettings settings;
    // Prompt for a fileName
    QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Select time series..."),
            settings.value("motionDirectory",
                           QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString());

    if (!fileName.isEmpty()) {
        QFileInfo fileInfo = QFileInfo(fileName);
        settings.setValue("motionDirectory", fileInfo.filePath());

        // Read the file
        QFile data(fileName);

        if (data.open(QFile::ReadOnly)) {
            QTextStream stream(&data);
            m_textEdit->setText(stream.readAll());
        } else {
            qCritical("Error opening file: %s", qPrintable(fileName));
            return;
        }
        
        // Set the starting line for AT2 files if the start line hasn't been changed.
        if (fileName.endsWith(".AT2", Qt::CaseInsensitive)){
            QTextCursor cursor(m_textEdit->document());
            // Second line has the properties
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 1);
            m_descripLineEdit->setText(cursor.block().text());
           
            // 4th line has the time step and point count
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 2);
            QStringList parts = cursor.block().text().split(QRegExp("\\s+"));

            m_countLineEdit->setText(parts.at(0));
            m_timeStepLineEdit->setText(parts.at(1));

            // Start line should be 5
            m_startLineSpinBox->setValue(5);

            // Color the text edit
            updateTextEdit();
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
