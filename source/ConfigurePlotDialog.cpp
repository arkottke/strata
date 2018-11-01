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

#include "ConfigurePlotDialog.h"

#include "MyQwtCompatibility.h"

#include <QGridLayout>
#include <QLabel>
#include <QDoubleValidator>
#include <QDialogButtonBox>

#include <QDebug>

AxisOptions::AxisOptions( const QString & title, QWidget * parent )
    : QGroupBox( title, parent )
{
    QGridLayout * layout = new QGridLayout;

    // Spacing combo box
    layout->addWidget( new QLabel(tr("Spacing:")), 0, 0);

    _spacingComboBox = new QComboBox;
    _spacingComboBox->addItem(tr("Linear"));
    _spacingComboBox->addItem(tr("Log10"));

    layout->addWidget( _spacingComboBox, 0, 1);

    // Automatic scaling of axis
    _autoCheckBox = new QCheckBox(tr("Automatic scaling of axis"));

    layout->addWidget( _autoCheckBox, 1, 0);

    // Minimum value
    layout->addWidget( new QLabel(tr("Minimum value:")), 2, 0);

    _minLineEdit = new QLineEdit;
    _minLineEdit->setValidator( new QDoubleValidator(_minLineEdit));

    layout->addWidget( _minLineEdit, 2, 1 );

    // Maximum value
    layout->addWidget( new QLabel(tr("Maximum value:")), 3, 0);

    _maxLineEdit = new QLineEdit;
    _maxLineEdit->setValidator( new QDoubleValidator(_maxLineEdit));

    layout->addWidget( _maxLineEdit, 3, 1 );

    connect( _autoCheckBox, SIGNAL(toggled(bool)), _minLineEdit, SLOT(setDisabled(bool)));
    connect( _autoCheckBox, SIGNAL(toggled(bool)), _maxLineEdit, SLOT(setDisabled(bool)));

    setLayout(layout);
}

void AxisOptions::setDefaults( const QwtScaleEngine * scaleEngine, bool autoScale, const QwtScaleDiv * scaleDiv )
{
    if ( dynamic_cast<const QwtLinearScaleEngine*>(scaleEngine) )
        _spacingComboBox->setCurrentIndex(0);
    else
        _spacingComboBox->setCurrentIndex(1);

    if ( autoScale )
        _autoCheckBox->setChecked(true);
    else
        _autoCheckBox->setChecked(false);

    // Get lower and upper bounds
#if QWT_VERSION < 0x050200
    _minLineEdit->setText(QString::number(scaleDiv->lBound()));
    _maxLineEdit->setText(QString::number(scaleDiv->hBound()));
#else
    _minLineEdit->setText(QString::number(scaleDiv->lowerBound()));
    _maxLineEdit->setText(QString::number(scaleDiv->upperBound()));
#endif
}

bool AxisOptions::linearSpacing() const
{
    if ( _spacingComboBox->currentIndex() == 0 )
        return true;
    else 
        return false;
}

bool AxisOptions::autoScale() const
{
    return _autoCheckBox->isChecked();
}

double AxisOptions::min() const
{
    return _minLineEdit->text().toDouble();
}

double AxisOptions::max() const
{
    return _maxLineEdit->text().toDouble();
}

ConfigurePlotDialog::ConfigurePlotDialog( QwtPlot * plot, QWidget * parent)
    : QDialog(parent), _plot(plot)
{
    // Create the dialog
    QGridLayout * layout = new QGridLayout;

    _xAxisOptions = new AxisOptions(tr("X Axis") );
    _xAxisOptions->setDefaults( 
            _plot->axisScaleEngine( QwtPlot::xBottom ), 
            _plot->axisAutoScale( QwtPlot::xBottom ), 
#if QWT_VERSION < 0x060100
            _plot->axisScaleDiv( QwtPlot::xBottom )
#else
            &(_plot->axisScaleDiv( QwtPlot::xBottom ))
#endif
            );

   
    layout->addWidget( _xAxisOptions, 0, 0 );

    _yAxisOptions = new AxisOptions(tr("Y Axis") );
    _yAxisOptions->setDefaults( 
            _plot->axisScaleEngine( QwtPlot::yLeft ), 
            _plot->axisAutoScale( QwtPlot::yLeft ), 
#if QWT_VERSION < 0x060100
            _plot->axisScaleDiv( QwtPlot::yLeft )
#else
            &(_plot->axisScaleDiv( QwtPlot::yLeft ))
#endif
            );
    
    layout->addWidget( _yAxisOptions, 1, 0 );
    
    // Add the buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox( 
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect( buttonBox, SIGNAL(accepted()), this, SLOT(tryAccept()));
    connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget( buttonBox, 2, 0 );

    setLayout(layout);
}

void ConfigurePlotDialog::tryAccept()
{
    setAxis( QwtPlot::xBottom, _xAxisOptions );
    setAxis( QwtPlot::yLeft, _yAxisOptions );
    
    accept();
}

void ConfigurePlotDialog::setAxis( int axisid, const AxisOptions * axisOptions )
{
    QwtScaleEngine * oldEngine = _plot->axisScaleEngine(axisid);
    QwtScaleEngine * newEngine;

    if ( axisOptions->linearSpacing() )
        newEngine = new QwtLinearScaleEngine;
    else
        newEngine = logScaleEngine();

    newEngine->setAttributes( oldEngine->attributes() );

    // Deletes the old engine
    _plot->setAxisScaleEngine( axisid, newEngine );

    // Set the axis range
    if ( axisOptions->autoScale() )
        _plot->setAxisAutoScale(axisid);
    else
        _plot->setAxisScale( axisid, axisOptions->min(), axisOptions->max() );
}
