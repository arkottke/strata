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

#ifndef CONFIGURE_PLOT_DIALOG_H_
#define CONFIGURE_PLOT_DIALOG_H_

#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include <qwt_plot.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_div.h>

//! A QGroupBox contain the options to customize a plot axis.

class AxisOptions : public QGroupBox
{
    Q_OBJECT

    public:
        AxisOptions( const QString & title, QWidget * parent = 0 );

        /*! Set the default values from the current axis properties.
         *
         * \param scaleEngine the scale engine
         * \param autoScale if the axis is auto-scaled
         * \param scaleDiv the scale division
         */
        void setDefaults( const QwtScaleEngine * scaleEngine, bool autoScale, const QwtScaleDiv * scaleDiv );

        bool linearSpacing() const;

        bool autoScale() const;

        double min() const;
        double max() const;

    private:
        QComboBox * _spacingComboBox;

        QCheckBox * _autoCheckBox;

        QLineEdit * _minLineEdit;
        QLineEdit * _maxLineEdit;
};

//! Dialog used to configure the axes of a QwtPlot.

class ConfigurePlotDialog : public QDialog
{
    Q_OBJECT

    public:
        ConfigurePlotDialog( QwtPlot * plot, QWidget * parent = 0);
       

    protected slots:
        void tryAccept();

    protected:
        void setAxis( int axisid, const AxisOptions * axisOptions );

    private:
        QwtPlot * _plot;

        AxisOptions* _xAxisOptions;
        AxisOptions* _yAxisOptions;
};

#endif
