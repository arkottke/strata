#include "VanmarckePeakCalculator.h"

#include <QDebug>

VanmarckePeakCalculator::VanmarckePeakCalculator() {
    _name = "Vanmarcke (1975)";
    _workspace = gsl_integration_workspace_alloc(1000);
}

VanmarckePeakCalculator::~VanmarckePeakCalculator() {
    gsl_integration_workspace_free(_workspace);
}

struct ccdfParams {
    double zeroCrossings;
    double bandwidthEff;
};

double calcCCDF(double x, void *params) {
    struct ccdfParams *p = (struct ccdfParams *) params;
    return (1 -
            (1 - exp(-(x * x) / 2)) *
            exp(-1 * p->zeroCrossings *
                (1 - exp(-1 * sqrt(M_PI / 2) * p->bandwidthEff * x)) /
                (exp((x * x) / 2) - 1)));
}

double VanmarckePeakCalculator::calcPeakFactor(double duration, double oscFreq, double oscDamping) {
    Q_UNUSED(oscFreq);
    Q_UNUSED(oscDamping);

    // Compute the root - mean - squared response
    double m0 = getMoment(0);
    double m1 = getMoment(1);
    double m2 = getMoment(2);

    double bandwidth = sqrt(1 - pow(m1, 2) / (m0 * m2));
    double bandwidthEff = pow(bandwidth, 1.2);

    double zeroCrossings =
            limitZeroCrossings(duration * sqrt(m2 / m0) * M_1_PI);
    // The expected peak factor is computed as the integral of the complementary
    // CDF(1 - CDF(x)).
    double peakFactor, error;
    struct ccdfParams params = {zeroCrossings, bandwidthEff};

    gsl_function F;
    F.function = &calcCCDF;
    F.params = &params;

    gsl_integration_qagiu(&F, 0, 1e-4, 1e-4, 1000, _workspace, &peakFactor, &error);
    return peakFactor;
}
