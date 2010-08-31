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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "FrequencyDependentCalculator.h"

#include "AbstractMotion.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

#include <QDebug>

#include <cmath>

FrequencyDependentCalculator::FrequencyDependentCalculator(QObject* parent)
    : AbstractIterativeCalculator(parent)
{
    reset();
}

QString FrequencyDependentCalculator::toHtml() const
{
    return tr(
            "<li>Frequency Dependent Equivalent Linear Parameters"
            "<table border=\"0\">"
            "<tr><th>Error tolerance:</th><td>%2</td></tr>"
            "<tr><th>Maximum number of iterations:</th><td>%3</td></tr>"
            "</table>"
            "</li>"
            )
            .arg(m_errorTolerance)
            .arg(m_maxIterations);
}

bool FrequencyDependentCalculator::updateSubLayer(int index, const QVector<std::complex<double> > strainTf)
{
    const double strainMax = 100 * Units::instance()->gravity() * m_motion->calcMaxStrain(strainTf);

    if (strainMax <= 0)
        return false;

    const QVector<double> strainFas = m_motion->absFourierVel(strainTf);

    const QVector<double> &freq = m_motion->freq();

    // Compute the mean frequency and mean strain parameters defined by Kausel and Assimaki (2002)
    double numer = 0;
    double denom = 0;
    double dFreq;

    for (int i = 1; i < freq.size(); ++i) {
        dFreq = freq.at(i) - freq.at(i-1);
        numer += dFreq * (freq.at(i-1) * strainFas.at(i-1)
                          + freq.at(i) * strainFas.at(i)) / 2.;
        denom += dFreq * (strainFas.at(i-1) + strainFas.at(i)) / 2.;
    }

    const double freqAvg = numer / denom;

    double sum = 0;
    int offset = 1;

    while (freq.at(offset) < freqAvg) {
        dFreq = freq.at(offset) - freq.at(offset-1);
        sum += dFreq * (strainFas.at(offset-1) + strainFas.at(offset)) / 2.;
        ++offset;

        Q_ASSERT(offset < freq.size());
    }

    const double strainAvg = sum / freqAvg;

    // Update the sublayer with the representative strain -- FIXME strainAvg doesn't appear to be representative
    m_site->subLayers()[index].setStrain(strainMax, strainMax);

    // Calculate model parameter using a least squares fit
    const int n = m_nf - offset;
    double chisq;
    gsl_multifit_linear_workspace* work = gsl_multifit_linear_alloc(n, 2);
    gsl_matrix* model = gsl_matrix_alloc(n, 2);
    gsl_vector* data = gsl_vector_alloc(n);
    gsl_vector* params = gsl_vector_alloc(2);
    gsl_matrix* cov = gsl_matrix_alloc(2, 2);

    for (int i = 0; i < n; ++i) {
        gsl_matrix_set(model, i, 0, -freq.at(i + offset) / freqAvg);
        gsl_matrix_set(model, i, 1, -log(freq.at(i + offset) / freqAvg));

        gsl_vector_set(data, i, log(strainFas.at(i + offset) / strainAvg));
    }        

    gsl_multifit_linear(model, data, params, cov, &chisq, work);

    const double alpha = gsl_vector_get(params, 0);
    const double beta = gsl_vector_get(params, 1);

    // Clean up
    gsl_multifit_linear_free(work);
    gsl_matrix_free(model);
    gsl_vector_free(data);
    gsl_vector_free(params);
    gsl_matrix_free(cov);

    // Compute the complex shear modulus and complex shear-wave velocity
    // for each soil layer -- these change because the damping and shear
    // modulus change.
    double modulus;
    double damping;
    double strain;
    const SubLayer &sl = m_site->subLayers().at(index);

    for (int i = 0; i < m_nf; ++i) {
        // Compute the strain from the function
        if (freq.at(i) < freqAvg) {
            strain = 1.;
        } else {
            strain = exp(-alpha * freq.at(i) / freqAvg)
                     / pow(freq.at(i) / freqAvg, beta);
        }
        // Scale strain by maximum strain
        strain *= strainMax;

        sl.interp(strain, &modulus, &damping);
        m_shearMod[index][i] = calcCompShearMod(modulus, damping / 100.);
    }

    return true;
}


QDataStream & operator<<(QDataStream & out,
                                 const FrequencyDependentCalculator* fdc)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractIterativeCalculator*>(fdc);

    return out;
}

QDataStream & operator>>(QDataStream & in,
                                 FrequencyDependentCalculator* fdc)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractIterativeCalculator*>(fdc);

    return in;
}
