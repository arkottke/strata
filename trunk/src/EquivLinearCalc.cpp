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
// Copyright 2007 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "EquivLinearCalc.h"
#include "RecordedMotion.h"
#include "RvtMotion.h"
#include "Units.h"

#include <cmath>

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QObject>
#include <QDebug>

EquivLinearCalc::EquivLinearCalc( bool * okToContinue, QObject * parent)
    : QObject(parent), m_okToContinue(okToContinue)
{
    reset();
}

void EquivLinearCalc::reset()
{
    // Default values
    m_maxIterations = 8;
    m_errorTolerance = 1.00;
    m_strainRatio = 0.65;

    // Clear vectors
    m_shearMod.clear();
    m_waveA.clear();
    m_waveB.clear();
    m_waveNum.clear();
}

void EquivLinearCalc::stop()
{
    m_okToContinue = false;
}

void EquivLinearCalc::setMaxIterations( int maxIterations)
{
    if ( m_maxIterations != maxIterations ) {
        emit wasModified();
    }

    m_maxIterations = maxIterations;
}

int EquivLinearCalc::maxIterations() const
{
    return m_maxIterations;
}

void EquivLinearCalc::setStrainRatio( double strainRatio )
{
    if ( m_strainRatio != strainRatio ) {
        emit wasModified();
    }

    m_strainRatio = strainRatio;
}
double EquivLinearCalc::strainRatio() const
{
    return m_strainRatio;
}

void EquivLinearCalc::setErrorTolerance( double errorTolerance )
{
    if ( m_errorTolerance != errorTolerance ) {
        emit wasModified();
    }

    m_errorTolerance = errorTolerance;
}

double EquivLinearCalc::errorTolerance() const
{
    return m_errorTolerance;
}

bool EquivLinearCalc::converged() const
{
    return m_converged;
}

void EquivLinearCalc::setTextLog( TextLog * textLog )
{
    m_textLog = textLog;
}

QMap<QString, QVariant> EquivLinearCalc::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("maxIterations", QVariant(m_maxIterations));
    map.insert("errorTolerance", QVariant(m_errorTolerance));
    map.insert("strainRatio", QVariant(m_strainRatio));

    return map;
}

void EquivLinearCalc::fromMap(const QMap<QString, QVariant> & map)
{
    m_maxIterations = map.value("maxIterations").toInt();
    m_errorTolerance = map.value("errorTolerance").toDouble();
    m_strainRatio = map.value("strainRatio").toDouble();
}

bool EquivLinearCalc::run( Motion * motion, SiteProfile * site)
{
    m_okToContinue = true;
    // Save the references
    m_motion = motion;
    m_site = site;

    // Determine the number of SubLayers and freqAtuency points
    m_nsl = m_site->subLayerCount();
    m_nf = m_motion->freqCount();

    // Size the vectors
    m_shearMod.resize( m_nsl + 1 );
    m_waveA.resize( m_nsl + 1 );
    m_waveB.resize( m_nsl + 1 );
    m_waveNum.resize( m_nsl + 1 );
    QVector<std::complex<double> > strainTf;

    for ( int i = 0; i <= m_nsl; ++i ) {
        m_waveA[i].resize( m_nf );
        m_waveB[i].resize( m_nf );
        m_waveNum[i].resize( m_nf );
    }

    /*
         * While the error in the properties is greater than the tolerable limit
         * and the number of iterations is under the maximum compute the strain
         * compatible properties.
         */

    // Compute the bedrock properties -- these do not change during the process.
    m_shearMod[m_nsl] = calcCompShearMod( m_site->shearMod(m_nsl), m_site->damping(m_nsl) / 100. );
    // Initialize the loop control variables
    int iter = 0;
    double maxError;

    do {
        if (!m_okToContinue) {
            m_converged = false;
            return false;
        }

        // Compute the complex shear modulus and complex shear-wave velocity
        // for each soil layer -- these change because the damping and shear
        // modulus change.
        for ( int i = 0; i < m_nsl; ++i ) {
            m_shearMod[i] = calcCompShearMod( m_site->shearMod(i), m_site->damping(i) / 100. );
        }

        // Compute the upgoing and downgoing waves
        if (!calcWaves()) {
            return false;
        }

        maxError = -1;
        // Compute the strain in each of the layers
        for (int i = 0; i < m_nsl; ++i)
        {
            if (!calcStrainTf(m_site->inputLocation(), m_motion->type(),
                               Location(i, m_site->subLayers().at(i).thickness() / 2), strainTf)) {
                return false;
            }
            // Compute the maximum strain within the layer -- report the strain in %
            // The motion is in gravity and needs to be converted to useful units
            double maxStrain = 100 * Units::instance()->gravity() * m_motion->max(strainTf);
            // Interpolate the soil properites
            m_site->subLayers()[i].setStrain(m_strainRatio * maxStrain, maxStrain);
            // Save the error in the layer if it is larger than the previously saved max
            if (maxError < m_site->subLayers().at(i).error()) {
                maxError = m_site->subLayers().at(i).error();
            }

            if (!m_okToContinue) {
                m_converged = false;
                return false;
            }
        }

        // Print information regarding the iteration
        if ( m_textLog->level() > TextLog::Low ) {
            m_textLog->append(QString(QObject::tr("\t\t\tIteration: %1 Maximum Error: %2 %"))
                    .arg(iter+1)
                    .arg(maxError, 0, 'f', 2));
        }

        if ( m_textLog->level() > TextLog::Medium ) {
            m_textLog->append("\t\t" + m_site->subLayerTable());
        }

        // Step the iteration
        ++iter;

    } while((maxError > m_errorTolerance)  && (iter < m_maxIterations));

    if ((iter == m_maxIterations ) && ( maxError > m_errorTolerance ) ) {
        m_textLog->append(QString(QObject::tr("\t\t\t!! -- Maximum number of iterations reached (%1). Maximum Error: %2 %"))
                          .arg(iter)
                          .arg(maxError, 0, 'f', 2));

        m_converged = false;
    } else {
        m_converged = true;
    }

    return true;
}

SiteProfile * EquivLinearCalc::site() const
{
    return m_site;
}

Motion * EquivLinearCalc::motion() const
{
    return m_motion;
}

double EquivLinearCalc::surfacePGA() const
{
    // Compute the acceleration at the top of the surface
    return m_motion->max(calcAccelTf( m_site->inputLocation(), m_motion->type(), Location(0, 0), Motion::Outcrop ));
}

const QVector<double> EquivLinearCalc::maxAccelProfile() const
{
    QVector<double> profile(m_nsl + 1);

    for (int i = 0; i <= m_nsl; ++i) {
        Motion::Type type = (i == 0) ? Motion::Outcrop : Motion::Within;
        // Compute the acceleration at the top of each layer
        profile[i] = m_motion->max(calcAccelTf( m_site->inputLocation(), m_motion->type(), Location(i, 0), type ));
    }

    return profile;
}

const QVector<double> EquivLinearCalc::maxVelProfile() const
{
    QVector<double> profile(m_nsl + 1);

    for (int i = 0; i <= m_nsl; ++i) {
        Motion::Type type = (i == 0) ? Motion::Outcrop : Motion::Within;

        // Compute the acceleration at the top of each layer
        QVector<std::complex<double> > tf = calcAccelTf( m_site->inputLocation(), m_motion->type(), Location(i, 0), type );

        double maxVel = 0;

        if ( dynamic_cast<RvtMotion*>(m_motion) ) {
            // Transform the transfer function from acceleration to velocity
            for (int j = 0; j < tf.size(); ++j ) {
                tf[j] /=  std::complex<double>( 0, m_motion->angFreqAt(j));
            }
            maxVel = m_motion->max(tf);
        } else {
            RecordedMotion * motion = dynamic_cast<RecordedMotion*>(m_motion);

            QVector<double> accelTs = motion->timeSeries(tf);
            QVector<double> velTs(accelTs.size());

            velTs[0] = 0;
            // Integration of the acceleration time series
            for ( int j = 1; j < accelTs.size(); ++j ) {
                velTs[j] = velTs.at(j-1) + motion->timeStep() * ( accelTs.at(j) + accelTs.at(j-1) ) / 2.;

                // Store the maximum value
                if ( maxVel < fabs(velTs.at(j)) ) {
                    maxVel = fabs(velTs.at(j));
                }
            }
        }
        // Convert from g-s to the proper units
        profile[i] = Units::instance()->tsConv() * maxVel;
    }

    return profile;
}

std::complex<double> EquivLinearCalc::calcCompShearMod( const double shearMod, const double damping ) const
{
    // This is the complex shear-modulus used in SHAKE91.  The DeepSoil manual
    // states that this shear modulus results in frequency dependent dependent
    // damping, and therefore should be used with caution. (Udaka, 1975)
    // return shearMod * std::complex<double>( 1.0 - 2 * damping * damping, 2 * damping * sqrt( 1.0 - damping * damping ));
    
    // Simplified complex shear modulus (Kramer, 1996)
    return shearMod * std::complex<double>( 1.0 - damping * damping, 2 * damping);
}

bool EquivLinearCalc::calcWaves()
{
    std::complex<double> cImped;
    std::complex<double> cTerm;

    // Compute the complex wave numbers of the system
    for ( int i = 0; i <= m_nsl; ++i ) {
        for ( int j = 0; j < m_nf; ++j ) {
            m_waveNum[i][j] = m_motion->angFreqAt(j) / sqrt( m_shearMod.at(i) / m_site->density(i) );
        }
    }

    for ( int i = 0; i < m_nsl; ++i ) {
        for ( int j = 0; j < m_nf; ++j ) {
            // In the top surface layer, the up-going and down-going waves have
            // an amplitude of 1 as they are completely reflected at the
            // surface.
            if ( i == 0 ) {
                m_waveA[i][j] = 1.0;
                m_waveB[i][j] = 1.0;
            }

            // At frequencies less than 0.0001 (zero) the amplitude of the
            // upgoing and downgoing waves is 1.
            if ( m_motion->freqAt(j) < 0.0001 ) {
                m_waveA[i+1][j] = 1.0;
                m_waveB[i+1][j] = 1.0;
            } else {
                // Complex impedence
                cImped = ( m_waveNum.at(i).at(j) * m_shearMod.at(i) ) /
                         ( m_waveNum.at(i+1).at(j) * m_shearMod.at(i+1) );

                // Complex term to simplify equations -- uses full layer height
                cTerm =  std::complex<double>( 0.0, 1.0) * m_waveNum.at(i).at(j) *
                         m_site->subLayers().at(i).thickness();

                m_waveA[i+1][j] = 0.5 * m_waveA.at(i).at(j) * ( 1.0 + cImped ) * exp( cTerm )
                                  + 0.5 * m_waveB.at(i).at(j) * ( 1.0 - cImped ) * exp( -cTerm );

                m_waveB[i+1][j] = 0.5 * m_waveA.at(i).at(j) * ( 1.0 - cImped ) * exp( cTerm )
                                  + 0.5 * m_waveB.at(i).at(j) * ( 1.0 + cImped ) * exp( -cTerm );

                if (m_waveA[i+1][j] != m_waveA[i+1][j] ||
                        m_waveB[i+1][j] != m_waveB[i+1][j]) {
                    // Check for NaNs
                    return false;
                }
            }
        }
    }
    return true;
}

bool EquivLinearCalc::calcStrainTf( const Location & inLocation, const Motion::Type inputType,
        const Location & outLocation, QVector<std::complex<double> > & tf ) const
{
    std::complex<double> cTerm;
    std::complex<double> numer;
    std::complex<double> denom;
    // Resize the transfer function to the number of frequencies
    tf.resize(m_nf);

    for ( int i = 0; i < m_nf; i++ )
    {
        cTerm = std::complex<double>( 0.0,  1.0 ) *
                m_waveNum.at(outLocation.layer()).at(i) * outLocation.depth();

        // Compute the numerator cannot be computed using waves since it is A-B
        numer = std::complex<double>( 0.0, 1.0 ) * m_waveNum.at(outLocation.layer()).at(i) *
                ( m_waveA.at(outLocation.layer()).at(i) * exp( cTerm ) -
                  m_waveB.at(outLocation.layer()).at(i) * exp( -cTerm ) );

        // Compute the denominator is modified by the square of the angular
        // frequency and a negative
        denom = -m_motion->angFreqAt(i) * m_motion->angFreqAt(i) *
                waves(i, inLocation, inputType);

        if ( m_motion->freqAt(i) < 0.0001 ) {
            tf[i] = 0.0;
        } else {
            tf[i] = numer / denom;
        }

        if ( tf[i] != tf[i] ) {
            // Check for NaNs
            return false;
        }
    }
    return true;
}

const QVector<std::complex<double> > EquivLinearCalc::calcAccelTf(
        const Location & inLocation, const Motion::Type inputType,
        const Location & outLocation, const Motion::Type outputType ) const
{
    QVector<std::complex<double> > tf(m_nf);

    for ( int i = 0; i < m_nf; i++ ) {
        tf[i] = waves( i, outLocation, outputType ) / waves( i, inLocation, inputType );
    }

    return tf;
}

const std::complex<double> EquivLinearCalc::waves( const int freqIdx,
                                                   const Location & location, const Motion::Type type ) const
{
    std::complex<double> cTerm = std::complex<double>(0., 1.) *
                                 m_waveNum.at(location.layer()).at(freqIdx) * location.depth();

    if ( type == Motion::Within ) {
        return m_waveA.at(location.layer()).at(freqIdx) * exp(cTerm) +
                m_waveB.at(location.layer()).at(freqIdx) * exp(-cTerm);
    } else if ( type == Motion::Outcrop ) {
        return 2.0 * m_waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
    }

    return -1;
}
