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

#include "SourceTheoryRvtMotion.h"

#include "CrustalAmplification.h"
#include "Dimension.h"
#include "PathDurationModel.h"

#include <QDebug>

SourceTheoryRvtMotion::SourceTheoryRvtMotion(QObject *parent)
    : AbstractRvtMotion(parent), _isCustomized(false), _seismicMoment(0),
      _depth(0), _hypoDistance(0), _cornerFreq(0), _stressDrop(0), _geoAtten(0),
      _pathAttenCoeff(0), _pathAttenPower(0), _shearVelocity(0), _density(0),
      _siteAtten(0) {
  _freq = new Dimension(this);
  _freq->setMin(0.05);
  _freq->setMax(50);
  _freq->setSize(1024);
  _freq->setSpacing(Dimension::Log);

  _fourierAcc = QVector<double>(freqCount(), 0.);

  _crustalAmp = new CrustalAmplification;
  connect(_crustalAmp, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
  connect(this, SIGNAL(regionChanged(int)), _crustalAmp, SLOT(setRegion(int)));

  _pathDuration = new PathDurationModel;
  connect(_pathDuration, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
  connect(this, SIGNAL(regionChanged(int)), _pathDuration,
          SLOT(setRegion(int)));

  // Reset the parameters
  setRegion(_region);
  setMagnitude(_magnitude);
  setDistance(_distance);
  setDepth(8.);
  setIsCustomized(false);

  _name = tr("Source Theory RVT Motion (M $mag @ $dist km)");

  calculate();
}

SourceTheoryRvtMotion::~SourceTheoryRvtMotion() {
  _crustalAmp->deleteLater();
  _pathDuration->deleteLater();
}

auto SourceTheoryRvtMotion::freq() const -> const QVector<double> & {
  return _freq->data();
}

auto SourceTheoryRvtMotion::freqDimension() -> Dimension * { return _freq; }

auto SourceTheoryRvtMotion::toHtml() const -> QString {
  // FIXME
  return QString();
}

void SourceTheoryRvtMotion::setRegion(AbstractRvtMotion::Region region) {
  AbstractRvtMotion::setRegion(region);

  _crustalAmp->setRegion(_region);
  _pathDuration->setRegion(_region);

  switch (_region) {
  case AbstractRvtMotion::WUS:
    setStressDrop(100);
    setPathAttenCoeff(180);
    setPathAttenPower(0.45);
    setShearVelocity(3.5);
    setDensity(2.8);
    setSiteAtten(0.04);
    break;
  case AbstractRvtMotion::CEUS:
    setStressDrop(150);
    setPathAttenCoeff(680);
    setPathAttenPower(0.36);
    setShearVelocity(3.6);
    setDensity(2.8);
    setSiteAtten(0.006);
    break;
  default:
    break;
  }

  // Geometric attenuation may have changed
  calcGeoAtten();
}

// QString SourceTheoryRvtMotion::toHtml() const
//{
//     QString html = QString(tr(
//                 "<tr><th>Location:</th><td>%1</td></tr>"
//                 "<tr><th>Moment Magnitude:</th><td>%2</td></tr>"
//                 "<tr><th>Distance:</th><td>%3 km</td></tr>"
//                 "<tr><th>Depth:</th><td>%4 km </td></tr>"
//                 "<tr><th>Stress Drop:</th><td>%5 bars</td></tr>"
//                 "<tr><th>Geometric Attenuation:</th><td>%6</td></tr>"
//                 "<tr><th>Path Atten Coeff.:</th><td>%7</td></tr>"
//                 "<tr><th>Path Atten Power.:</th><td>%8</td></tr>"
//                 "<tr><th>Shear-wave velocity:</th><td>%9 km/s</td></tr>"
//                 "<tr><th>Density:</th><td>%10 gm/cm%11</td></tr>"
//                 "<tr><th>Site Attenuation (kappa):</th><td>%12</td></tr>"
//                 "<tr><th>Generic Crustal Amplication:</th><td>%13</td></tr>"
//                 "</table>"
//                ))
//         .arg(locationList().at(_location))
//         .arg(_momentMag)
//         .arg(_distance)
//         .arg(_depth)
//         .arg(_stressDrop)
//         .arg(_geoAtten)
//         .arg(_pathAttenCoeff)
//         .arg(_pathAttenPower)
//         .arg(_shearVelocity)
//         .arg(_density).arg(QChar(0x00B3))
//         .arg(_CrustalAtten)
//         .arg(_siteSpecificCrustalAmp ? tr("yes") : tr("no"));
//
//     html += "<table><tr>";
//
//     if (_siteSpecificCrustalAmp) {
//         // Add the velocity profile
//         html += "<td><h4>Velocity Profile</h4><table border = \"1\">";
//         html += QString("<tr><th>Thickness (km)</th><th>Shear Velocity
//         (km/s)</th><th>Density (gm/cm%1)</th></tr>").arg(QChar(0x00B3));
//
//         for (int i = 0; i < _crustThickness.size(); ++i) {
//             html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
//                 .arg(_crustThickness.at(i))
//                 .arg(_crustVelocity.at(i))
//                 .arg(_crustDensity.at(i));
//         }
//
//         html += "</table></td>";
//
//         // Add the crustal amplification
//         // FIXME
////        if (_crustAmpNeedsUpdate) {
////            calcCrustalAmp();
////        }
//    }
//
//    html += "<td><h4>Crustal Amplification</h4><table border = \"1\">";
//    html += "<tr><th>Frequency (Hz)</th><th>Amplification</th></tr>";
//
//    for (int i = 0; i < _freq.size(); ++i) {
//        html += QString("<tr><td>%1</td><td>%2</td></tr>")
//            .arg(freqAt(i))
//            .arg(_crustAmp.at(i));
//    }
//    html += "</table></td></tr></table>";
//
//    return html;
//}

auto SourceTheoryRvtMotion::isCustomized() const -> bool {
  return _isCustomized;
}

void SourceTheoryRvtMotion::setIsCustomized(bool b) {
  if (_isCustomized != b) {
    _isCustomized = b;
    emit isCustomizedChanged(b);
    emit wasModified();
  }
}

void SourceTheoryRvtMotion::setMagnitude(double magnitude) {
  if (abs(magnitude - _magnitude) > DBL_EPSILON) {
    AbstractRvtMotion::setMagnitude(magnitude);
    init();
  }
}

void SourceTheoryRvtMotion::setDistance(double distance) {
  if (abs(distance - _distance) > DBL_EPSILON) {
    AbstractRvtMotion::setDistance(distance);
    init();
  }
}

auto SourceTheoryRvtMotion::depth() const -> double { return _depth; }

void SourceTheoryRvtMotion::setDepth(double depth) {
  if (abs(depth - _depth) > DBL_EPSILON) {
    _depth = depth;
    init();
  }
}

auto SourceTheoryRvtMotion::stressDrop() const -> double { return _stressDrop; }

void SourceTheoryRvtMotion::setStressDrop(double stressDrop) {
  if (abs(_stressDrop - stressDrop) > DBL_EPSILON) {
    _stressDrop = stressDrop;
    emit stressDropChanged(stressDrop);
    init();
  }
}

auto SourceTheoryRvtMotion::geoAtten() const -> double { return _geoAtten; }

void SourceTheoryRvtMotion::setGeoAtten(double geoAtten) {
  if (abs(_geoAtten - geoAtten) > DBL_EPSILON) {
    _geoAtten = geoAtten;
    emit geoAttenChanged(geoAtten);
  }
}

auto SourceTheoryRvtMotion::pathAttenCoeff() const -> double {
  return _pathAttenCoeff;
}

void SourceTheoryRvtMotion::setPathAttenCoeff(double pathAttenCoeff) {
  if (abs(_pathAttenCoeff - pathAttenCoeff) > DBL_EPSILON) {
    _pathAttenCoeff = pathAttenCoeff;
    emit pathAttenCoeffChanged(pathAttenCoeff);
  }
}

auto SourceTheoryRvtMotion::pathAttenPower() const -> double {
  return _pathAttenPower;
}

void SourceTheoryRvtMotion::setPathAttenPower(double pathAttenPower) {
  if (abs(_pathAttenPower - pathAttenPower) > DBL_EPSILON) {
    _pathAttenPower = pathAttenPower;
    emit pathAttenPowerChanged(pathAttenPower);
  }
}

auto SourceTheoryRvtMotion::shearVelocity() const -> double {
  return _shearVelocity;
}

void SourceTheoryRvtMotion::setShearVelocity(double shearVelocity) {
  if (abs(_shearVelocity - shearVelocity) > DBL_EPSILON) {
    _shearVelocity = shearVelocity;
    emit shearVelocityChanged(shearVelocity);
    init();
  }
}

auto SourceTheoryRvtMotion::density() const -> double { return _density; }

void SourceTheoryRvtMotion::setDensity(double density) {
  if (abs(_density - density) > DBL_EPSILON) {
    _density = density;
    emit densityChanged(density);
  }
}

auto SourceTheoryRvtMotion::siteAtten() const -> double { return _siteAtten; }

auto SourceTheoryRvtMotion::duration() const -> double { return _duration; }

void SourceTheoryRvtMotion::setSiteAtten(double siteAtten) {
  if (abs(_siteAtten - siteAtten) > DBL_EPSILON) {
    _siteAtten = siteAtten;
    emit siteAttenChanged(siteAtten);
  }
}

auto SourceTheoryRvtMotion::crustalAmp() -> CrustalAmplification * {
  return _crustalAmp;
}

auto SourceTheoryRvtMotion::pathDuration() -> PathDurationModel * {
  return _pathDuration;
}

void SourceTheoryRvtMotion::init() {
  _seismicMoment = pow(10, 1.5 * (_magnitude + 10.7));
  _cornerFreq =
      4.9e6 * _shearVelocity * pow(_stressDrop / _seismicMoment, 1. / 3.);

  _hypoDistance = sqrt(_depth * _depth + _distance * _distance);

  if (!_isCustomized) {
    // Compute default value
    calcGeoAtten();
  }

  double sourceDur = 1 / _cornerFreq;
  double pathDur = _pathDuration->duration(_hypoDistance);

  _duration = sourceDur + pathDur;
  emit durationChanged(_duration);
}

void SourceTheoryRvtMotion::calcGeoAtten() {
  if (_hypoDistance > 0) {
    // Determine the geometric attenuation based on a piecewise linear
    // calculation
    switch (_region) {
    case AbstractRvtMotion::WUS:
      if (_hypoDistance < 40.) {
        _geoAtten = 1. / _hypoDistance;
      } else {
        _geoAtten = 1. / 40. * sqrt(40. / _hypoDistance);
      }
      break;
    case AbstractRvtMotion::CEUS:
      if (_hypoDistance < 70.) {
        _geoAtten = 1. / _hypoDistance;
      } else if (_hypoDistance < 130.) {
        _geoAtten = 1. / 70.;
      } else {
        _geoAtten = 1. / 70. * sqrt(130. / _hypoDistance);
      }
      break;
    default:
      break;
    }

    emit geoAttenChanged(_geoAtten);
  }
}

void SourceTheoryRvtMotion::calculate() {
  // Make sure all of the parameters are up-to-date
  init();
  // Conversion factor to convert from dyne-cm into gravity-sec
  const double conv = 1e-20 / 981;

  // Constant term for the model component
  const double C =
      (0.55 * 2) / (M_SQRT2 * 4 * M_PI * _density * pow(_shearVelocity, 3));

  for (int i = 0; i < _fourierAcc.size(); ++i) {
    // Model component
    const double sourceComp =
        C * _seismicMoment / (1 + pow(freqAt(i) / _cornerFreq, 2));

    // Path component
    const double pathAtten = _pathAttenCoeff * pow(freqAt(i), _pathAttenPower);
    const double pathComp =
        _geoAtten *
        exp((-M_PI * freqAt(i) * _hypoDistance) / (pathAtten * _shearVelocity));

    // Site component
    const double siteAmp = _crustalAmp->interpAmpAt(freqAt(i));
    const double siteDim = exp(-M_PI * _siteAtten * freqAt(i));
    const double siteComp = siteAmp * siteDim;

    // Combine the three components and convert from displacement to
    // acceleleration
    _fourierAcc[i] =
        conv * pow(2 * M_PI * freqAt(i), 2) * sourceComp * pathComp * siteComp;
  }

  dataChanged(index(0, AmplitudeColumn),
              index(_fourierAcc.size(), AmplitudeColumn));

  AbstractRvtMotion::calculate();
}

void SourceTheoryRvtMotion::fromJson(const QJsonObject &json) {
  AbstractRvtMotion::fromJson(json);

  _isCustomized = json["isCustomized"].toBool();
  if (_isCustomized) {
    _depth = json["depth"].toDouble();
    _stressDrop = json["stressDrop"].toDouble();
    _geoAtten = json["geoAtten"].toDouble();
    _pathAttenCoeff = json["pathAttenCoeff"].toDouble();
    _pathAttenPower = json["pathAttenPower"].toDouble();
    _shearVelocity = json["shearVelocity"].toDouble();
    _density = json["density"].toDouble();
    _siteAtten = json["siteAtten"].toDouble();

    _crustalAmp->fromJson(json["crustalAmp"].toObject());
    _pathDuration->fromJson(json["pathDuration"].toObject());
  }

  calculate();
}

auto SourceTheoryRvtMotion::toJson() const -> QJsonObject {
  QJsonObject json = AbstractRvtMotion::toJson();
  json["depth"] = _depth;
  json["freq"] = _freq->toJson();
  json["isCustomized"] = _isCustomized;

  if (_isCustomized) {
    json["stresDrop"] = _stressDrop;
    json["geoAtten"] = _geoAtten;
    json["pathAttenCoeff"] = _pathAttenCoeff;
    json["pathAttenPower"] = _pathAttenPower;
    json["shearVelocity"] = _shearVelocity;
    json["density"] = _density;
    json["siteAtten"] = _siteAtten;
    json["crustalAmp"] = _crustalAmp->toJson();
    json["pathDuration"] = _pathDuration->toJson();
  }
  return json;
}

auto operator<<(QDataStream &out, const SourceTheoryRvtMotion *strm)
    -> QDataStream & {
  out << static_cast<quint8>(4);
  out << qobject_cast<const AbstractRvtMotion *>(strm);
  // Properties of SourceTheoryRvtMotion
  out << strm->_depth << strm->_freq << strm->_isCustomized;
  if (strm->_isCustomized) {
    out << strm->_stressDrop << strm->_geoAtten << strm->_pathAttenCoeff
        << strm->_pathAttenPower << strm->_shearVelocity << strm->_density
        << strm->_siteAtten << strm->_crustalAmp << strm->_pathDuration;
  }

  return out;
}

auto operator>>(QDataStream &in, SourceTheoryRvtMotion *strm) -> QDataStream & {
  quint8 ver;
  in >> ver;

  in >> qobject_cast<AbstractRvtMotion *>(strm);

  // Properties of SourceTheoryRvtMotion
  double depth;

  in >> depth >> strm->_freq;
  strm->setDepth(depth);

  if (ver > 3)
    in >> strm->_isCustomized;

  if (strm->_isCustomized) {
    in >> strm->_stressDrop >> strm->_geoAtten >> strm->_pathAttenCoeff >>
        strm->_pathAttenPower >> strm->_shearVelocity >> strm->_density >>
        strm->_siteAtten >> strm->_crustalAmp >> strm->_pathDuration;
  }
  // Compute the FAS
  strm->calculate();
  return in;
}
