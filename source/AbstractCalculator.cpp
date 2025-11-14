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

AbstractCalculator::AbstractCalculator(QObject *parent)
    : QObject(parent), _status(CalculationStatus::NotRun) {
  _site = nullptr;
  _motion = nullptr;
  _nsl = 0;
  _nf = 0;
  _okToContinue = false;
  _textLog = nullptr;
}

auto AbstractCalculator::toHtml() const -> QString { return ""; }

void AbstractCalculator::reset() {
  // Do nothing!
}

auto AbstractCalculator::site() const -> SoilProfile * { return _site; }

auto AbstractCalculator::motion() const -> AbstractMotion * { return _motion; }

void AbstractCalculator::stop() { _okToContinue = false; }

auto AbstractCalculator::status() const -> CalculationStatus { return _status; }

void AbstractCalculator::setTextLog(TextLog *textLog) { _textLog = textLog; }

auto AbstractCalculator::surfacePGA() const -> double {
  // Compute the acceleration at the top of the surface
  return _motion->max(calcAccelTf(_site->inputLocation(), _motion->type(),
                                  Location(0, 0), AbstractMotion::Outcrop));
}

void AbstractCalculator::init(AbstractMotion *motion, SoilProfile *site) {
  bool motionChanged = false;
  if (_motion != motion) {
    _motion = motion;
    _nf = _motion->freqCount();
    motionChanged = true;
  }

  bool siteChanged = false;
  if (_site != site) {
    _site = site;
    siteChanged = true;
  }

  if (_nsl != _site->subLayerCount()) {
    _nsl = _site->subLayerCount();
    siteChanged = true;
  }

  if (siteChanged || motionChanged) {
    // Size the vectors
    _shearMod.resize(_nsl + 1);
    _waveA.resize(_nsl + 1);
    _waveB.resize(_nsl + 1);
    _waveNum.resize(_nsl + 1);

    for (int i = 0; i <= _nsl; ++i) {
      _shearMod[i].resize(_nf);
      _waveA[i].resize(_nf);
      _waveB[i].resize(_nf);
      _waveNum[i].resize(_nf);
    }
  }
}

auto AbstractCalculator::calcCompShearMod(const double shearMod,
                                          const double damping)
    -> std::complex<double> {
  // This is the complex shear-modulus used in SHAKE91.  The DeepSoil manual
  // states that this shear modulus results in frequency dependent dependent
  // damping, and therefore should be used with caution. (Udaka, 1975)
  // return shearMod * std::complex<double>( 1.0 - 2 * damping * damping, 2 *
  // damping * sqrt( 1.0 - damping * damping ));

  // Simplified complex shear modulus (Kramer, 1996)
  return shearMod * std::complex<double>(1.0 - damping * damping, 2 * damping);
}

auto AbstractCalculator::calcWaves() -> bool {
  std::complex<double> cImped;
  std::complex<double> cTerm;

  // Compute the complex wave numbers of the system
  for (int i = 0; i <= _nsl; ++i) {
    const double density_i = _site->density(i);
    for (int j = 0; j < _nf; ++j) {
      _waveNum[i][j] =
          _motion->angFreqAt(j) / sqrt(_shearMod[i][j] / density_i);
    }
  }

  for (int i = 0; i < _nsl; ++i) {
    const double thickness = _site->subLayers().at(i).thickness();
    for (int j = 0; j < _nf; ++j) {
      // In the top surface layer, the up-going and down-going waves have
      // an amplitude of 1 as they are completely reflected at the
      // surface.
      if (i == 0) {
        _waveA[i][j] = 1.0;
        _waveB[i][j] = 1.0;
      }

      // At frequencies less than 0.000001 (zero) the amplitude of the
      // upgoing and downgoing waves is 1.
      if (_motion->freqAt(j) < 0.000001) {
        _waveA[i + 1][j] = 1.0;
        _waveB[i + 1][j] = 1.0;
      } else {
        // Cache complex values to avoid repeated lookups
        const std::complex<double> &waveNum_i_j = _waveNum[i][j];
        const std::complex<double> &waveNum_ip1_j = _waveNum[i + 1][j];
        const std::complex<double> &shearMod_i_j = _shearMod[i][j];
        const std::complex<double> &shearMod_ip1_j = _shearMod[i + 1][j];
        const std::complex<double> &waveA_i_j = _waveA[i][j];
        const std::complex<double> &waveB_i_j = _waveB[i][j];

        // Complex impedence
        cImped =
            (waveNum_i_j * shearMod_i_j) / (waveNum_ip1_j * shearMod_ip1_j);

        // Complex term to simplify equations -- uses full layer height
        cTerm = std::complex<double>(0.0, 1.0) * waveNum_i_j * thickness;

        const std::complex<double> exp_cTerm = exp(cTerm);
        const std::complex<double> exp_neg_cTerm = exp(-cTerm);
        const std::complex<double> one_plus_cImped = 1.0 + cImped;
        const std::complex<double> one_minus_cImped = 1.0 - cImped;

        _waveA[i + 1][j] = 0.5 * waveA_i_j * one_plus_cImped * exp_cTerm +
                           0.5 * waveB_i_j * one_minus_cImped * exp_neg_cTerm;

        _waveB[i + 1][j] = 0.5 * waveA_i_j * one_minus_cImped * exp_cTerm +
                           0.5 * waveB_i_j * one_plus_cImped * exp_neg_cTerm;
      }
    }
  }
  return true;
}

auto AbstractCalculator::calcStrainTf(const Location &inLocation,
                                      const AbstractMotion::Type inputType,
                                      const Location &outLocation) const
    -> QVector<std::complex<double>> {

  /* The strain transfer function from the acceleration at layer n (outcrop)
  to the mid-height of layer m (within) is defined as:

  Strain(angFreq, z=h_m/2)   i k*_m [ A_m exp(i k*_m h_m / 2) - B_m exp(-i k*_m
  h_m / 2)]
  ------------------------ =
  ------------------------------------------------------------ accel_n(angFreq)
  -angFreq^2 (2 * A_n)

  The problem with this formula is that at low frequencies the division is
  prone to errors -- in particular when angFreq = 0.

  To solve this problem, strain is computed from the velocity FAS.  The
  associated transfer function to compute the strain is then defined as:

  Strain(angFreq, z=h_m/2)   [ A_m exp(i k*_m h_m / 2) - B_m exp(-i k*_m h_m /
  2)]
  ------------------------ =
  ----------------------------------------------------- vel_n(angFreq) v*_s (2 *
  A_n)

  */

  QVector<std::complex<double>> tf(_nf);
  std::complex<double> cTerm, numer, denom;
  std::complex<double> value;

  const int l = outLocation.layer();
  const double gravity = Units::instance()->gravity();
  const double outDepth = outLocation.depth();
  const double density_l = _site->density(l);
  const std::complex<double> gravity_cmplx(gravity, 0.0);
  const std::complex<double> imag_unit(0.0, 1.0);

  // Strain is inversely proportional to the complex shear-wave velocity
  for (int i = 0; i < tf.size(); ++i) {
    cTerm = imag_unit * _waveNum[l][i] * outDepth;

    const std::complex<double> exp_cTerm = exp(cTerm);
    const std::complex<double> exp_neg_cTerm = exp(-cTerm);

    // Compute the numerator cannot be computed using waves since it is
    // A-B. The numerator includes gravity to correct for the Vs scaling.
    numer = gravity_cmplx *
            (_waveA[l][i] * exp_cTerm - _waveB[l][i] * exp_neg_cTerm);

    denom = sqrt(_shearMod[l][i] / density_l) * waves(i, inLocation, inputType);

    value = numer / denom;
    tf[i] = (value == value) ? value : 0; // Check for NaN
  }

  return tf;
}

auto AbstractCalculator::calcStressTf(const Location &inLocation,
                                      const AbstractMotion::Type inputType,
                                      const Location &outLocation) const
    -> QVector<std::complex<double>> {
  QVector<std::complex<double>> tf =
      calcStrainTf(inLocation, inputType, outLocation);
  const int l = outLocation.layer();

  for (int i = 0; i < tf.size(); ++i) {
    tf[i] *= _shearMod.at(l).at(i);
  }

  return tf;
}

auto AbstractCalculator::calcAccelTf(
    const Location &inLocation, const AbstractMotion::Type inputType,
    const Location &outLocation, const AbstractMotion::Type outputType) const
    -> const QVector<std::complex<double>> {
  QVector<std::complex<double>> tf(_nf);
  std::complex<double> value;

  for (int i = 0; i < _nf; i++) {
    value = waves(i, outLocation, outputType) / waves(i, inLocation, inputType);
    tf[i] = (value == value) ? value : 0;
  }

  return tf;
}

auto AbstractCalculator::waves(const int freqIdx, const Location &location,
                               const AbstractMotion::Type type) const
    -> const std::complex<double> {
  std::complex<double> cTerm = std::complex<double>(0., 1.) *
                               _waveNum.at(location.layer()).at(freqIdx) *
                               location.depth();

  if (type == AbstractMotion::Within) {
    return _waveA.at(location.layer()).at(freqIdx) * exp(cTerm) +
           _waveB.at(location.layer()).at(freqIdx) * exp(-cTerm);
  } else if (type == AbstractMotion::Outcrop) {
    return 2.0 * _waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
  } else { // type == AbstractMotion::IncomingOnly
    return _waveA.at(location.layer()).at(freqIdx) * exp(cTerm);
  }
}
