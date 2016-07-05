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

#include "DarendeliNonlinearProperty.h"

#include "Dimension.h"
#include "SoilType.h"

#include <gsl/gsl_math.h>

DarendeliNonlinearProperty::DarendeliNonlinearProperty(Type type, QObject *parent)
    : NonlinearProperty(parent)
{
    m_name = "Darendeli & Stokoe (2001)";
    m_type = type;
    m_strain = Dimension::logSpace(pow(10.,-4), pow(10.,0.5), 19);
    m_average.resize(m_strain.size());
    initialize();
}

void DarendeliNonlinearProperty::calculate(const SoilType *soilType)
{
    // Compute the reference strain based on the PI, OCR, and mean stress
    double refStrain = (0.0352 + 0.0010 * soilType->pi()
                        * pow(soilType->ocr(), 0.3246)) * pow(soilType->meanStress(), 0.3483);

    // Curvature coefficient of the hyperbolic strain model
    const double curv =  0.9190;

    for (int i = 0; i < m_strain.size(); ++i) {
        // Normalized shear modulus
        const double shearMod = 1 / ( 1 + pow(m_strain.at(i) / refStrain, 0.9190));

        if (m_type == ModulusReduction) {
            m_average[i] = shearMod;
        } else {
            // Minimum damping based on soil properties
            const double minDamping = (0.8005 + 0.0129 * soilType->pi() * pow(soilType->ocr(), -0.1069))
                                      * pow(soilType->meanStress(), -0.2889) * (1 + 0.2919 * log(soilType->freq()));

            // Masing damping based on shear-modulus reduction
            const double masingDamping_a1 = (100./M_PI) *
                                            (4 * (m_strain.at(i) - refStrain * log((m_strain.at(i) + refStrain)/ refStrain))
                                             / (pow(m_strain.at(i),2.) / (m_strain.at(i) + refStrain)) - 2.);

            // Correction between perfect hyperbolic strain model and modified model.
            const double c1 = -1.1143 * curv * curv + 1.8618 * curv + 0.2523;
            const double c2 =  0.0805 * curv * curv - 0.0710 * curv - 0.0095;
            const double c3 = -0.0005 * curv * curv + 0.0002 * curv + 0.0003;

            //double masingD = c1 * masingD_a1 + c2 * pow( masingD_a1, 2. ) + c3 * pow( masingD_a1, 3. );
            const double masingDamping = c1 * masingDamping_a1 + c2 * pow(masingDamping_a1, 2.) + c3 * pow(masingDamping_a1, 3.);

            // Masing correction factor
            const double b = 0.6329 - 0.00566 * log(soilType->nCycles());

            // Compute the damping in percent
            m_average[i] = (minDamping + masingDamping * b * pow(shearMod, 0.1));
        }
    }
    emit dataChanged(index(0, PropertyColumn), index(rowCount(), PropertyColumn));
    setVaried(m_average);
}
