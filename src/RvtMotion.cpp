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

#include <QDebug>
#include <QApplication>

#include <gsl/gsl_interp.h>

#include <cmath>

#include <QFile>
#include <QTextStream>

RvtMotion::RvtMotion()
{
    m_targetRespSpec = new ResponseSpectrum;
    reset();
}

void RvtMotion::reset()
{
    m_sdofTfIsComputed = false;
    m_duration = 10;
    m_strainFactor = 1;
    m_soilFactor = 1;
    m_source = DefinedFourierSpectrum;

    m_respSpec->reset();
    m_targetRespSpec->reset();

    m_fas.clear(); 
    m_freq.clear();
}

QStringList RvtMotion::sourceList()
{
    QStringList list;

    list << "Fourier Spectrum" << "Response Spectrum";

    return list;
}

double RvtMotion::max( Motion::DurationType durationType, const QVector<std::complex<double> > & tf ) const
{
    QVector<double> fas(m_fas.size());

    if ( !tf.isEmpty() ) {
        // Apply the transfer function to the fas
        for (int i = 0; i < m_fas.size(); ++i)
            fas[i] = abs(tf.at(i)) * m_fas.at(i);
    } else
        fas = m_fas;

    return rvt( fas, durationOfGm(durationType) );
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
    m_duration = duration;
}

double RvtMotion::strainFactor() const
{
    return m_strainFactor;
}

void RvtMotion::setStrainFactor(double strainFactor)
{
    m_strainFactor = strainFactor;
}

double RvtMotion::soilFactor() const
{
    return m_soilFactor;
}

void RvtMotion::setSoilFactor(double soilFactor)
{
    m_soilFactor = soilFactor;
}

RvtMotion::Source RvtMotion::source() const
{
    return m_source;
}

void RvtMotion::setSource( RvtMotion::Source source )
{
    m_source = source;
}

ResponseSpectrum * RvtMotion::targetRespSpec()
{
    return m_targetRespSpec;
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

    for (int i = 0; i < xi.size(); ++i) {
        logXi[i] = log10(xi.at(i));
        logYi[i] = log10(yi.at(i));
    }

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
        int right = data.size() - i;

        if ( window < left && window < right )
            // Enough room on either side of the given point
            adjustedWindow = window;
        else if ( window >= left && window < right )
            // Not enough room on the left side
            adjustedWindow = left;
        else if ( window < left && window >= right )
            // Not enough room on the right side
            adjustedWindow = right;
        else {
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

void RvtMotion::forceDown(int dir)
{
    Q_ASSERT( m_freq.at(0) > m_freq.at(1) );
    // Direction should be either 1 or -1
    dir = ( dir > 0 ) ? 1 : -1;

    // Find the maximum value of the FAS
    double max = 0;
    int maxPos = 0;

    for ( int i = 0; i < m_fas.size(); ++i ) {
        if ( m_fas.at(i) > max ) {
            max = m_fas.at(i);
            maxPos = i;
        }
    }

    // Determine the final index
    int stop = ( dir > 0 ) ? m_fas.size() : -1;

    // Previous slope
    double minSlope = -0.16;
    double maxSlope = 0;
    double x0 = 0;
    double y0 = 0;

    int i = maxPos;

    while ( i+dir != stop ) {
        x0 = log10(m_freq.at(i));
        y0 = log10(m_fas.at(i));

        // Compute the slope is log-log space
        double slope = dir * ( log10(m_fas.at(i+dir)) - y0 ) / (log10(m_freq.at(i+dir)) - x0 );

        if ( i != maxPos && slope < minSlope ) {
            // qDebug() << "min slope:" << minSlope << "slope:" << slope;
            // qDebug() << "applying correction from" << i << "to" << stop << "slope:" << maxSlope;
            // Correct the curve using the previous slope
            while ( i != stop ) {
                m_fas[i] = pow( 10, dir * maxSlope * (log10(m_freq.at(i)) - x0) + y0);
                i += dir;
            }
            return;
        }

        if ( slope > maxSlope )
            maxSlope = slope;

        // Advance the counter
        i += dir;
    }
}

QVector<double> RvtMotion::computeSa( DurationType durationType, const QVector<double> & period, double damping, const QVector<std::complex<double> > & accelTf )
{
    double durationGm = durationOfGm(durationType);
    // Compute the SDOF
    computeSdofTf( period, damping );

    QVector<double> sa(period.size());
    QVector<double> fas(m_fas.size());

    // Compute the response at each period
    for ( int i = 0; i < sa.size(); ++i ) {
        // If there is an acceleration transfer function combine the SDOF and
        // acceleration transfer functions
        if ( !accelTf.isEmpty() ) {
            for (int j = 0; j < fas.size(); ++j)
                fas[j] = abs(accelTf.at(j)) * abs(m_sdofTf.at(i).at(j)) * m_fas.at(j);
        } else
            for (int j = 0; j < fas.size(); ++j)
                fas[j] = abs(m_sdofTf.at(i).at(j)) * m_fas.at(j);

        // The duration is first increased by T0 to account for the fact peak
        // response of a single-degree-of-freedom oscillator may occur ater short
        // period motions.  The duration is then reduced by the correction by Boore
        // (1984).  The reduction is too limit the _T0_ increase to acceptable
        // values.
        double T0 = period.at(i)/(2 * M_PI * damping / 100);
        double durationRms = durationGm + T0 * pow(durationGm/period.at(i),3) / ( pow(durationGm/period.at(i),3) + 1/3 );

        // Compute the maximum expected acceleration
        sa[i] = rvt(fas, durationGm, durationRms);
    }

    return sa;
}

bool RvtMotion::invert( const bool * okToContinue, QProgressBar * progress)
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

    // Use the same period and damping as the target FIXME
    m_respSpec->period() = m_targetRespSpec->period();
    m_respSpec->setDamping( m_targetRespSpec->damping() );

    // Compute the frequency from the minimum and maximum periods
    m_freq = Dimension::logSpace( 1 / m_respSpec->period().first(), 1 / m_respSpec->period().last(), 1024 );
    m_fas.resize(m_freq.size());

    // Compute the target reponse spectrum interpolated at each of the frequency values
    gsl_interp * interpolator = gsl_interp_alloc(gsl_interp_linear, m_targetRespSpec->period().size());
    gsl_interp_init( interpolator, m_targetRespSpec->period().data(), m_targetRespSpec->sa().data(), m_targetRespSpec->period().size());
    gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

    QVector<double> targetSa(m_freq.size());
    for ( int i = 0; i < targetSa.size(); ++i )
        targetSa[i] = gsl_interp_eval( interpolator, m_targetRespSpec->period().data(),
                m_targetRespSpec->sa().data(), 1 / m_freq.at(i), accelerator);

    gsl_interp_free( interpolator );
    gsl_interp_accel_free( accelerator );

    // The Fourier spectrum is initially computed base on a simple shape
    //simpleFas();
    // The Fourier spectrum is initially computed based on the Brune 
    // brune();
    // Compute the Fourier Spectrum using the spectral density and inversion
    vanmarckeInversion(targetSa, m_targetRespSpec->damping());

    // Compute the response spectra
    m_respSpec->sa() = computeSa( Motion::Rock, m_respSpec->period(), m_respSpec->damping());

    // Prepare the interpolation 
    interpolator = gsl_interp_alloc(gsl_interp_linear, m_respSpec->period().size());
    gsl_interp_init( interpolator, m_respSpec->period().data(), m_respSpec->sa().data(), m_respSpec->period().size());
    accelerator =  gsl_interp_accel_alloc();

    // Loop over ratio correction while the rmse error is larger than FIXME
    double rmse = 0;
    double oldRmse = 1;
    int maxCount = 25;
    double minRmse = 0.02;
    double minRmseChange = 0.001;
    int count = 0;
    // Set the maximum value of the progress
    if (progress)
        progress->setRange(0, maxCount);

    do {
        // Update the progress dialog
        if (progress) {
            progress->setValue(count);
            QApplication::processEvents();
        }

        // Correct the FAS by the ratio
        for (int i = 0; i < m_fas.size(); ++i) {
            double sa = gsl_interp_eval( interpolator,
                    m_respSpec->period().data(), m_respSpec->sa().data(), 1 /
                    m_freq.at(i), accelerator);
            m_fas[i] = m_fas.at(i) * targetSa.at(i) / sa;
        }

        // Smooth the data
        smooth( m_fas, int(m_fas.size()/100));
        // Force the tails down
        forceDown(1);
        forceDown(-1);
        // Re-compute the Sa
        m_respSpec->sa() = computeSa( Motion::Rock, m_respSpec->period(), m_respSpec->damping());

        // Compute the root-mean-squared error
        double error = 0;
        for (int i = 0; i < m_respSpec->sa().size(); ++i)
            error += pow( (m_respSpec->sa().at(i) - m_targetRespSpec->sa().at(i) ) / m_targetRespSpec->sa().at(i), 2 );
        rmse = sqrt(error / m_respSpec->sa().size());

        // Increment the count
        ++count;

        //qDebug() << count << rmse;
        // Stop if the RMSE is below the specified RMSE
        if ( rmse < minRmse || fabs(oldRmse-rmse) < minRmseChange )
            break;
        // Allow the user to cancel the operation
        if ( okToContinue && ! *okToContinue )
            break;
        // Save old rmse
        oldRmse = rmse;
    } while( count < maxCount );

    // Delete the interpolators
    gsl_interp_free( interpolator );
    gsl_interp_accel_free( accelerator );

    // Set progress at the maximum
    if (progress)
        progress->setValue(maxCount);

    // Reset the modified target flag
    m_targetRespSpec->setModified(false);

    // Signal that the Fourier Spectrum has been updated
    emit fourierSpectrumChanged();

    // If the rmse value is below  0.15 it was successful
    if ( rmse < 0.15 )
        return true;
    else 
        return false;
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
    map.insert("strainFactor", m_strainFactor);
    map.insert("soilFactor", m_soilFactor);
    map.insert("source", m_source);
    map.insert("type", m_type);

    map.insert("fas", Serializer::toVariantList(m_fas));
    map.insert("freq", Serializer::toVariantList(m_freq));

    map.insert("respSpec", m_respSpec->toMap());
    map.insert("targetRespSpec", m_targetRespSpec->toMap());

    return map;
}

void RvtMotion::fromMap( const QMap<QString, QVariant> & map)
{
    m_duration = map.value("duration").toDouble();
    m_strainFactor = map.value("strainFactor").toDouble();
    m_soilFactor = map.value("soilFactor").toDouble();
    m_source = (RvtMotion::Source)map.value("source").toInt();
    m_type = (Motion::Type)map.value("type").toInt();

    m_fas = Serializer::fromVariantList(map.value("fas").toList()).toVector();
    m_freq = Serializer::fromVariantList(map.value("freq").toList()).toVector();

    emit fourierSpectrumChanged();
    
    m_respSpec->fromMap(map.value("respSpec").toMap());
    m_targetRespSpec->fromMap(map.value("targetRespSpec").toMap());
}

QString RvtMotion::toHtml() const
{
    QString html;

    html += QString(QObject::tr(
                "<table border=\"0\">"
                "<tr><td><strong>Type:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Duration:</strong></td><td>%1</td></tr>"
                ))
        .arg(typeList().at(m_type))
        .arg(m_duration);

    switch (m_source)
    {
        case DefinedFourierSpectrum:
            html += QObject::tr("</table><table><tr><th>Frequency (Hz)</th><th>FAS (g-s)</th></tr>");

            for ( int i = 0; i < m_freq.size(); ++i )
                html += QString("<tr><td>%1</td><td>%2</td></tr>")
                    .arg( m_freq.at(i) )
                    .arg( m_fas.at(i) );

            html += "</table>";
            break;
        case DefinedResponseSpectrum:
            html += QString(QObject::tr("<tr><td><strong>Damping:</strong></td><td>%1</td></tr>"))
                .arg(m_targetRespSpec->damping());

            html += QObject::tr("</table><table><tr><th>Period (s)</th><th>Spectral Accel. (g)</th></tr>");

            for ( int i = 0; i < m_targetRespSpec->period().size(); ++i )
                html += QString("<tr><td>%1</td><td>%2</td></tr>")
                    .arg( m_targetRespSpec->period().at(i) )
                    .arg( m_targetRespSpec->sa().at(i) );

            html += "</table>";
            break;
    }
    return html;
}
        
double RvtMotion::durationOfGm(Motion::DurationType durationType) const
{
    switch (durationType)
    {
        case Rock:
            return m_duration;
        case Soil:
            return m_soilFactor * m_duration;
        case Strain:
            return m_strainFactor * m_duration;
        default:
            return 0;
    }
}

void RvtMotion::brune( double magnitude, double distance, double stressDrop, double kappa)
{
    // Constants
    double rho          = 2.8;          // Soil density in grams per cubic centimeter
    double beta         = 3.5;          // Shear wave velocity in km per second
    double depth        = 10;           // Depth to the earthquake

    // Compute the radius taking into account the depth
    double radius = sqrt(distance*distance + depth*depth);
    // Seismic moment
    double seismicMoment = pow( 10, 1.5 * magnitude + 16.05 );
    // Compute the corner frequency
    double cornerFreq = 4.9e6 * beta * pow(stressDrop / seismicMoment, 1./3.);
    // Compute the rock amplification
    QVector<double> rockAmp(m_freq.size());
    QVector<double> freq_i;
    QVector<double> amp_i;
    // Load the empirical arrays with the values based on the Western US
    // Generic Rock amplification by Boore and Joyner
    freq_i << 0.01 << 0.09 << 0.16 << 0.51 << 0.84 << 1.25 << 2.26 << 3.17 << 6.05 << 16.6 << 61.2 << 81.2 << 100 << 200;
    amp_i << 1 << 1.1 << 1.18 << 1.42 << 1.58 << 1.74 << 2.06 << 2.25 << 2.58 << 3.13 << 4 << 4.39 << 4.77 << 5;
    // Interpolate
    logLogInterp(freq_i, amp_i, m_freq, rockAmp);

    // Compute the geometric attenuation factor
    double geometricAtten;
    if ( radius > 40 )
        geometricAtten = 1/sqrt(radius);
    else
        geometricAtten = 1/radius;


    // Compute the FAS by assembling the different parts
    for (int i=0; i < m_fas.size(); ++i) {
        // Source effects
        double source = 0.78 * (M_PI/(rho*pow(beta,3))) * seismicMoment * 
            pow(m_freq.at(i),2) / ( 1 + pow(m_freq.at(i)/cornerFreq,2));
        // Quality -- seismological damping
        double quality = 200 * pow(m_freq.at(i), 0.60);
        // Path effects
        double path = geometricAtten * exp((-M_PI * m_freq.at(i) * radius)/(quality*beta));
        // High frequency length()unution
        double freqDim = exp( -M_PI * kappa * m_freq.at(i));

        // Assemble the parts  FIXME the constant may need to get corrected
        m_fas[i] = 10e-21/981 * source * path * freqDim * rockAmp.at(i);
    }
}

void RvtMotion::simpleFas()
{
    // Create a simple Fourier amplitude spectrum
    QVector<double> slope;
    QVector<double> freq;
    slope << 2 << 0 << -0.25;
    freq << 10 << 0.7;
    double initialLogY = log10(5e-5);
    double initialLogX = log10(m_freq.first());
    int idx = 0;

    for (int i = 0; i < slope.size(); ++i)
    {
        if ( i < freq.size()) {
            while ( m_freq.at(idx) > freq.at(i) ) {
                m_fas[idx] = pow( 10, slope.at(i) * (initialLogX - log10(m_freq.at(idx))) + initialLogY);
                ++idx;
            }
            // Save the final value as the next initial value
            initialLogY = log10(m_fas[idx-1]);
            initialLogX = log10(m_freq[idx-1]);

        } else {
            while ( idx < m_fas.size() ) {
                // No ending frequency continue until the end
                m_fas[idx] = pow( 10, slope.at(i) * (initialLogX - log10(m_freq.at(idx))) + initialLogY);
                ++idx;
            }
        }
    }
} 

void RvtMotion::vanmarckeInversion(const QVector<double> & sa, double damping )
{
    Q_ASSERT( sa.size() == m_fas.size() );

    // Assume a constant peak factor
    double peakFactor = 3.5;
    double prevSpecDensity = 0;
    double specDensity = 0;
    double sum = 0;
    double duration = durationOfGm(Motion::Rock);

    for ( int i = 0; i < m_fas.size(); ++i ) {
        // Compute spectral density
        specDensity = ((pow(sa.at(i),2) / pow(peakFactor,2)) - 2 * M_PI * sum ) /
            ( angFreqAt(i) * ( M_PI / ( 4 * damping / 100 ) - 1) );

        if ( specDensity < 0 )
            specDensity = prevSpecDensity;

        // Convert from spectral density into FAS
        m_fas[i] = sqrt(specDensity * M_PI * duration);

        if ( i == 0 )
            sum = specDensity * m_freq.at(i) / 2;
        else
            sum += ( specDensity - prevSpecDensity ) / 2 * ( m_freq.at(i) - m_freq.at(i-1) );

        prevSpecDensity = specDensity;
    }
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

// The integral of this function is used to compute the peak factor
double RvtMotion::calcPeakFactor( const double bandWidth, const double numExtrema) const
{
    // Compute the peak factor
    double peakFactor = 0;
    double delta = 0.005;
    double z = 0;
    double last = 1.0;
    double current = 0.0;
    // Integrate using the trapezoid rule
    do {
        z += delta;
        // Compute the current piece of the peak factor
        current = 1 - pow( 1 - bandWidth * exp(-z*z), numExtrema);
        peakFactor += delta * (last+current) / 2.0;
        // Save the current value
        last = current;
    } while( last/peakFactor > 0.000001 );

    return M_SQRT2 * peakFactor;
}

double RvtMotion::rvt( const QVector<double> & fas, double durationGm, double durationRms ) const
{
    if ( durationRms < 0 )
        durationRms = durationGm;

    QVector<double> fasSqr(fas.size());
    for ( int i = 0; i < fasSqr.size(); ++i )
        fasSqr[i] = fas.at(i) * fas.at(i);

    // QFile file("output.csv");
    // file.open(QIODevice::WriteOnly | QIODevice::Text);
    // QTextStream out(&file);
    // for ( int i = 0; i < m_freq.size(); ++i)
    //     out << m_freq.at(i) << "," << fasSqr.at(i) << endl;
    
    // The zero moment is the area of the spectral density computed by the
    // trapezoid rule.
    double moment_0 = moment( 0, fasSqr);
    double moment_2 = moment( 2, fasSqr);
    double moment_4 = moment( 4, fasSqr);

    // Compute the bandwidth
    double bandWidth = sqrt((moment_2 * moment_2 )/( moment_0 * moment_4));

    // Compute the number extrema 
    double numExtrema = sqrt(moment_4/moment_2) * durationGm / M_PI;

    // If the number of extrema is less than 2, increase to 2.  There must be
    // one full cycle (two peaks)
    if (numExtrema <2)
        numExtrema = 2;

    // Compute the peak factor based on the bandwidth and number of extrema
    double peakFactor = calcPeakFactor( bandWidth, numExtrema );

    // Return the peak value which is found by multiplying the variation by the peakfactor
    return sqrt(moment_0/durationRms) * peakFactor;
}
