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

#include "AbstractPeakCalculator.h"

#include <QDebug>

#include <cmath>


AbstractPeakCalculator::AbstractPeakCalculator()
{

}

AbstractPeakCalculator::~AbstractPeakCalculator()
{

}

auto AbstractPeakCalculator::limitZeroCrossings(double num) const -> double {
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

auto
AbstractPeakCalculator::calcPeak(double duration,
                                 const QVector<double> &freqs,
                                 const QVector<double> &fourierAmps,
                                 double oscFreq,
                                 double oscDamping,
                                 const QVector<std::complex<double> > &siteTransFunc) -> double {
    initCache(freqs, fourierAmps);
    double peakFactor = calcPeakFactor(duration, oscFreq, oscDamping);
    double durationRms = calcDurationRms(duration, oscFreq, oscDamping, siteTransFunc);    
    double respRms = std::sqrt(getMoment(0) / durationRms);
    clearCache();
    return peakFactor * respRms;
}

auto AbstractPeakCalculator::getMoment(int power) -> double {
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

auto AbstractPeakCalculator::calcDurationRms(double duration,
                                               double oscFreq,
                                               double oscDamping,
                                               const QVector<std::complex<double> > &siteTransFunc) -> double {
    Q_UNUSED(oscFreq);
    Q_UNUSED(oscDamping);
    Q_UNUSED(siteTransFunc);

    return duration;
}
