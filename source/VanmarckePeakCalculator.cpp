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

auto calcCCDF(double x, void *params) -> double {
    auto *p = static_cast<struct ccdfParams *>(params);
    return (1 -
            (1 - exp(-(x * x) / 2)) *
            exp(-1 * p->zeroCrossings *
                (1 - exp(-1 * sqrt(M_PI_2) * p->bandwidthEff * x)) /
                (exp((x * x) / 2) - 1)));
}

auto VanmarckePeakCalculator::calcPeakFactor(double duration, double oscFreq, double oscDamping) -> double {
    Q_UNUSED(oscFreq);
    Q_UNUSED(oscDamping);

    // Compute the root - mean - squared response
    double m0 = getMoment(0);
    double m1 = getMoment(1);
    double m2 = getMoment(2);

    double bandwidth = sqrt(1 - (m1 * m1) / (m0 * m2));
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
