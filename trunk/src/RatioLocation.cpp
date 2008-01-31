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

#include "RatioLocation.h"
#include "Algorithms.h"

#include <QtAlgorithms>
#include <QObject>
#include <QDebug>

#include <gsl/gsl_interp.h>

RatioLocation::RatioLocation()
{
    m_toDepth = 0;
    m_toType = Motion::Outcrop;

    m_fromDepth = -1; //Bedrock layer
    m_fromType = Motion::Outcrop;

    m_transFunc = new Output(Output::TransferFunction);
    m_respRatio = new Output(Output::SpectralRatio);

    renameOutputs();
}

double RatioLocation::toDepth() const
{
    return m_toDepth;
}

void RatioLocation::setToDepth(double toDepth)
{
    m_toDepth = toDepth;

    renameOutputs();
}

double RatioLocation::fromDepth() const
{
    return m_fromDepth;
}

void RatioLocation::setFromDepth(double fromDepth)
{
    m_fromDepth = fromDepth;

    renameOutputs();
}

Motion::Type RatioLocation::toType() const
{
    return m_toType;
}

void RatioLocation::setToType(Motion::Type toType)
{
    m_toType = toType;

    renameOutputs();
}

Motion::Type RatioLocation::fromType() const
{
    return m_fromType;
}

void RatioLocation::setFromType(Motion::Type fromType)
{
    m_fromType = fromType;

    renameOutputs();
}

Output * RatioLocation::transFunc()
{
    return m_transFunc;
}

Output * RatioLocation::respRatio()
{
    return m_respRatio;
}
        
void RatioLocation::clear()
{
    m_transFunc->clear();
    m_respRatio->clear();
}
        
void RatioLocation::saveResults( const EquivLinearCalc & calc, const QVector<double> & freq, const QVector<double> & period, double damping )
{
    const Location toLocation = calc.site()->depthToLocation( m_toDepth );
    const Location fromLocation = calc.site()->depthToLocation( m_fromDepth );

    if ( m_transFunc->enabled() ) {
        // Compute the transfer function
        const QVector<std::complex<double> > tf = calc.calcAccelTf(
                fromLocation, m_fromType, toLocation, m_toType );

        // Compute the absolute value of the transfer function
        QVector<double> absTf(tf.size());

        for (int i = 0; i < tf.size(); ++i)
            absTf[i] = abs(tf.at(i));

        // Interpolation requires increasing values of x
        QVector<double> tfFreq = calc.motion()->freq();

        if ( tfFreq.first() > tfFreq.last() ) {
            int halfSize = 1 + tf.size() / 2;

            for ( int i = 0; i < halfSize; ++i ) {
                qSwap( tfFreq[i], tfFreq[tf.size() - 1 - i ]);
                qSwap( absTf[i], absTf[tf.size() - 1 - i ]);
            }
        }
        
        // Interpolate between the given frequency and the requested
        QVector<double> interpAbsTf(freq.size());
        
        gsl_interp * interp = gsl_interp_alloc(gsl_interp_linear, absTf.size());

        gsl_interp_init( interp, tfFreq.data(), absTf.data(), absTf.size());
        gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

        for ( int i = 0; i < freq.size(); ++i)
            interpAbsTf[i] =
                gsl_interp_eval( interp, tfFreq.data(), absTf.data(), freq.at(i), accelerator);

        m_transFunc->addData(interpAbsTf);

        // Delete the interpolator and accelerator
        gsl_interp_free( interp );
        gsl_interp_accel_free( accelerator );
    }

    if ( m_respRatio->enabled() ) {
        // fromLayer
        // FIXME should compute the depth into the input layer
        const QVector<std::complex<double> > fromAccelTf = calc.calcAccelTf(
                calc.site()->inputLocation(), calc.motion()->type(),
                fromLocation, m_fromType);

        Motion::DurationType fromDurationType = ( m_fromDepth < 0 ) ? Motion::Rock : Motion::Soil;

        const QVector<double> fromSa = calc.motion()->computeSa( fromDurationType, period, damping, fromAccelTf );
        
        // toLayer
        const QVector<std::complex<double> > toAccelTf = calc.calcAccelTf(
                calc.site()->inputLocation(), calc.motion()->type(),
                toLocation, m_toType);
        
        Motion::DurationType toDurationType = ( m_toDepth < 0 ) ? Motion::Rock : Motion::Soil;

        const QVector<double> toSa = calc.motion()->computeSa( toDurationType, period, damping, toAccelTf );

        // Compute the ratio
        QVector<double> ratio(toSa.size());

        for (int i = 0; i < ratio.size(); ++i)
            ratio[i] = toSa.at(i) / fromSa.at(i);
       
        // Save the ratio
        m_respRatio->addData(ratio);
    }
}

QMap<QString, QVariant> RatioLocation::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;

    map.insert("toDepth", m_toDepth);
    map.insert("toType", m_toType);

    map.insert("fromDepth", m_fromDepth);
    map.insert("fromType", m_fromType);

    map.insert("transFunc", m_transFunc->toMap(saveData));
    map.insert("respRatio", m_respRatio->toMap(saveData));

    return map;
}

void RatioLocation::fromMap(const QMap<QString, QVariant> & map)
{
    m_toDepth = map.value("toDepth").toDouble();
    m_toType = (Motion::Type)map.value("toType").toInt();

    m_fromDepth = map.value("fromDepth").toDouble();
    m_fromType = (Motion::Type)map.value("fromType").toInt();

    m_transFunc->fromMap(map.value("transFunc").toMap());
    m_respRatio->fromMap(map.value("respRatio").toMap());
}

void RatioLocation::renameOutputs()
{
    QString prefix = QString(QObject::tr("%1 (%2) to %3 (%4)"))
        .arg(locationToString(m_fromDepth))
        .arg(Motion::typeList().at(m_fromType))
        .arg(locationToString(m_toDepth))
        .arg(Motion::typeList().at(m_toType));

    m_transFunc->setPrefix(prefix);
    m_respRatio->setPrefix(prefix);
}

