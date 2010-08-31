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

#include "CompatibleRvtMotion.h"

#include "Dimension.h"
#include "ResponseSpectrum.h"

#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"

#include <QDebug>

CompatibleRvtMotion::CompatibleRvtMotion(QObject* parent)
    : AbstractRvtMotion(parent)
{
    m_freq = new Dimension(this);
    m_freq->setMin(0.05);
    m_freq->setMax(50);
    m_freq->setSize(1024);
    m_freq->setSpacing(Dimension::Log);

    m_fourierAcc = QVector<double>(freq().size(), 0.);

    m_duration = 5.0;
    m_limitFas = true;

    m_targetRespSpec = new ResponseSpectrum;
    m_targetRespSpec->setDamping(5.0);

    m_name = tr("Compatible RVT Motion");
}

CompatibleRvtMotion::~CompatibleRvtMotion()
{
    m_targetRespSpec->deleteLater();
}

const QVector<double> & CompatibleRvtMotion::freq() const
{
    return m_freq->data();
}

Dimension*  CompatibleRvtMotion::freqDimension()
{
    return m_freq;
}

ResponseSpectrum * CompatibleRvtMotion::targetRespSpec()
{
    return m_targetRespSpec;
}

bool CompatibleRvtMotion::limitFas() const
{
    return m_limitFas;
}

void CompatibleRvtMotion::setLimitFas(bool limitFas)
{
    if ( m_limitFas != limitFas ) {
        m_modified = true;
        emit wasModified();
    }

    m_limitFas = limitFas;
}

void CompatibleRvtMotion::setDuration(double duration)
{
    if (m_duration != duration) {
        setModified(true);
    }

    m_duration = duration;
}

QString CompatibleRvtMotion::toHtml() const
{
    //FIXME

    return QString();
}

bool CompatibleRvtMotion::loadFromTextStream(QTextStream &stream, double scale)
{
    if (!AbstractRvtMotion::loadFromTextStream(stream, scale))
        return false;

    QVector<double> period, sa;
    bool ok = false;
    QString line = stream.readLine();
    QStringList parts;
    double d;
    while (!line.isNull()) {
        parts = line.split(',');

        // Break if the line doesn't contain at two columns
        if (parts.size() < 2)
            break;

        d = parts.at(0).toFloat(&ok);
        if (!ok) {
            qWarning() << tr("Unable to parse period in line:") << line;
            return false;
        } else {
            period << d;
        }

        d = parts.at(1).toFloat(&ok);
        if (!ok) {
            qWarning() << tr("Unable to parse the spectral acceleration in line:") << line;
            return false;
        } else {
            sa << (scale * d);
        }
        line = stream.readLine();
    }

    m_targetRespSpec->setPeriod(period);
    m_targetRespSpec->setSa(sa);

    calculate();

    return true;
}

void CompatibleRvtMotion::calculate()
{
    beginResetModel();
    // Check that both period and sa have some length
    if ( m_targetRespSpec->period().size() == 0 ||
         m_targetRespSpec->sa().size() != m_targetRespSpec->period().size()
        ) {
        qCritical("Both period and sa must be defined.");
        return;
    }

    // Check that the given period is constantly increasing
    for( int i = 0; i < m_targetRespSpec->period().size() - 1; ++i) {
        if( m_targetRespSpec->period().at(i) > m_targetRespSpec->period().at(i+1) ) {
            qCritical("The given period must be increasing");
            return;
        }
    }

    //
    // Estimate the FAS using the methodology proposed by Vanmarcke
    //
    const QVector<double> estimateFas = vanmarckeInversion();

    // Interpolate the FAS using a cubic spline, extrapolate at low frequencies.
    const double targetMinFreq = 1. / m_targetRespSpec->period().last();

//    m_freq = Dimension::logSpace( qMin(targetMinFreq / 2., 0.05), m_maxEngFreq, 1024 );
//    m_fourierAcc.resize(freq().size());
    int offset = 0;

    gsl_interp_accel * acc = gsl_interp_accel_alloc ();
    gsl_spline * spline = gsl_spline_alloc(gsl_interp_cspline, m_targetRespSpec->period().size());

    gsl_spline_init (spline, m_targetRespSpec->period().data(),
                     estimateFas.data(), m_targetRespSpec->period().size());

    const double logFas0 = log(estimateFas.first());
    const double freq0 = 1 / m_targetRespSpec->period().last();

    for (int i = 0; i < freq().size(); ++i) {
        if (freq().at(i) < targetMinFreq) {
            // Linearly extrapolate in log-log space.  This extrapolation is
            // not very rigorous, but it is just used to develop an initial
            // estimate of the FAS.
            m_fourierAcc[i] = exp(1.92 * log(freq().at(i)/freq0) + logFas0);
            offset = i;
        } else {
            m_fourierAcc[i] = gsl_spline_eval(spline, 1. / freq().at(i), acc);
        }
    }

    ++offset;

    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);

    // Interpolation for the ratio between target and calculated response spectrum
    acc = gsl_interp_accel_alloc();
    spline = gsl_spline_alloc(gsl_interp_cspline, m_targetRespSpec->period().size());

    // Loop over ratio correction while the rmse error is larger than FIXME
    m_okToContinue = true;
    double rmse = 0;
    double maxError = 0;
    double oldRmse = 1;
    int maxCount = 30;
    //double minRmse = 0.02;
    double minRmse = 0.005;
    double minRmseChange = 0.0002;
    int count = 0;

    // Initial response spectrum
    m_respSpec->setDamping(m_targetRespSpec->damping());
    m_respSpec->setPeriod(m_targetRespSpec->period());
    m_respSpec->setSa(computeSa(m_targetRespSpec->period(), m_targetRespSpec->damping()));

    QVector<double> ratio(m_respSpec->sa().size());

    // Set the maximum value of the progress
    emit progressRangeChanged(0, maxCount);

    do {
        emit progressValueChanged(count);

        // Determine the correction ratio
        for (int i = 0; i < ratio.size(); ++i)
            ratio[i] = m_targetRespSpec->sa().at(i) / m_respSpec->sa().at(i);

        // Apply the ratio correction to the FAS
        gsl_spline_init(spline, m_respSpec->period().data(), ratio.data(), ratio.size());

        for (int i = offset; i < freq().size(); ++i)
            m_fourierAcc[i] *= gsl_spline_eval(spline, 1. / freq().at(i), acc);

        // Extrapolate the low frequency values
        double logFreq0 = log(freq().at(offset));
        double logFas0 = log(m_fourierAcc.at(offset));

        double slope = m_limitFas ? 1.92 :
                       (log(m_fourierAcc.at(offset)/m_fourierAcc.at(offset+1))
                        / log( freq().at(offset)/freq().at(offset+1)));

        for (int i = 0; i < offset; ++i)
            m_fourierAcc[i] = exp( slope * (log(freq().at(i)) - logFreq0 ) + logFas0 );

        // Force down the high frequency tail

        // Find the minimum slope
        if (m_limitFas) {
            double minSlope = 0;
            int minSlopeIdx = 0;
            int i = offset;

            while (i < freq().size()-1) {
                const double slope = log(m_fourierAcc.at(i)/m_fourierAcc.at(i+1)) / log(freq().at(i)/freq().at(i+1));

                if (slope < minSlope) {
                    minSlope = slope;
                    minSlopeIdx = i;
                }
                ++i;
            }


            // Extrapolate from deviation
            i = minSlopeIdx;
            double x0 = log(freq().at(i));
            double y0 = log(m_fourierAcc.at(i));
            //double kappa0 = exp( -M_PI * 0.01 * freq().at(idx) );

            ++i;
            while ( i < m_fourierAcc.size() ) {
                // Extrapolate the value based, but reduce the value using a kappa filter
                //m_fourierAcc[idx] = exp( cutoff * (log(freq().at(idx)) - x0 ) + y0 ) * exp(-M_PI * 0.01 * freq().at(idx) ) / kappa0 ;
                m_fourierAcc[i] = exp( -slope * (log(freq().at(i)) - x0 ) + y0 );
                ++i;
            }
        }

        // Re-compute the Sa
        m_respSpec->setSa(computeSa(m_targetRespSpec->period(), m_targetRespSpec->damping()));

        // Compute the root-mean-squared error
        double sumError = 0;
        for (int i = 0; i < m_respSpec->sa().size(); ++i) {
            const double e = (m_respSpec->sa().at(i) - m_targetRespSpec->sa().at(i) ) / m_targetRespSpec->sa().at(i);

            // Save the maximum error
            if (fabs(maxError) < fabs(e)) {
                maxError = e;
            }

            // Add the squared error to the sum
            sumError += e * e;
        }
        rmse = sqrt(sumError / m_respSpec->sa().size());

        // Increment the count
        ++count;

        // qDebug() << count << maxError << rmse << minRmse << fabs(oldRmse-rmse) << minRmseChange;
        // Stop if the RMSE is below the specified RMSE
        if (rmse < minRmse || fabs(oldRmse-rmse) < minRmseChange)
            break;
        // Allow the user to cancel the operation
        if (!m_okToContinue)
            break;
        // Save old rmse
        oldRmse = rmse;
    } while (count < maxCount);

    // Delete the interpolators
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);

    // Set progress at the maximum
    emit progressValueChanged(maxCount);

    // Reset the modified target flag
    m_targetRespSpec->setModified(false);

    // Signal that the changes have taken place
    endResetModel();

    AbstractRvtMotion::calculate();
}

QVector<double> CompatibleRvtMotion::vanmarckeInversion() const
{
    QVector<double> fas(m_targetRespSpec->period().size());

    // Assume a constant peak factor
    double peakFactor = 2.5;
    double prevFasSqr = 0;
    double fasSqr = 0;
    double sum = 0;

    // Single degree of freedom factor
    double sdofFactor = M_PI / ( 4 * m_targetRespSpec->damping() / 100. ) - 1;
    const double damping = m_targetRespSpec->damping();

    for ( int i = fas.size()-1; i > -1; --i ) {
        double freq = 1 / m_targetRespSpec->period().at(i);
        double sa = m_targetRespSpec->sa().at(i);
        const double rmsDuration = calcRmsDuration(1/freq, damping);

        // Compute the squared Fourier amplitude spectrum
        fasSqr = ( ( rmsDuration * pow(sa,2) ) / ( 2 * pow( peakFactor, 2) ) - sum ) /
            ( freq * sdofFactor );

        if ( fasSqr < 0 )
            fasSqr = prevFasSqr;

        // Convert from spectral density into FAS
        fas[i] = sqrt(fasSqr);

        if ( i == fas.size()-1 )
            sum = fasSqr * freq / 2;
        else
            sum += ( fasSqr - prevFasSqr ) / 2 * ( freq - (1/m_targetRespSpec->period().at(i+1)) );

        prevFasSqr = fasSqr;
    }

    return fas;
}


QDataStream & operator<< (QDataStream & out, const CompatibleRvtMotion* crm)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractRvtMotion*>(crm);
    out << crm->m_freq << crm->m_limitFas << crm->m_targetRespSpec;

    return out;
}

QDataStream & operator>> (QDataStream & in, CompatibleRvtMotion* crm)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractRvtMotion*>(crm);
    in >> crm->m_freq >> crm->m_limitFas >> crm->m_targetRespSpec;

    crm->calculate();
    return in;
}
