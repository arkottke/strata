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

#include "WangRathjePeakCalculator.h"
#include "BooreThompsonPeakCalculator.h"

#include <QDebug>


WangRathjePeakCalculator::WangRathjePeakCalculator() {
    _coefs << WangRathjeCoef{1,  0.2688,  0.0030,  1.8380, -0.0198, 0.091}
            << WangRathjeCoef{2,  0.2555, -0.0002,  1.2154, -0.0183, 0.081}
            << WangRathjeCoef{3,  0.2287, -0.0014,  0.9404, -0.0130, 0.056};
}

double WangRathjePeakCalculator::calcDurationRms(
        double duration,
        double oscFreq,
        double oscDamping,
        const QVector<std::complex<double> > &siteTransFunc)
{    
    // Compute the Boore & Thompson (2015) correction for oscillator response
    double durationRms = BooreThompsonPeakCalculator::calcDurationRms(
                duration, oscFreq, oscDamping, siteTransFunc);

    bool validTransFunc = false;
    // Make sure there are some values greater than 1.1
    if (siteTransFunc.size() == _freqs.size()) {
        for (std::complex<double> stf : siteTransFunc) {
            if (abs(stf) > 1.1) {
                validTransFunc = true;
                break;
            }
        }
    }

    // Modify the duration for the soil response
    if (validTransFunc) {
        // Compute the expected rock oscillator duration

        // Equation 4a
        const double fLim = 5.274 * std::pow(duration, -0.640);
        double ratio = 1;
        // Equation 2
        if (0.1 <= oscFreq && oscFreq < fLim) {
            // Equation 4b
            const double dur0 = 31.858 * std::pow(duration, -0.849);
            // Equation 4c
            const double durMin = 1.009 * duration / (3.583 + duration);
            // Equation 3b
            const double b = 1 / (dur0 - durMin);
            // Equation 3a
            const double a = (1 / (dur0 - 1) - b) * (fLim - 0.1);
            // Equation 2
            ratio = (dur0 - (oscFreq - 0.1) / (a + b * (oscFreq - 0.1)));
        }

        const double durOscRock = ratio * duration;

        // Compute the expected soil oscillator duration
        QList<double> modeFreqs;
        QList<double> modeAmps;

        QVector<double> absSiteTransFunc(siteTransFunc.size());
        for (int i = 0; i < absSiteTransFunc.size(); ++i) {
            absSiteTransFunc[i] = std::abs(siteTransFunc.at(i));
        }

        // Compute the location of the peaks in the transfer function
        const int offset = 2;
        bool valid;
        for (int i = offset; i < (_freqs.size() - offset); ++i) {
            valid = true;
            for (int j = (i - offset); j <= (i + offset); ++j) {
                if (j < i) {
                    valid &= absSiteTransFunc.at(j) < absSiteTransFunc.at(i);
                } else if (i < j) {
                    valid &= absSiteTransFunc.at(i) > absSiteTransFunc.at(j);
                }
            }

            if (valid) {
                modeFreqs << _freqs.at(i);
                modeAmps << absSiteTransFunc.at(i);
            }

            if (modeFreqs.size() > 2) {
                break;
            }
        }
        // Amplitude / frequency ratio of the first mode
        double afRatio = modeAmps.at(0) / modeFreqs.at(0);

        double incrMax;
        double c;
        double m;
        double incr = 0;
        for (int i = 0; i < modeFreqs.size(); ++i) {
            c = _coefs.at(i).a * afRatio + _coefs.at(i).b * std::pow(afRatio, 2);
            m = _coefs.at(i).d * afRatio + _coefs.at(i).e * std::pow(afRatio, 2);
            incrMax = c * std::exp(-duration / m);
            incr += (incrMax * std::exp(-std::pow(std::log(oscFreq / modeFreqs.at(i)), 2) /
                    (2 * std::pow(_coefs.at(i).sd, 2)))
            );
        }
        const double durOscSoil = durOscRock + incr;
        durationRms *= (durOscSoil / durOscRock);
    }
    return durationRms;
}
