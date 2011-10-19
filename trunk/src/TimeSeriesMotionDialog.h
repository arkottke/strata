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

#ifndef TIME_SERIES_MOTION_DIALOG_H_
#define TIME_SERIES_MOTION_DIALOG_H_

#include <QComboBox>
#include <QDialog>
#include <QFrame>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>

#include <qwt_plot_curve.h>

class TimeSeriesMotion;

class TimeSeriesMotionDialog : public QDialog
{
    Q_OBJECT

public:
    TimeSeriesMotionDialog(TimeSeriesMotion* motion, bool readOnly, QWidget * parent=0, Qt::WindowFlags f = 0);

signals:
    void lineCountChanged(int lineCount);

public slots:
    void openFile();

    void updateTextEdit();
    void updatePosition();
    void updateDataColumn(int);

    void help();

    void tryAccept();

    void apply();

private:
    //! Create the frame for defining the parameters of the motion
    QFrame* createInputFrame(bool readOnly);

    //! Create the frame of plots of the data
    QTabWidget* createPlotsFrame();

    //! Read the motion data from the file and update the text edit
    void loadPreview(const QString &fileName);

    //! Plot the data
    void plot();

    //! Try to apply the settings to the motion
    bool tryApply();

    TimeSeriesMotion * m_motion;

    QTabWidget *m_tabWidget;

    QLineEdit *m_fileNameLineEdit;
    QLineEdit *m_descripLineEdit;

    QSpinBox *m_pointCountSpinBox;
    QDoubleSpinBox *m_timeStepSpinBox;
    QDoubleSpinBox *m_scaleSpinBox;

    QComboBox *m_formatComboBox;
    QSpinBox *m_dataColSpinBox;

    QComboBox *m_unitsComboBox;
    QDoubleSpinBox *m_pgaSpinBox;

    QSpinBox *m_startLineSpinBox;
    QSpinBox *m_stopLineSpinBox;

    QSpinBox *m_positionSpinBox;
    QTextEdit *m_textEdit;

    QwtPlotCurve *m_atsCurve;
    QwtPlotCurve *m_saCurve;
    QwtPlotCurve *m_fasCurve;

    //! Total number of lines in the file
    int m_lineCount;
};
#endif
