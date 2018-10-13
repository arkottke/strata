
#include "AbstractPeakCalculator.h"

#include <QDebug>

#include <cmath>


AbstractPeakCalculator::AbstractPeakCalculator()
{

}

AbstractPeakCalculator::~AbstractPeakCalculator()
{

}

double AbstractPeakCalculator::limitZeroCrossings(double num) const {
    return std::max(num, 1.33);
}

void AbstractPeakCalculator::initCache(const QVector<double> &freqs,
                                       const QVector<double> &fourierAmps) {
    _momentCache.clear();
    _freqs = freqs;
    _squaredAmps.clear();
    for (double fa : fourierAmps) {
        _squaredAmps << fa * fa;
    }
}

void AbstractPeakCalculator::clearCache() {
    _freqs.clear();
    _squaredAmps.clear();
    _momentCache.clear();
}

double
AbstractPeakCalculator::calcPeak(double duration,
                                 const QVector<double> &freqs,
                                 const QVector<double> &fourierAmps,
                                 double oscFreq,
                                 double oscDamping,
                                 const QVector<std::complex<double> > &siteTransFunc) {
    initCache(freqs, fourierAmps);
    double peakFactor = calcPeakFactor(duration, oscFreq, oscDamping);
    double durationRms = calcDurationRms(duration, oscFreq, oscDamping, siteTransFunc);    
    double respRms = std::sqrt(getMoment(0) / durationRms);
    clearCache();
    return peakFactor * respRms;
}

double AbstractPeakCalculator::getMoment(int power) {
    double moment = 0;
    if (_momentCache.contains(power)) {
        moment = _momentCache[power];
    } else {
        // Compute the moment
        double left = pow(2 * M_PI * _freqs.at(0), power) * _squaredAmps.at(0);
        double right;
        double dFreq;
        moment = 0;
        for (int i = 1; i < _freqs.size(); ++i) {
            right = pow(2 * M_PI * _freqs.at(i), power) * _squaredAmps.at(i);
            dFreq = abs(_freqs.at(i) - _freqs.at(i - 1));
            /*
             * For typical trapezoidal integration, we would divide by 2 (average),
             * but for the moment calculation the value is multiplied by 2 at the end.
             */
            moment += dFreq * (right + left);
            left = right;
        }
        _momentCache[power] = moment;
    }

    return moment;
}

double AbstractPeakCalculator::calcDurationRms(double duration,
                                               double oscFreq,
                                               double oscDamping,
                                               const QVector<std::complex<double> > &siteTransFunc) {
    Q_UNUSED(oscFreq);
    Q_UNUSED(oscDamping);
    Q_UNUSED(siteTransFunc);

    return duration;
}
