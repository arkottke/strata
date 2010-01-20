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

#ifndef RECORDED_MOTION_DIALOG_H_
#define RECORDED_MOTION_DIALOG_H_

#include "RecordedMotion.h"
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QTextDocument>
#include <QPointer>
#include <qwt_plot.h>

class RecordedMotionDialog : public QDialog
{
    Q_OBJECT

public:
    RecordedMotionDialog(RecordedMotion * motion, QString & workingDir, QWidget * parent=0, Qt::WindowFlags f = 0);

public slots:
    void openFile();
    void updateTextEdit();
    void updatePosition();
    void updateDataColumn(int);
    void plot();

    void help();

    void tryAccept();

private:
    RecordedMotion * m_motion;

    QLineEdit * m_fileLineEdit;
    QLineEdit * m_descripLineEdit;
    QLineEdit * m_timeStepLineEdit;
    QLineEdit * m_countLineEdit;
    QLineEdit * m_scaleLineEdit;

    QComboBox * m_formatComboBox;
    QSpinBox * m_dataColumnSpinBox;

    QSpinBox * m_startLineSpinBox;
    QSpinBox * m_stopLineSpinBox;

    QLineEdit * m_positionLineEdit;
    QTextEdit * m_textEdit;

    QPointer<QwtPlot> m_plot;

    int m_totalLines;

    // Setup the dialog
    void setup();

    // Save the values to the model
    void save();
};
#endif
