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

#include "AbstractCalculator.h"

#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"

#include <QDebug>

AbstractCalculator::AbstractCalculator(QObject *parent) :
        QObject(parent)
{  
    m_site = 0;
    m_motion = 0;
    m_nsl = 0;
    m_nf = 0;
    m_okToContinue = false;
    m_textLog = 0;
}

QString AbstractCalculator::toHtml() const
{
    return "";
}

void AbstractCalculator::reset()
{
    // Do nothing!
}

SoilProfile* const AbstractCalculator::site() const
{
    return m_site;
}

AbstractMotion* const AbstractCalculator::motion() const
{
    return m_motion;
}

void AbstractCalculator::stop()
{
    m_okToContinue = false;
}

void AbstractCalculator::setTextLog( TextLog * textLog )
{
    m_textLog = textLog;
}

double AbstractCalculator::surfacePGA() const
{
    // Compute the acceleration at the top of the surface
    return m_motion->max(calcAccelTf(m_site->inputLocation(), m_motion->type(), Location(0, 0), AbstractMotion::Outcrop));
}

void AbstractCalculator::init(AbstractMotion* motion, SoilProfile* site)
{
    m_motion = motion;
    m_site = site;

    // Determine the number of SubLayers and freqAtuency points
    m_nsl = m_site->subLayerCount();
    m_nf = m_motion->freqCount();

    // Size the vectors
    m_shearMod.resize(m_nsl + 1);
    m_waveA.resize(m_nsl + 1);
    m_waveB.resize(m_nsl + 1);
    m_waveNum.resize(m_nsl + 1);

    for ( int i = 0; i <= m_nsl; ++i ) {
        m_shearMod[i].resize(m_nf);
        m_waveA[i].resize(m_nf);
        m_waveB[i].resize(m_nf);
        m_waveNum[i].resize(m_nf);
    }

    Q_ASSERT(m_nsl);
}

std::complex<double> AbstractCalculator::calcCompShearMod(const double shearMod, const double damping) const
{
    // This is the complex shear-modulus used in SHAKE91.  The DeepSoil manual
    // states that this shear modulus results in frequency dependent dependent
    // damping, and therefore should be used with caution. (Udaka, 1975)
    // return shearMod * std::complex<double>( 1.0 - 2 * damping * damping, 2 * damping * sqrt( 1.0 - damping * damping ));

    // Simplified complex shear modulus (Kramer, 1996)
    return shearMod * std::complex<double>( 1.0 - damping * damping, 2 * damping);
}

bool AbstractCalculator::calcWaves()
{
    std::complex<double> cImped;
    std::complex<double> cTerm;

    // Compute the complex wave numbers of the system
    for (int i = 0; i <= m_nsl; ++i) {
        for (int j = 0; j < m_nf; ++j) {
            m_waveNum[i][j] = m_motion->angFreqAt(j)
                              / sqrt(m_shearMod.at(i).at(j) / m_site->density(i));
        }
    }

    for (int i = 0; i < m_nsl; ++i ) {
        for (int j = 0; j < m_nf; ++j ) {
            // In the top surface layer, the up-going and down-going waves have
            // an amplitude of 1 as they are completely reflected at the
            // surface.
            if (i == 0) {
                m_waveA[i][j] = 1.0;
                m_waveB[i][j] = 1.0;
            }

            // At frequencies less than 0.000001 (zero) the amplitude of the
            // upgoing and downgoing waves is 1.
            if (m_motion->freqAt(j) < 0.000001 ) {
                m_waveA[i+1][j] = 1.0;
                m_waveB[i+1][j] = 1.0;
            } else {
                // Complex impedence
                cImped = (m_waveNum.at(i).at(j) * m_shearMod.at(i).at(j)) /
                         (m_waveNum.at(i+1).at(j) * m_shearMod.at(i+1).at(j));

                // Complex term to simplify equations -- uses full layer height
                cTerm =  std::complex<double>( 0.0, 1.0) * m_waveNum.at(i).at(j) *
                         m_site->subLayers().at(i).thickness();

                m_waveA[i+1][j] = 0.5 * m_waveA.at(i).at(j) * (1.0 + cImped) * exp(cTerm)
                                  + 0.5 * m_waveB.at(i).at(j) * (1.0 - cImped) * exp(-cTerm);

                m_waveB[i+1][j] = 0.5 * m_waveA.at(i).at(j) * (1.0 - cImped) * exp(cTerm)
                                  + 0.5 * m_waveB.at(i).at(j) * (1.0 + cImped) * exp(-cTerm);

                // Check for NaNs
                if (m_waveA[i+1][j] != m_waveA[i+1][j] ||
                    m_waveB[i+1][j] != m_waveB[i+1][j])
                    return false;
            }
        }
    }
    return true;
}

QVector<std::complex<double> > AbstractCalculator::calcStrainTf(
        const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const
{

    /* The strain transfer function from the acceleration at layer n (outcrop)
    to the mid-height of layer m (within) is defined as:

    Strain(angFreq, z=h_m/2)   i k*_m [ A_m exp(i k*_m h_m / 2) - B_m exp(-i k*_m h_m / 2)]
    ------------------------ = ------------------------------------------------------------
       accel_n(angFreq)                       -angFreq^2 (2 * A_n)

    The problem with this formula is that at low frequencies the division is
    prone to errors -- in particular when angFreq = 0.

    To solve this problem, strain is computed from the velocity FAS.  The associated
    transfer function to compute the strain is then defined as:

    Strain(angFreq, z=h_m/2)   -i [ A_m exp(i k*_m h_m / 2) - B_m exp(-i k*_m h_m / 2)]
    ------------------------ = ------------------------------------------------------------
         vel_n(angFreq)                       v*_s (2 * A_n)

    */

    QVector<std::complex<double> > tf(m_nf);
    std::complex<double> cTerm, numer, denom;

    const int l = outLocation.layer();
    const double gravity = Units::instance()->gravity();

    // Strain is inversely proportional to the complex shear-wave velocity
    for (int i = 0; i < tf.size(); ++i) {
        cTerm = std::complex<double>( 0.0,  1.0 ) *
                m_waveNum.at(outLocation.layer()).at(i) * outLocation.depth();

        // Compute the numerator cannot be computed using waves since it is
        // A-B. The numerator includes gravity to correct for the Vs scaling.
        numer = std::complex<double>(gravity, -1.0 ) *
                (m_waveA.at(outLocation.layer()).at(i) * exp(cTerm) -
                 m_waveB.at(outLocation.layer()).at(i) * exp(-cTerm));

        denom = sqrt(m_shearMod.at(l).at(i) / m_site->density(l)) * waves(i, inLocation, inputType);

        tf[i] = numer / denom;
    }

    return tf;
}

QVector<std::complex<double> > AbstractCalculator::calcStressTf(
        const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const
{
    QVector<std::complex<double> > tf = calcStrainTf(inLocation, inputType, outLocation);

    const int l = outLocation.layer();

    for (int i = 0; i < tf.size(); ++i)
        tf[i] *= m_shearMod.at(l).at(i);

    return tf;
}

const QVector<std::complex<double> > AbstractCalculator::calcAccelTf(
        const Location & inLocation, const AbstractMotion::Type inputType,
        const Location & outLocation, const AbstractMotion::Type outputType ) const
{
    QVector<std::complex<double> > tf(m_nf);

    for (int i = 0; i < m_nf; i++)
        tf[i] = waves(i, outLocation, outputType) / waves(i, inLocation, inputType);

    return tf;
}

const std::complex<double> AbstractCalculator::waves(const int freqIdx,
                                                     const Location & location, const AbstractMotion::Type type ) const
{
    std::complex<double> cTerm = std::complex<double>(0., 1.) *
                                 m_waveNum.at(location.layer()).at(freqIdx) * location.depth();

    if (type == AbstractMotion::Within) {
        return m_waveA.at(location.layer()).at(freqIdx) * exp(cTerm) +
                m_waveB.at(location.layer()).at(freqIdx) * exp(-cTerm);
    } else if (type == AbstractMotion::Outcrop) {
        return 2.0 * m_waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
    } else { // type == AbstractMotion::IncomingOnly
        return m_waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
    }
}
