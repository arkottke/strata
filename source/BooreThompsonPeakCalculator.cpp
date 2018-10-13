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


#include "BooreThompsonPeakCalculator.h"
#include "Serialize.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include <cmath>

BooreThompsonPeakCalculator::BooreThompsonPeakCalculator() {
    _mag = -1;
    _dist = -1;
    _region = AbstractRvtMotion::Unknown;
    _name = "Boore & Thompson (2015)";
    _nmags = 13;
    _ndists = 15;

    _interp = gsl_spline2d_alloc(gsl_interp2d_bilinear, _nmags, _ndists);
    _magAcc = gsl_interp_accel_alloc();
    _lnDistAcc = gsl_interp_accel_alloc();

    // Load the model coefficients
    for (QString region : {"cena", "wna"}) {
        // Load the JSON and convert to vectors
        QString fileName = QString(":/data/%1_bt15_trms4osc.json").arg(region).toUtf8();

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical("Unable to open file: %s", qPrintable(fileName));
            continue;
        }

        QByteArray savedData = file.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(savedData);
        QJsonObject json = jsonDoc.object();

        QMap<QString, QVector<double>> paramMap;
        for (QString key : json.keys()) {
            Serialize::toDoubleVector(json[key], paramMap[key]);
        }

        // Extract the values into double arrays
        double *mags = new double[_nmags];
        for (size_t i = 0; i < _nmags; ++i) {
            mags[i] = paramMap["M"].at(i);
        }

        double *lnDists = new double[_ndists];
        for (size_t i = 0; i < _ndists; ++i) {
            lnDists[i] = log(paramMap["R"].at(i * _nmags));
        }

        QMap<QString, double*> coeffs;
        for (const QString &key : paramMap.keys()) {
            if (!key.startsWith("c")) {
                continue;
            }
            const QVector<double> &values = paramMap[key];
            double *c = new double[_nmags * _ndists];

            for (size_t i=0; i < _nmags; ++i) {
                for (size_t j=0; j < _ndists; ++j) {
                    gsl_spline2d_set(_interp, c, i, j, values.at(i + j * _nmags));
                }
            }
            coeffs[key] = c;
        }

        _params[region] = BooreThompsonParams{mags, lnDists, coeffs};
    }
}

BooreThompsonPeakCalculator::~BooreThompsonPeakCalculator() {
    gsl_spline2d_free(_interp);
    gsl_interp_accel_free(_magAcc);
    gsl_interp_accel_free(_lnDistAcc);

    // FIXME delete params
}

double BooreThompsonPeakCalculator::mag() const{
    return _mag;
}

double BooreThompsonPeakCalculator::dist() const{
    return _dist;
}

AbstractRvtMotion::Region BooreThompsonPeakCalculator::region() const {
    return _region;
}

double BooreThompsonPeakCalculator::interpCoeff(double mag, double lnDist, BooreThompsonParams &params, const QString &key) {

    gsl_spline2d_init(
        _interp, params.mags, params.lnDists, params.coeffs[key], _nmags, _ndists);

    return gsl_spline2d_eval(
                _interp, mag, lnDist, _magAcc, _lnDistAcc);
}


void BooreThompsonPeakCalculator::setScenario(double mag, double dist, AbstractRvtMotion::Region region)
{
    Q_ASSERT(region != AbstractRvtMotion::Unknown);

    QString regionStr = region == AbstractRvtMotion::CEUS ? "cena" : "wna";
    BooreThompsonParams &p = _params[regionStr];

    double lnDist = log(dist);

    // Clip to the provided values
    mag = std::max(p.mags[0], std::min(mag, p.mags[_nmags - 1]));
    lnDist = std::max(p.lnDists[0], std::min(lnDist, p.lnDists[_ndists - 1]));

    _interped = BooreThompsonCoeffs{
            interpCoeff(mag, lnDist, p, "c1"),
            interpCoeff(mag, lnDist, p, "c2"),
            interpCoeff(mag, lnDist, p, "c3"),
            interpCoeff(mag, lnDist, p, "c4"),
            interpCoeff(mag, lnDist, p, "c5"),
            interpCoeff(mag, lnDist, p, "c6"),
            interpCoeff(mag, lnDist, p, "c7")
    };
}

double BooreThompsonPeakCalculator::calcDurationRms(double duration, double oscFreq, double oscDamping, const QVector<std::complex<double> > &siteTransFunc)
{
    Q_UNUSED(siteTransFunc);

    if (oscFreq > 0 && oscDamping > 0) {
        double foo = 1 / (oscFreq * duration);
        double durRatio = ((_interped.c1 + _interped.c2 *
                            (1 - pow(foo, _interped.c3)) / (1 + pow(foo, _interped.c3))) *
                           (1 + _interped.c4 / (2 * M_PI * oscDamping) *
                            pow(foo / (1 + _interped.c5 * pow(foo, _interped.c6)), _interped.c7)));
        duration *= durRatio;
    }
    return duration;
}
