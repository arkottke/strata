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

#include "RvtMotion.h"
#include "Serializer.h"
#include "Dimension.h"
#include "MinimumFinder.h"

#include <QDebug>

// FIXME
#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_math.h>

#include <QFile>
#include <QTextStream>

// QTextStream fout(stdout);

RvtMotion::RvtMotion()
{
    m_oscCorrection = LiuPezeshk;
    m_workspace = gsl_integration_workspace_alloc(1024);
    m_targetRespSpec = new ResponseSpectrum;
    m_pointSourceModel = new PointSourceModel;
    reset();
}

RvtMotion::~RvtMotion()
{
    gsl_integration_workspace_free(m_workspace);
}

void RvtMotion::reset()
{
    m_type = Motion::Outcrop;
    m_sdofTfIsComputed = false;
    m_duration = 10;
    m_limitFas = true;
   m_source = DefinedFourierSpectrum;

    m_respSpec->reset();
    m_targetRespSpec->reset();

    m_fas.clear(); 
    m_freq.clear();
}

QStringList RvtMotion::sourceList()
{
    QStringList list;

    list << "Defined Fourier Spectrum" << "Defined Response Spectrum" << "Calculated Fourier Spectrum";

    return list;
}

double RvtMotion::max(const QVector<std::complex<double> > & tf ) const
{
    QVector<double> fas(m_fas.size());

    if ( !tf.isEmpty() ) {
        // Apply the transfer function to the fas
        for (int i = 0; i < m_fas.size(); ++i)
            fas[i] = abs(tf.at(i)) * m_fas.at(i);
    } else {
        fas = m_fas;
    }

    return calcMax(fas, m_duration);
}

const QVector<double> RvtMotion::absFas( const QVector<std::complex<double> > & tf ) const
{
    QVector<double> absFas(m_fas.size());

    if ( !tf.isEmpty() ) {
        // Apply the transfer function to the fas
        for (int i = 0; i < m_fas.size(); ++i)
            absFas[i] = abs(tf.at(i)) * m_fas.at(i);
    } else
        absFas = m_fas;

    return absFas;
}

QVector<double> & RvtMotion::fas()
{
    return m_fas;
}

double RvtMotion::duration() const
{
    return m_duration;
}

void RvtMotion::setDuration(double duration)
{
    setModified(true);
    m_duration = duration;
}

bool RvtMotion::limitFas() const
{
    return m_limitFas;
}

void RvtMotion::setLimitFas(bool limitFas)
{
    if ( m_limitFas != limitFas ) {
        m_modified = true;
        emit wasModified(); 
    }

    m_limitFas = limitFas;
}

RvtMotion::Source RvtMotion::source() const
{
    return m_source;
}

void RvtMotion::setSource(int source)
{
    setSource((Source)source);
}

void RvtMotion::setSource( RvtMotion::Source source )
{
    if ( m_source != source ) {
        emit wasModified();
    }

    m_source = source;
}

ResponseSpectrum * RvtMotion::targetRespSpec()
{
    return m_targetRespSpec;
}

PointSourceModel * RvtMotion::pointSourceModel()
{
    return m_pointSourceModel;
}

//! Basic linear interpolation/extrapolation in log-log space
/*! The function takes the log10 of the values and then uses linear
 * interpolation to determine the values.  If the interpolated x coordinates
 * are beyond the specified x coordinates the then extrapolation is used based
 * on the last two specified points.
 */
bool logLogInterp( const QVector<double> & x, const QVector<double> & y, 
        const QVector<double> & xi, QVector<double> & yi)
{
    // Check the sizes of xi and yi	
    if ( xi.size() != yi.size() )
        yi.resize(xi.size());

    // Compute the log of the values
    QVector<double> logX(x.size());
    QVector<double> logY(y.size());
    QVector<double> logXi(xi.size());
    QVector<double> logYi(yi.size());

    for (int i = 0; i < x.size(); ++i) {
        logX[i] = log10(x.at(i));
        logY[i] = log10(y.at(i));
    }

    for (int i = 0; i < xi.size(); ++i)
        logXi[i] = log10(xi.at(i));

    // Interpolate -- Extrapolate
    for ( int i = 0; i < logXi.size(); ++i )
    {
        for ( int j = 0; j < logX.size()-1; ++j )
        {
            if ( fabs(logXi.at(i)-logX.at(j)) < 0.000001 ) {
                // XI and X are the same value
                logYi[i] = logY.at(j);
                // Exit out of the loop
                break;
            } else if ( logX.at(j) < logXi.at(i) && logXi.at(i) <= logX.at(j+1) ) {
                // X_J < XI <= X_J+1
                // LinearllogY interpolate
                double slope = (logY.at(j+1) - logY.at(j)) / (logX.at(j+1) - logX.at(j));
                logYi[i] = slope * (logXi.at(i) - logX.at(j)) + logY.at(j);

                // Exit out of the loop
                break;
            } else if ( logXi.at(i) < logX.first() ) {
                // Extrapolate based on the first two values
                // Extrapolate based on the first two values
                double slope = (logY.at(1) - logY.first()) / (logX.at(1) - logX.first());
                logYi[i] = slope * (logXi.at(i) - logX.first()) + logY.first();
                // Exit out of the loop
                break;
            } else if (logX.last() < logXi.at(i)) {
                // Extrapolate based on the last two values
                // The second to last point
                int n = logY.size() - 2;
                // Extrapolate based on the first two values
                double slope = (logY.last() - logY.at(n)) / (logX.last() - logX.at(n));
                logYi[i] = slope * (logXi.at(i) - logX.last()) + logY.last();
                // Exit out of the loop
                break;
            }
        }
    }

    // Compute the power of the yi values
    for (int i = 0; i < yi.size(); ++i)
        yi[i] = pow(10, logYi.at(i));

    return true;
}

//! A moving average over the data series
/*!
 * \param data data series to smooth
 * \param window the number of ponits on either side of a given point to average against
 */
void smooth(QVector<double> & data, int window)
{
    QVector<double> smoothData(data.size());
    // Window size is adjusted at tails of the data
    int adjustedWindow = 0;

    for ( int i = 0; i < data.size(); ++i) {
        // Adjust the window based on the number of points
        // Number of indexes to the left of a point
        int left = i;
        // Number of indexes to the right of a point
        int right = data.size() - 1 - i;

        if ( window < left && window < right )
            // Enough room on either side of the given point
            adjustedWindow = window;
        else if ( window >= left && window < right )
            // Not enough room on the left side
            adjustedWindow = left;
        else if ( window < left && window >= right ) {
            // Not enough room on the right side
            adjustedWindow = right;

            // qDebug() << "not enough room on the right side, adjusted window:" << adjustedWindow;
        } else {
            // Not enough room on either side -- use the short side
            if ( left < right )
                adjustedWindow = left;
            else
                adjustedWindow = right;
        }

        // Average the points
        double sum = 0;

        for (int j = -adjustedWindow; j <= adjustedWindow; ++j)
            sum += data.at(i+j);

        smoothData[i] = sum / ( 1 + 2 * adjustedWindow);
    }
    // Replace the original data with the smoothed data
    data = smoothData;
}

QVector<double> RvtMotion::computeSa(const QVector<double> & period, double damping, const QVector<std::complex<double> > & tf )
{
    QVector<double> sa(period.size());
    QVector<double> fas = m_fas;

    // Apply the transfer function to the motion
    if (!tf.isEmpty()) {
        Q_ASSERT(fas.size() == tf.size());

        for (int i = 0; i < fas.size(); ++i) {
            fas[i] *= abs(tf.at(i));
        }
    }

    // Compute the response at each period
    for ( int i = 0; i < sa.size(); ++i ) {
        sa[i] = calcOscillatorMax(fas, period.at(i), damping);
    }

    return sa;
}

bool RvtMotion::invert()
{
    // Check that both period and sa have some length
    if ( m_targetRespSpec->period().size() == 0 ||
            m_targetRespSpec->sa().size() != m_targetRespSpec->period().size()
       ) {
        qCritical("Both period and sa must be defined.");
        return false;
    }

    // Check that the given period is constantly increasing
    for( int i = 0; i < m_targetRespSpec->period().size() - 1; ++i) {
        if( m_targetRespSpec->period().at(i) > m_targetRespSpec->period().at(i+1) ) {
            qCritical("The given period must be increasing");
            return false;
        }
    }

    //
    // Estimate the FAS using the methodology proposed by Vanmarcke
    //
    const QVector<double> estimateFas = vanmarckeInversion();

    // Interpolate the FAS using a cubic spline, extrapolate at low frequencies.
    const double targetMinFreq = 1. / m_targetRespSpec->period().last();

    m_freq = Dimension::logSpace( qMin(targetMinFreq / 2., 0.05), m_maxEngFreq, 1024 );
    m_fas.resize(m_freq.size());
    int offset = 0;

    gsl_interp_accel * acc = gsl_interp_accel_alloc ();
    gsl_spline * spline = gsl_spline_alloc(gsl_interp_cspline, m_targetRespSpec->period().size());

    gsl_spline_init (spline, m_targetRespSpec->period().data(),
                     estimateFas.data(), m_targetRespSpec->period().size());

    const double logFas0 = log(estimateFas.first());
    const double freq0 = 1 / m_targetRespSpec->period().last();

    for (int i = 0; i < m_freq.size(); ++i) {
        if (m_freq.at(i) < targetMinFreq) {
            // Linearly extrapolate in log-log space.  This extrapolation is
            // not very rigorous, but it is just used to develop an initial
            // estimate of the FAS.
            m_fas[i] = exp(1.92 * log(m_freq.at(i)/freq0) + logFas0);
            offset = i;
        } else {
            m_fas[i] = gsl_spline_eval(spline, 1. / m_freq.at(i), acc);
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

        for (int i = 0; i < ratio.size(); ++i) {
            ratio[i] = m_targetRespSpec->sa().at(i) / m_respSpec->sa().at(i);
        }

        // Apply the ratio correction to the FAS
        gsl_spline_init(spline, m_respSpec->period().data(), ratio.data(), ratio.size());

        for (int i = offset; i < m_freq.size(); ++i) {
            m_fas[i] *= gsl_spline_eval(spline, 1. / m_freq.at(i), acc);
        }

        // Extrapolate the low frequency values
        double logFreq0 = log(m_freq.at(offset));
        double logFas0 = log(m_fas.at(offset));

        double slope = m_limitFas ? 1.92 :
            (log(m_fas.at(offset)/m_fas.at(offset+1))
             / log( m_freq.at(offset)/m_freq.at(offset+1)));

        for (int i = 0; i < offset; ++i) {

                m_fas[i] = exp( slope * (log(m_freq.at(i)) - logFreq0 ) + logFas0 );

        }

        // Force down the high frequency tail

        // Find the minimum slope
        if (m_limitFas) {
            double minSlope = 0;
            int minSlopeIdx = 0;
            int i = offset;

            while (i < m_freq.size()-1) {
                const double slope = log(m_fas.at(i)/m_fas.at(i+1)) / log(m_freq.at(i)/m_freq.at(i+1));

                if (slope < minSlope) {
                    minSlope = slope;
                    minSlopeIdx = i;
                }
                ++i;
            }


            // Extrapolate from deviation
            i = minSlopeIdx;
            double x0 = log(m_freq.at(i));
            double y0 = log(m_fas.at(i));
            //double kappa0 = exp( -M_PI * 0.01 * m_freq.at(idx) );

            ++i;
            while ( i < m_fas.size() ) {
                // Extrapolate the value based, but reduce the value using a kappa filter
                //m_fas[idx] = exp( cutoff * (log(m_freq.at(idx)) - x0 ) + y0 ) * exp(-M_PI * 0.01 * m_freq.at(idx) ) / kappa0 ;
                m_fas[i] = exp( -slope * (log(m_freq.at(i)) - x0 ) + y0 );
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

        qDebug() << count << maxError << rmse << minRmse << fabs(oldRmse-rmse) << minRmseChange;
        // Stop if the RMSE is below the specified RMSE
        if ( rmse < minRmse || fabs(oldRmse-rmse) < minRmseChange )
            break;
        // Allow the user to cancel the operation
        if ( !m_okToContinue )
            break;
        // Save old rmse
        oldRmse = rmse;
    } while( count < maxCount );

    // Delete the interpolators
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);

    // Set progress at the maximum
    emit progressValueChanged(maxCount);

    // Reset the modified target flag
    m_targetRespSpec->setModified(false);

    // Signal that the Fourier Spectrum has been updated
    emit fourierSpectrumChanged();

    // If the rmse value is below  0.15 it was successful
    if ( rmse < minRmse )
        return true;
    else
        return false;
}

void RvtMotion::calcPointSource()
{
    m_freq = Dimension::logSpace( 0.05, m_maxEngFreq, 1024 );
    m_fas = m_pointSourceModel->calcFourierSpectrum(m_freq);
}

bool RvtMotion::hasTime() const
{
    return false;
}

QString RvtMotion::toString() const
{
    return "RVT Motion";
}

QMap<QString, QVariant> RvtMotion::toMap(bool /*saveData*/) const
{
    QMap<QString, QVariant> map;

    map.insert("duration", m_duration);
    map.insert("limitFas", m_limitFas);
    map.insert("source", m_source);
    map.insert("type", m_type);
    map.insert("maxEngFreq", m_maxEngFreq);

    map.insert("fas", Serializer::toVariantList(m_fas));
    map.insert("freq", Serializer::toVariantList(m_freq));

    map.insert("respSpec", m_respSpec->toMap());
    map.insert("targetRespSpec", m_targetRespSpec->toMap());
    map.insert("pointSourceModel", m_pointSourceModel->toMap());

    return map;
}

void RvtMotion::fromMap( const QMap<QString, QVariant> & map)
{
    m_duration = map.value("duration").toDouble();
    m_limitFas = map.value("limitFas").toBool();
    m_source = (RvtMotion::Source)map.value("source").toInt();
    m_type = (Motion::Type)map.value("type").toInt();
    m_maxEngFreq = map.value("maxEngFreq", 25).toDouble();

    m_fas = Serializer::fromVariantList(map.value("fas").toList()).toVector();
    m_freq = Serializer::fromVariantList(map.value("freq").toList()).toVector();

    emit fourierSpectrumChanged();

    m_respSpec->fromMap(map.value("respSpec").toMap());
    m_targetRespSpec->fromMap(map.value("targetRespSpec").toMap());
    m_pointSourceModel->fromMap(map.value("pointSourceModel").toMap());
}

QString RvtMotion::toHtml() const
{
    QString html;

    html += QString(tr(
                "<table border=\"0\">"
                "<tr><td><strong>Type:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Duration:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Source:</strong></td><td>%3</td></tr>"

                ))
        .arg(typeList().at(m_type))
        .arg(m_duration)
        .arg(sourceList().at(m_source));

    switch (m_source)
    {
        case DefinedFourierSpectrum:
            html += tr("</table><table><tr><th>Frequency (Hz)</th><th>FAS (g-s)</th></tr>");

            for ( int i = 0; i < m_freq.size(); ++i )
                html += QString("<tr><td>%1</td><td>%2</td></tr>")
                    .arg( m_freq.at(i) )
                    .arg( m_fas.at(i) );

            html += "</table>";
            break;
        case DefinedResponseSpectrum:
            html += QString(tr("<tr><td><strong>Damping:</strong></td><td>%1</td></tr>"))
                .arg(m_targetRespSpec->damping());

            html += tr("</table><table><tr><th>Period (s)</th><th>Spectral Accel. (g)</th></tr>");

            for ( int i = 0; i < m_targetRespSpec->period().size(); ++i )
                html += QString("<tr><td>%1</td><td>%2</td></tr>")
                    .arg( m_targetRespSpec->period().at(i) )
                    .arg( m_targetRespSpec->sa().at(i) );

            html += "</table>";
            break;
        case CalculatedFourierSpectrum:
            html += m_pointSourceModel->toHtml();
            break;
    }
    return html;
}

void RvtMotion::stop()
{
    m_okToContinue = false;
}

QVector<double> RvtMotion::vanmarckeInversion() const
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

double RvtMotion::moment( int power, const QVector<double> & fasSqr) const
{
    // The moment is found by:
    //           /
    // m_n = 2 * | ( 2 * pi * freq )^n * FAS^2 * df
    //           /
    double sum = 0;
    double dFreq = 0;
    double last = pow( 2 * M_PI * m_freq.at(0), power) * fasSqr.at(0);
    double current = 0.0;
    // Integrate using the trapezoid rule
    for (int i=1; i < fasSqr.size(); ++i) {
        // Compute the current piece
        current = pow( 2 * M_PI * m_freq.at(i), power) * fasSqr.at(i);

        // Frequency may be increasing or decreasing, just want the difference
        dFreq = fabs(m_freq.at(i) - m_freq.at(i-1));

        // Compute the area of the trapezoid defined by the current and last value
        sum += dFreq * ( current + last ) / 2;
        // Save the current piece
        last = current;
    }

    return 2 * sum;
}

struct peakFactorParams
{
    double bandWidth;
    double numExtrema;
};

double RvtMotion::peakFactorEqn(double z, void * p)
{
    struct peakFactorParams * params = (struct peakFactorParams *)p;

    return 1 - pow( 1 - params->bandWidth * exp(-z*z), params->numExtrema );
}

double RvtMotion::calcMax( const QVector<double> & fas, double durationRms ) const
{
    if ( durationRms < 0 ) {
        durationRms = m_duration;
    }

    // Square the Fourier amplitude spectrum
    QVector<double> fasSqr(fas.size());
    for ( int i = 0; i < fasSqr.size(); ++i ) {
        fasSqr[i] = fas.at(i) * fas.at(i);
    }

    // The zero moment is the area of the spectral density computed by the
    // trapezoid rule.
    double m0 = moment( 0, fasSqr);
    // double m1 = moment( 1, fasSqr); // FIXME Remove this
    double m2 = moment( 2, fasSqr);
    double m4 = moment( 4, fasSqr);

    // Compute the bandwidth
    double bandWidth = sqrt((m2 * m2 )/( m0 * m4));

    // Compute the number extrema 
    double numExtrema = sqrt(m4/m2) * m_duration / M_PI;

    // If the number of extrema is less than 2, increase to 2.  There must be
    // one full cycle (two peaks)
    if (numExtrema <2) {
        numExtrema = 2;
    }

    // Use GSL to integrate the peak factor equation
    struct peakFactorParams params = { bandWidth, numExtrema };
    gsl_function F = { &peakFactorEqn, &params};
    double result;
    double error;

    // Try the adaptive integration without bounds
    if ( bandWidth != bandWidth || numExtrema != numExtrema ) {
        // We have issues!
        QString fileName;
        int i = 0;

        do {
            fileName = QString("rvtInfo-%1.dat").arg(i);
        } while (QFile::exists(fileName));

         QFile file(fileName);

         if (file.open(QFile::WriteOnly)) {
             QTextStream fout(&file);

             fout << "durationGm: " << m_duration << endl;
             fout << "durationRms: " << durationRms << endl;
             fout << "m0: " << m0 << endl;
             fout << "m2: " << m2 << endl;
             fout << "m4: " << m4 << endl;
             fout << "bandWidth: " << bandWidth << endl;
             fout << "numExtrema: " << numExtrema << endl;

             for (int i = 0; i < fas.size(); ++i) {
                 fout << i << " " << fas.at(i) << endl;
             }
         }

        result = 1.0;
    } else {
        gsl_integration_qagiu( &F, 0, 0, 1e-7, 1000, m_workspace, &result, &error);
    }

    const double peakFactor = sqrt(2) * result;

    // fout << bandWidth << ","
    //     << sqrt(1 - m1 * m1 / m0 / m2 ) << ","
    //     << numExtrema << ","
    //     << peakFactor << ","
    //     << sqrt(m0/durationRms) << endl;

    // FIXME Compute the peak factor using the asympototic solution
    // double numZero = sqrt(m2/m0) * durationGm / M_PI;
    // double peakFactor_asym = sqrt(2 * log(numZero)) + 0.5772 / sqrt(2 * log(numZero));
    // qDebug() << peakFactor <<  peakFactor_asym;

    // Return the peak value which is found by multiplying the variation by the peakfactor
    return sqrt(m0/durationRms) * peakFactor;
}

double RvtMotion::calcOscillatorMax(QVector<double> fas, const double period, const double damping) const
{
    const QVector<std::complex<double> > tf = calcSdofTf(period, damping);

    Q_ASSERT(tf.size() == fas.size());

    for (int i = 0; i < fas.size(); ++i) {
        fas[i] *= abs(tf.at(i));
    }

    return calcMax(fas, calcRmsDuration(period, damping, fas));
}

double RvtMotion::calcRmsDuration(const double period, const double damping, const QVector<double> & fas) const
{
    // Use BooreJoyner if there is no FAS defined
    OscillatorCorrection oscCorrection = fas.isEmpty() ? BooreJoyner : m_oscCorrection;

    // Duration of the oscillator
    const double durOsc = period / (2 * M_PI * damping / 100);

    int power = 0;
    double bar = 0;

    switch (oscCorrection)  {
    case BooreJoyner:
        power = 3;
        bar = 1. / 3.;
        break;

    case LiuPezeshk:          
        QVector<double> fasSqr(fas.size());

        for (int i = 0; i < fas.size(); ++i) {
            fasSqr[i] = fas.at(i) * fas.at(i);
        }

        const double m0 = moment(0, fasSqr);
        const double m1 = moment(1, fasSqr);
        const double m2 = moment(2, fasSqr);

        power = 2;
        bar = sqrt( 2 * M_PI * ( 1 - (m1 * m1)/(m0 * m2)));
        break;
    }

    const double foo = pow(m_duration / period, power);

    const double durationRms = m_duration + durOsc * (foo / (foo + bar));

    return durationRms;
}
