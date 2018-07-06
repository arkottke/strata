#ifndef VANMARCKEPEAKCALCULATOR_H
#define VANMARCKEPEAKCALCULATOR_H

#include "AbstractPeakCalculator.h"

#include <gsl/gsl_integration.h>

class VanmarckePeakCalculator : public AbstractPeakCalculator {
public:
    explicit VanmarckePeakCalculator();
    ~VanmarckePeakCalculator();


protected:
    double calcPeakFactor(double duration, double oscFreq, double oscDamping);

    gsl_integration_workspace *_workspace;
};

#endif // VANMARCKEPEAKCALCULATOR_H
