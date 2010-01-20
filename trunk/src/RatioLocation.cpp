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
    m_strainTransFunc = new Output(Output::StrainTransferFunction);
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

Output * RatioLocation::strainTransFunc()
{
    return m_strainTransFunc;
}

Output * RatioLocation::respRatio()
{
    return m_respRatio;
}
        
void RatioLocation::clear()
{
    m_transFunc->clear();
    m_strainTransFunc->clear();
    m_respRatio->clear();
}

QVector<double> interp(const QVector<double> & x, const QVector<std::complex<double> > & y, const QVector<double> & xi)
{
    // Compute the absolute value of the series
    QVector<double> absY(y.size());

    for (int i = 0; i < y.size(); ++i)
        absY[i] = abs(y.at(i));

    // Interpolation requires increasing frequency range, make sure that we have it
    QVector<double> _x = x;
    const int n = _x.size();
    if ( _x.first() > _x.last() ) {
        int halfSize = 1 + n / 2;

        for ( int i = 0; i < halfSize; ++i ) {
            qSwap( _x[i], _x[n - 1 - i]);
            qSwap( absY[i], absY[n - 1 - i]);
        }
    }

    // Interpolate between the given frequency and the requested
    gsl_interp * interp = gsl_interp_alloc(gsl_interp_linear, _x.size());
    gsl_interp_init( interp, _x.data(), absY.data(), _x.size());
    gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

    QVector<double> absYi;

    for ( int i = 0; i < xi.size(); ++i)
        if (xi.at(i) < _x.last())
            absYi << gsl_interp_eval( interp, _x.data(), absY.data(), xi.at(i), accelerator);

    // Delete the interpolator and accelerator
    gsl_interp_free(interp);
    gsl_interp_accel_free(accelerator);

    return absYi;
}

void RatioLocation::saveResults( const EquivLinearCalc * calc, const QVector<double> & freq, const QVector<double> & period, double damping )
{
    const Location toLocation = calc->site()->depthToLocation(m_toDepth);
    const Location fromLocation = calc->site()->depthToLocation(m_fromDepth);

    if (m_transFunc->enabled())
        m_transFunc->addData(interp(
                calc->motion()->freq(),
                calc->calcAccelTf(fromLocation, m_fromType, toLocation, m_toType),
                freq));

    if (m_strainTransFunc->enabled()) {
        QVector<std::complex<double> > tf;
        calc->calcStrainTf(fromLocation, m_fromType, toLocation, tf);

        for (int i = 0; i < 10; ++i)
            qDebug() << abs(tf.at(i));
        m_strainTransFunc->addData(interp(
                calc->motion()->freq(),
                tf, freq));
    }

    if (m_respRatio->enabled()) {
        // fromLayer
        // FIXME should compute the depth into the input layer
        const QVector<std::complex<double> > fromAccelTf = calc->calcAccelTf(
                calc->site()->inputLocation(), calc->motion()->type(),
                fromLocation, m_fromType);

        const QVector<double> fromSa = calc->motion()->computeSa(period, damping, fromAccelTf);
        
        // toLayer
        const QVector<std::complex<double> > toAccelTf = calc->calcAccelTf(
                calc->site()->inputLocation(), calc->motion()->type(),
                toLocation, m_toType);

        const QVector<double> toSa = calc->motion()->computeSa(period, damping, toAccelTf );

        // Compute the ratio
        QVector<double> ratio(toSa.size());

        for (int i = 0; i < ratio.size(); ++i) {
            ratio[i] = toSa.at(i) / fromSa.at(i);
        }
       
        // Save the ratio
        m_respRatio->addData(ratio);
    }
}

void RatioLocation::removeLast()
{
    QList<Output*> outputs;
    outputs << m_transFunc << m_strainTransFunc << m_respRatio;

    foreach (Output * output, outputs) {
        if (output->enabled()) {
            output->removeLast();
        }
    }
}

QMap<QString, QVariant> RatioLocation::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("toDepth", m_toDepth);
    map.insert("toType", m_toType);

    map.insert("fromDepth", m_fromDepth);
    map.insert("fromType", m_fromType);

    map.insert("transFunc", m_transFunc->toMap());
    map.insert("strainTransFunc", m_strainTransFunc->toMap());
    map.insert("respRatio", m_respRatio->toMap());

    return map;
}

void RatioLocation::fromMap(const QMap<QString, QVariant> & map)
{
    m_toDepth = map.value("toDepth").toDouble();
    m_toType = (Motion::Type)map.value("toType").toInt();

    m_fromDepth = map.value("fromDepth").toDouble();
    m_fromType = (Motion::Type)map.value("fromType").toInt();

    m_transFunc->fromMap(map.value("transFunc").toMap());
    m_strainTransFunc->fromMap(map.value("strainTransFunc").toMap());
    m_respRatio->fromMap(map.value("respRatio").toMap());
}

void RatioLocation::renameOutputs()
{
    QString prefix = QString(QObject::tr("%1 (%2) to %3 (%4)"))
        .arg(locationToString(m_toDepth))
        .arg(Motion::typeList().at(m_toType))
        .arg(locationToString(m_fromDepth))
        .arg(Motion::typeList().at(m_fromType));

    m_transFunc->setPrefix(prefix);
    m_strainTransFunc->setPrefix(prefix);
    m_respRatio->setPrefix(prefix);
}

