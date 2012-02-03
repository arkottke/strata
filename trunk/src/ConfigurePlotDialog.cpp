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

#include "ConfigurePlotDialog.h"

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

    m_spacingComboBox = new QComboBox;
    m_spacingComboBox->addItem(tr("Linear"));
    m_spacingComboBox->addItem(tr("Log10"));

    layout->addWidget( m_spacingComboBox, 0, 1);

    // Automatic scaling of axis
    m_autoCheckBox = new QCheckBox(tr("Automatic scaling of axis"));

    layout->addWidget( m_autoCheckBox, 1, 0);

    // Minimum value
    layout->addWidget( new QLabel(tr("Minimum value:")), 2, 0);

    m_minLineEdit = new QLineEdit;
    m_minLineEdit->setValidator( new QDoubleValidator(m_minLineEdit));

    layout->addWidget( m_minLineEdit, 2, 1 );

    // Maximum value
    layout->addWidget( new QLabel(tr("Maximum value:")), 3, 0);

    m_maxLineEdit = new QLineEdit;
    m_maxLineEdit->setValidator( new QDoubleValidator(m_maxLineEdit));

    layout->addWidget( m_maxLineEdit, 3, 1 );

    connect( m_autoCheckBox, SIGNAL(toggled(bool)), m_minLineEdit, SLOT(setDisabled(bool)));
    connect( m_autoCheckBox, SIGNAL(toggled(bool)), m_maxLineEdit, SLOT(setDisabled(bool)));

    setLayout(layout);
}

void AxisOptions::setDefaults( const QwtScaleEngine * scaleEngine, bool autoScale, const QwtScaleDiv * scaleDiv )
{
    if ( dynamic_cast<const QwtLinearScaleEngine*>(scaleEngine) )
        m_spacingComboBox->setCurrentIndex(0);
    else
        m_spacingComboBox->setCurrentIndex(1);

    if ( autoScale )
        m_autoCheckBox->setChecked(true);
    else
        m_autoCheckBox->setChecked(false);

    // Get lower and upper bounds
#if QWT_VERSION < 0x050200
    m_minLineEdit->setText(QString::number(scaleDiv->lBound()));
    m_maxLineEdit->setText(QString::number(scaleDiv->hBound()));
#else
    m_minLineEdit->setText(QString::number(scaleDiv->lowerBound()));
    m_maxLineEdit->setText(QString::number(scaleDiv->upperBound()));
#endif
}

bool AxisOptions::linearSpacing() const
{
    if ( m_spacingComboBox->currentIndex() == 0 )
        return true;
    else 
        return false;
}

bool AxisOptions::autoScale() const
{
    return m_autoCheckBox->isChecked();
}

double AxisOptions::min() const
{
    return m_minLineEdit->text().toDouble();
}

double AxisOptions::max() const
{
    return m_maxLineEdit->text().toDouble();
}

ConfigurePlotDialog::ConfigurePlotDialog( QwtPlot * plot, QWidget * parent)
    : QDialog(parent), m_plot(plot)
{
    // Create the dialog
    QGridLayout * layout = new QGridLayout;

    m_xAxisOptions = new AxisOptions(tr("X Axis") );
    m_xAxisOptions->setDefaults( 
            m_plot->axisScaleEngine( QwtPlot::xBottom ), 
            m_plot->axisAutoScale( QwtPlot::xBottom ), 
            m_plot->axisScaleDiv( QwtPlot::xBottom ) );
   
    layout->addWidget( m_xAxisOptions, 0, 0 );

    m_yAxisOptions = new AxisOptions(tr("Y Axis") );
    m_yAxisOptions->setDefaults( 
            m_plot->axisScaleEngine( QwtPlot::yLeft ), 
            m_plot->axisAutoScale( QwtPlot::yLeft ), 
            m_plot->axisScaleDiv( QwtPlot::yLeft ) );
    
    layout->addWidget( m_yAxisOptions, 1, 0 );
    
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
    setAxis( QwtPlot::xBottom, m_xAxisOptions );
    setAxis( QwtPlot::yLeft, m_yAxisOptions );
    
    accept();
}

void ConfigurePlotDialog::setAxis( int axisid, const AxisOptions * axisOptions )
{
    QwtScaleEngine * oldEngine = m_plot->axisScaleEngine(axisid);
    QwtScaleEngine * newEngine;

    if ( axisOptions->linearSpacing() )
        newEngine = new QwtLinearScaleEngine;
    else
        newEngine = new QwtLog10ScaleEngine;

    newEngine->setAttributes( oldEngine->attributes() );

    // Deletes the old engine
    m_plot->setAxisScaleEngine( axisid, newEngine );

    // Set the axis range
    if ( axisOptions->autoScale() )
        m_plot->setAxisAutoScale(axisid);
    else
        m_plot->setAxisScale( axisid, axisOptions->min(), axisOptions->max() );
}
