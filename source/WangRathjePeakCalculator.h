//
// Created by albert on 1/18/18.
//

#ifndef WANG_RATHJE_PEAK_CALCULATOR_H
#define WANG_RATHJE_PEAK_CALCULATOR_H

#include "BooreThompsonPeakCalculator.h"

struct WangRathjeCoef {
    int mode;
    double a;
    double b;
    double d;
    double e;
    double sd;
};

class WangRathjePeakCalculator : public BooreThompsonPeakCalculator {

public:
    WangRathjePeakCalculator();

    virtual double calcDurationRms(
            double duration,
            double oscFreq,
            double oscDamping,
            const QVector<std::complex<double> > &siteTransFunc);
protected:

    QList<WangRathjeCoef> _coefs;
};

#endif //WANG_RATHJE_PEAK_CALCULATOR_H
