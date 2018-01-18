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

#include "AbstractCalculator.h"

#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"

#include <QDebug>

AbstractCalculator::AbstractCalculator(QObject *parent) :
        QObject(parent)
{  
    _site = 0;
    _motion = 0;
    _nsl = 0;
    _nf = 0;
    _okToContinue = false;
    _textLog = 0;
}

QString AbstractCalculator::toHtml() const
{
    return "";
}

void AbstractCalculator::reset()
{
    // Do nothing!
}

SoilProfile* AbstractCalculator::site() const
{
    return _site;
}

AbstractMotion* AbstractCalculator::motion() const
{
    return _motion;
}

void AbstractCalculator::stop()
{
    _okToContinue = false;
}

void AbstractCalculator::setTextLog( TextLog * textLog )
{
    _textLog = textLog;
}

double AbstractCalculator::surfacePGA() const
{
    // Compute the acceleration at the top of the surface
    return _motion->max(calcAccelTf(_site->inputLocation(), _motion->type(), Location(0, 0), AbstractMotion::Outcrop));
}

void AbstractCalculator::init(AbstractMotion* motion, SoilProfile* site)
{
    _motion = motion;
    _site = site;

    // Determine the number of SubLayers and freqAtuency points
    _nsl = _site->subLayerCount();
    _nf = _motion->freqCount();

    // Size the vectors
    _shearMod.resize(_nsl + 1);
    _waveA.resize(_nsl + 1);
    _waveB.resize(_nsl + 1);
    _waveNum.resize(_nsl + 1);

    for ( int i = 0; i <= _nsl; ++i ) {
        _shearMod[i].resize(_nf);
        _waveA[i].resize(_nf);
        _waveB[i].resize(_nf);
        _waveNum[i].resize(_nf);
    }

    Q_ASSERT(_nsl);
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
    for (int i = 0; i <= _nsl; ++i) {
        for (int j = 0; j < _nf; ++j) {
            _waveNum[i][j] = _motion->angFreqAt(j)
                              / sqrt(_shearMod.at(i).at(j) / _site->density(i));
        }
    }

    for (int i = 0; i < _nsl; ++i ) {
        for (int j = 0; j < _nf; ++j ) {
            // In the top surface layer, the up-going and down-going waves have
            // an amplitude of 1 as they are completely reflected at the
            // surface.
            if (i == 0) {
                _waveA[i][j] = 1.0;
                _waveB[i][j] = 1.0;
            }

            // At frequencies less than 0.000001 (zero) the amplitude of the
            // upgoing and downgoing waves is 1.
            if (_motion->freqAt(j) < 0.000001 ) {
                _waveA[i+1][j] = 1.0;
                _waveB[i+1][j] = 1.0;
            } else {
                // Complex impedence
                cImped = (_waveNum.at(i).at(j) * _shearMod.at(i).at(j)) /
                         (_waveNum.at(i+1).at(j) * _shearMod.at(i+1).at(j));

                // Complex term to simplify equations -- uses full layer height
                cTerm =  std::complex<double>( 0.0, 1.0) * _waveNum.at(i).at(j) *
                         _site->subLayers().at(i).thickness();

                _waveA[i+1][j] = 0.5 * _waveA.at(i).at(j) * (1.0 + cImped) * exp(cTerm)
                                  + 0.5 * _waveB.at(i).at(j) * (1.0 - cImped) * exp(-cTerm);

                _waveB[i+1][j] = 0.5 * _waveA.at(i).at(j) * (1.0 - cImped) * exp(cTerm)
                                  + 0.5 * _waveB.at(i).at(j) * (1.0 + cImped) * exp(-cTerm);
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

    QVector<std::complex<double> > tf(_nf);
    std::complex<double> cTerm, numer, denom;
    std::complex<double> value;

    const int l = outLocation.layer();
    const double gravity = Units::instance()->gravity();

    // Strain is inversely proportional to the complex shear-wave velocity
    for (int i = 0; i < tf.size(); ++i) {
        cTerm = std::complex<double>( 0.0,  1.0 ) *
                _waveNum.at(outLocation.layer()).at(i) * outLocation.depth();

        // Compute the numerator cannot be computed using waves since it is
        // A-B. The numerator includes gravity to correct for the Vs scaling.
        numer = std::complex<double>(gravity, -1.0 ) *
                (_waveA.at(outLocation.layer()).at(i) * exp(cTerm) -
                 _waveB.at(outLocation.layer()).at(i) * exp(-cTerm));

        denom = sqrt(_shearMod.at(l).at(i) / _site->density(l)) * waves(i, inLocation, inputType);

        value = numer / denom;
        tf[i] = (value == value) ? value : 0;
    }

    return tf;
}

QVector<std::complex<double> > AbstractCalculator::calcStressTf(
        const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const
{
    QVector<std::complex<double> > tf = calcStrainTf(inLocation, inputType, outLocation);
    const int l = outLocation.layer();

    for (int i = 0; i < tf.size(); ++i) {
        tf[i] *= _shearMod.at(l).at(i);
    }

    return tf;
}

const QVector<std::complex<double> > AbstractCalculator::calcAccelTf(
        const Location & inLocation, const AbstractMotion::Type inputType,
        const Location & outLocation, const AbstractMotion::Type outputType ) const
{
    QVector<std::complex<double> > tf(_nf);
    std::complex<double> value;

    for (int i = 0; i < _nf; i++) {
        value = waves(i, outLocation, outputType) / waves(i, inLocation, inputType);
        tf[i] = (value == value) ? value : 0;
    }

    return tf;
}

const std::complex<double> AbstractCalculator::waves(const int freqIdx,
                                                     const Location & location, const AbstractMotion::Type type ) const
{
    std::complex<double> cTerm = std::complex<double>(0., 1.) *
                                 _waveNum.at(location.layer()).at(freqIdx) * location.depth();

    if (type == AbstractMotion::Within) {
        return _waveA.at(location.layer()).at(freqIdx) * exp(cTerm) +
                _waveB.at(location.layer()).at(freqIdx) * exp(-cTerm);
    } else if (type == AbstractMotion::Outcrop) {
        return 2.0 * _waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
    } else { // type == AbstractMotion::IncomingOnly
        return _waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
    }
}
