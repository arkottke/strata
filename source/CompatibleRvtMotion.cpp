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

#include "CompatibleRvtMotion.h"

#include "Dimension.h"
#include "ResponseSpectrum.h"
#include "RvtMotion.h"

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include <QDebug>

CompatibleRvtMotion::CompatibleRvtMotion(QObject* parent)
        : AbstractRvtMotion(parent)
{
    _freq = new Dimension(this);
    _freq->setMin(0.05);
    _freq->setMax(50);
    _freq->setSize(1024);
    _freq->setSpacing(Dimension::Log);

    _fourierAcc = QVector<double>(freq().size(), 0.);

    _duration = 5.0;
    _limitFas = false;

    _targetRespSpec = new ResponseSpectrum;
    _targetRespSpec->setDamping(5.0);

    _name = tr("Compatible RVT Motion (M $mag @ $dist km)");
}

CompatibleRvtMotion::~CompatibleRvtMotion()
{
    _targetRespSpec->deleteLater();
}

auto CompatibleRvtMotion::freq() const -> const QVector<double> &
{
    return _freq->data();
}

auto  CompatibleRvtMotion::freqDimension() -> Dimension*
{
    return _freq;
}

auto CompatibleRvtMotion::targetRespSpec() -> ResponseSpectrum *
{
    return _targetRespSpec;
}

auto CompatibleRvtMotion::limitFas() const -> bool
{
    return _limitFas;
}

void CompatibleRvtMotion::setLimitFas(bool limitFas)
{
    if ( _limitFas != limitFas ) {
        _modified = true;
        emit wasModified();
    }

    _limitFas = limitFas;
}

void CompatibleRvtMotion::setDuration(double duration)
{
    if (_duration != duration) {
        setModified(true);
    }

    _duration = duration;
}

auto CompatibleRvtMotion::toHtml() const -> QString
{
    //FIXME

    return QString();
}

auto CompatibleRvtMotion::loadFromTextStream(QTextStream &stream, double scale) -> bool
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

    _targetRespSpec->setPeriod(period);
    _targetRespSpec->setSa(sa);

    calculate();

    return true;
}

void CompatibleRvtMotion::calculate()
{
    beginResetModel();
    // Check that both period and sa have some length
    if ( _targetRespSpec->period().size() == 0 ||
         _targetRespSpec->sa().size() != _targetRespSpec->period().size()
            ) {
        qCritical("Both period and sa must be defined.");
        return;
    }

    // Check that the given period is constantly increasing
    for( int i = 0; i < _targetRespSpec->period().size() - 1; ++i) {
        if( _targetRespSpec->period().at(i) > _targetRespSpec->period().at(i+1) ) {
            qCritical("The given period must be increasing");
            return;
        }
    }

    for (auto &period : _targetRespSpec->period()) {
        if (period <= 0) {
            qCritical("Target periods must be greater than zero");
        }
    }

    //
    // Estimate the FAS using the methodology proposed by Vanmarcke
    //
    const QVector<double> estimateFas = vanmarckeInversion();

    // Interpolate the FAS using a cubic spline, extrapolate at low frequencies.
    const double targetMinFreq = 1. / _targetRespSpec->period().last();

//    _freq = Dimension::logSpace( qMin(targetMinFreq / 2., 0.05), _maxEngFreq, 1024 );
//    _fourierAcc.resize(freq().size());
    int offset = 0;

    gsl_interp_accel * acc = gsl_interp_accel_alloc ();
    gsl_spline * spline = gsl_spline_alloc(gsl_interp_cspline, _targetRespSpec->period().size());

    gsl_spline_init (spline, _targetRespSpec->period().data(),
                     estimateFas.data(), _targetRespSpec->period().size());

    const double logFas0 = log(estimateFas.first());
    const double freq0 = 1 / _targetRespSpec->period().last();

    for (int i = 0; i < freq().size(); ++i) {
        if (freq().at(i) < targetMinFreq) {
            // Linearly extrapolate in log-log space.  This extrapolation is
            // not very rigorous, but it is just used to develop an initial
            // estimate of the FAS.
            _fourierAcc[i] = exp(1.92 * log(freq().at(i)/freq0) + logFas0);
            offset = i;
        } else {
            _fourierAcc[i] = gsl_spline_eval(spline, 1. / freq().at(i), acc);
        }
    }

    ++offset;

    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);

    // Interpolation for the ratio between target and calculated response spectrum
    acc = gsl_interp_accel_alloc();
    spline = gsl_spline_alloc(gsl_interp_cspline, _targetRespSpec->period().size());

    // Loop over ratio correction while the rmse error is larger than FIXME
    _okToContinue = true;
    double rmse = 0;
    double maxError = 0;
    double oldRmse = 1;
    int maxCount = 30;
    //double minRmse = 0.02;
    double minRmse = 0.005;
    double minRmseChange = 0.0002;
    int count = 0;

    // Initial response spectrum
    _respSpec->setDamping(_targetRespSpec->damping());
    _respSpec->setPeriod(_targetRespSpec->period());
    _respSpec->setSa(computeSa(_targetRespSpec->period(), _targetRespSpec->damping()));

    QVector<double> ratio(_respSpec->sa().size());

    // Set the maximum value of the progress
    emit progressRangeChanged(0, maxCount);

    do {
        emit progressValueChanged(count);

        // Determine the correction ratio
        for (int i = 0; i < ratio.size(); ++i)
            ratio[i] = _targetRespSpec->sa().at(i) / _respSpec->sa().at(i);

        // Apply the ratio correction to the FAS
        gsl_spline_init(spline, _respSpec->period().data(), ratio.data(), ratio.size());

        for (int i = offset; i < freq().size(); ++i) {
            _fourierAcc[i] *= gsl_spline_eval(spline, 1. / freq().at(i), acc);
        }

        // Extrapolate the low frequency values
        double logFreq0 = log(freq().at(offset));
        double logFas0 = log(_fourierAcc.at(offset));

        // The theoretical slope at low frequenc
        double slope = _limitFas ? 2 :
                       (log(_fourierAcc.at(offset) / _fourierAcc.at(offset + 1))
                        / log(freq().at(offset) / freq().at(offset + 1)));

        for (int i = 0; i < offset; ++i) {
            _fourierAcc[i] = exp(slope * (log(freq().at(i)) - logFreq0) + logFas0);
        }

        // Force down the high frequency tail

        // Find the minimum slope
        if (_limitFas) {
            double minSlope = 0;
            int minSlopeIdx = 0;
            int i = offset;

            while (i < freq().size() - 1) {
                slope = log(_fourierAcc.at(i) / _fourierAcc.at(i + 1)) /
                        log(freq().at(i) / freq().at(i + 1));
                if (slope < minSlope) {
                    minSlope = slope;
                    minSlopeIdx = i;
                }
                ++i;
            }

            // Extrapolate from deviation
            i = minSlopeIdx;
            double x0 = log(freq().at(i));
            double y0 = log(_fourierAcc.at(i));
            //double kappa0 = exp( -M_PI * 0.01 * freq().at(idx) );

            ++i;
            while (i < _fourierAcc.size()) {
                // Extrapolate the value based, but reduce the value using a kappa filter
                //_fourierAcc[idx] = exp( cutoff * (log(freq().at(idx)) - x0 ) + y0 ) * exp(-M_PI * 0.01 * freq().at(idx) ) / kappa0 ;
                _fourierAcc[i] = exp(-slope * (log(freq().at(i)) - x0) + y0);
                ++i;
            }
        }

        // Re-compute the Sa
        _respSpec->setSa(computeSa(_targetRespSpec->period(), _targetRespSpec->damping()));

        // Compute the root-mean-squared error
        double sumError = 0;
        for (int i = 0; i < _respSpec->sa().size(); ++i) {
            const double e = (_respSpec->sa().at(i) - _targetRespSpec->sa().at(i)) /
                             _targetRespSpec->sa().at(i);

            // Save the maximum error
            if (abs(maxError) < abs(e)) {
                maxError = e;
            }

            // Add the squared error to the sum
            sumError += e * e;
        }
        rmse = sqrt(sumError / _respSpec->sa().size());

        // Increment the count
        ++count;

        // Stop if the RMSE is below the specified RMSE
        if (rmse < minRmse || abs(oldRmse - rmse) < minRmseChange) {
            break;
        }
        // Allow the user to cancel the operation
        if (!_okToContinue) {
            break;
        }
        // Save old rmse
        oldRmse = rmse;
    } while (count < maxCount);

    // Delete the interpolators
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);

    // Set progress at the maximum
    emit progressValueChanged(maxCount);

    // Reset the modified target flag
    _targetRespSpec->setModified(false);

    // Signal that the changes have taken place
    endResetModel();

    AbstractRvtMotion::calculate();
}

auto CompatibleRvtMotion::vanmarckeInversion() const -> QVector<double>
{
    if (_targetRespSpec->sa().size() < 10) {
        // Loglog interpolate prior to performing the inversion
        QVector<double> targetPeriod = _targetRespSpec->period();
        QVector<double> targetSa = _targetRespSpec->sa();
        const double decades = log10(targetPeriod.last() / targetPeriod.first());

        QVector<double> period = Dimension::logSpace(
                targetPeriod.first(), targetPeriod.last(),
                int(10 * decades));

        QVector<double>sa;
        logLogInterp(
                _targetRespSpec->period(), _targetRespSpec->sa(),
                period, sa);

        _targetRespSpec->setPeriod(period);
        _targetRespSpec->setSa(sa);
    }

    QVector<double> fas(_targetRespSpec->period().size());

    // Assume a constant peak factor
    double peakFactor = 2.5;
    double prevFasSqr = 0;
    double fasSqr = 0;
    double sum = 0;

    // Single degree of freedom factor
    double sdofFactor = M_PI / ( 4 * _targetRespSpec->damping() / 100. ) - 1;
    const double damping = _targetRespSpec->damping();

    for ( int i = fas.size()-1; i > -1; --i ) {
        double freq = 1 / _targetRespSpec->period().at(i);
        double sa = _targetRespSpec->sa().at(i);
        const double rmsDuration = _peakCalculator->calcDurationRms(
                _duration, freq, damping, QVector<std::complex<double> >());

        // Compute the squared Fourier amplitude spectrum
        fasSqr = ( ( rmsDuration * pow(sa,2) ) / ( 2 * pow( peakFactor, 2) ) - sum ) /
                 ( freq * sdofFactor );

        if ( fasSqr < 0 )
            fasSqr = prevFasSqr;

        // Convert from spectral density into FAS
        fas[i] = sqrt(fasSqr);

        if ( i == fas.size()-1 ) {
            sum = fasSqr * freq / 2;
        } else {
            sum += (fasSqr - prevFasSqr) / 2 * (freq - (1 / _targetRespSpec->period().at(i + 1)));
        }
        prevFasSqr = fasSqr;
    }

    return fas;
}

void CompatibleRvtMotion::fromJson(const QJsonObject &json)
{
    AbstractRvtMotion::fromJson(json);
    _limitFas = json["limitFas"].toBool();
    _freq->fromJson(json["freq"].toObject());
    _targetRespSpec->fromJson(json["targetRespSpec"].toObject());
    calculate();
}

auto CompatibleRvtMotion::toJson() const -> QJsonObject
{
    QJsonObject json = AbstractRvtMotion::toJson();
    json["freq"] = _freq->toJson();
    json["limitFas"] = _limitFas;
    json["targetRespSpec"] = _targetRespSpec->toJson();
    return json;
}

auto operator<< (QDataStream & out, const CompatibleRvtMotion* crm) -> QDataStream &
{
    out << (quint8)1;

    out << qobject_cast<const AbstractRvtMotion*>(crm);
    out << crm->_freq << crm->_limitFas << crm->_targetRespSpec;

    return out;
}

auto operator>> (QDataStream & in, CompatibleRvtMotion* crm) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractRvtMotion*>(crm);
    in >> crm->_freq >> crm->_limitFas >> crm->_targetRespSpec;

    crm->calculate();
    return in;
}
