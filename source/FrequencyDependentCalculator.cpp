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

#include "FrequencyDependentCalculator.h"

#include "EquivalentLinearCalculator.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"

#include <cmath>

#include <QDebug>

FrequencyDependentCalculator::FrequencyDependentCalculator(QObject* parent)
    : AbstractIterativeCalculator(parent)
{
    _name = "EQL-FDM";
    _useSmoothSpectrum = false;
    reset();
}

auto FrequencyDependentCalculator::useSmoothSpectrum() -> bool {
    return _useSmoothSpectrum;
}

void FrequencyDependentCalculator::setUseSmoothSpectrum(bool b) {
    if (_useSmoothSpectrum != b) {
        _useSmoothSpectrum = b;
        emit useSmoothSpectrumChanged(b);
        emit wasModified();
    }
}

auto FrequencyDependentCalculator::toHtml() const -> QString
{
    return tr(
            "<li>Frequency Dependent Equivalent Linear Parameters"
            "<table border=\"0\">"
            "<tr><th>Error tolerance:</th><td>%2</td></tr>"
            "<tr><th>Maximum number of iterations:</th><td>%3</td></tr>"
            "</table>"
            "</li>"
            )
            .arg(_errorTolerance)
            .arg(_maxIterations);
}

auto FrequencyDependentCalculator::updateSubLayer(
        int index,
        const QVector<std::complex<double> > &strainTf) -> bool
{
    const double strainMax = 100 * _motion->calcMaxStrain(strainTf);

    if (strainMax <= 0) {
        return false;
    }

    const QVector<double> strainFas = _motion->absFourierVel(strainTf);
    const QVector<double> &freq = _motion->freq();

    // Update the sublayer with the representative strain -- FIXME strainAvg doesn't appear to be representative
    _site->subLayers()[index].setStrain(strainMax, strainMax);

    double shearMod;
    double damping;
    double strain;
    const SubLayer &sl = _site->subLayers().at(index);

    if (_useSmoothSpectrum) {
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

        // Calculate model parameter using a least squares fit
        const int n = _nf - offset;
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
        for (int i = 0; i < _nf; ++i) {
            // Compute the strain from the function
            // Use a slightly different formulation from Assimaki and Kausel to provide a smooth taper
            strain = std::min(1.0, exp(-alpha * freq.at(i) / freqAvg)
                    / pow(freq.at(i) / freqAvg, beta));
            // Scale strain by maximum strain
            strain *= strainMax;

            sl.interp(strain, &shearMod, &damping);
            _shearMod[index][i] = calcCompShearMod(shearMod, damping / 100.);
        }

        return true;
    } else {
        double maxFas = 0;
        for (const double &d : strainFas) {
            maxFas = std::max(d, maxFas);
        }
        for (int i = 0; i < _nf; ++i) {
            strain = strainMax * strainFas.at(i) / maxFas;
            sl.interp(strain, &shearMod, &damping);
            _shearMod[index][i] = calcCompShearMod(shearMod, damping / 100.);
        }
    }

    return true;
}

void FrequencyDependentCalculator::estimateInitialStrains()
{
    if ( _textLog->level() > TextLog::Low ) {
        _textLog->append(tr("\t\tEstimating strains using EQL method"));
    }

    auto *calc = new EquivalentLinearCalculator();
    calc->setMaxIterations(_maxIterations);
    calc->setTextLog(_textLog);
    calc->run(_motion, _site);

    for (SubLayer &sl : _site->subLayers()) {
        sl.setInitialStrain(sl.effStrain());
    }

    // Compute the complex shear modulus and complex shear-wave velocity for
    // each soil layer -- initially this is assumed to be frequency independent
    for (int i = 0; i < _nsl; ++i ) {
        _shearMod[i].fill(calcCompShearMod(
                _site->shearMod(i), _site->damping(i) / 100.));
    }

    calc->deleteLater();
}


void FrequencyDependentCalculator::fromJson(const QJsonObject &json)
{
    AbstractIterativeCalculator::fromJson(json);
}

auto FrequencyDependentCalculator::toJson() const -> QJsonObject
{
    return AbstractIterativeCalculator::toJson();
}

auto operator<<(QDataStream & out,
                                 const FrequencyDependentCalculator* fdc) -> QDataStream &
{
    out << (quint8)2;

    out << qobject_cast<const AbstractIterativeCalculator*>(fdc);
    out << fdc->_useSmoothSpectrum;

    return out;
}

auto operator>>(QDataStream & in,
                                 FrequencyDependentCalculator* fdc) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractIterativeCalculator*>(fdc);
    if (ver > 1) {
        in >> fdc->_useSmoothSpectrum;
    }

    return in;
}
