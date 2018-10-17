//
// Created by albert on 1/18/18.
//

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
    validTransFunc &= siteTransFunc.size() == _freqs.size();
    // Make sure there are some values greater than 1
    if (validTransFunc) {
        for (std::complex<double> stf : siteTransFunc) {
            if (abs(abs(stf) - 1.) > 1E-1) {
                validTransFunc = true;
                break;
            }
        }
    }

    // Modify the duration for the soil response
    if (validTransFunc) {
        QList<double> modeFreqs;
        QList<double> modeAmps;

        QVector<double> absSiteTransFunc;
        for (std::complex<double> stf : siteTransFunc) {
            absSiteTransFunc << abs(stf);
        }

        // Compute the location of the peaks in the transfer function
        const int offset = 2;
        bool valid;
        for (int i = offset; i < (_freqs.size() - offset); ++i) {
            valid = true;
            qDebug() << _freqs.at(i) << absSiteTransFunc.at(i);

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

        double a;
        double c;
        double m;
        double incr = 0;
        for (int i = 0; i < modeFreqs.size(); ++i) {
            c = _coefs.at(i).a * afRatio + _coefs.at(i).b * std::pow(afRatio, 2);
            m = _coefs.at(i).d * afRatio + _coefs.at(i).e * std::pow(afRatio, 2);
            a = c * std::exp(-duration / m);
            incr += (a * std::exp(-std::pow(std::log(oscFreq / modeFreqs.at(i)), 2) /
                    (2 * std::pow(_coefs.at(i).sd, 2)))
            );
        }
        durationRms += incr;
    }
    return durationRms;
}
