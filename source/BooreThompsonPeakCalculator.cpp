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

#include <QFile>
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

    const int total = _nmags * _ndists;

    _interp = gsl_interp2d_alloc(gsl_interp2d_bilinear, _nmags, _ndists);
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

        auto data = QMap<QString, QVector<double> >();
        auto mag = QVector<double>(_nmags);
        for (int i = 0; i < _nmags; ++i) {
            mag[i] = paramMap["M"].at(i);
        }
        data["mag"] = mag;

        // Pre-compute log distances
        auto lnDist = QVector<double>(_ndists);
        for (int i = 0; i < _ndists; ++i) {
            lnDist[i] = log(paramMap["R"].at(i * _nmags));
        }
        data["lnDist"] = lnDist;

        for (const QString &key : {"c1", "c2", "c3", "c4", "c5", "c6", "c7"}) {
           data[key] = paramMap[key];
        }
        _tabularData[region] = data;
    }
}

BooreThompsonPeakCalculator::~BooreThompsonPeakCalculator() {
    gsl_interp2d_free(_interp);
    gsl_interp_accel_free(_magAcc);
    gsl_interp_accel_free(_lnDistAcc);
}

auto BooreThompsonPeakCalculator::mag() const -> double{
    return _mag;
}

auto BooreThompsonPeakCalculator::dist() const -> double{
    return _dist;
}

auto BooreThompsonPeakCalculator::region() const -> AbstractRvtMotion::Region {
    return _region;
}

auto BooreThompsonPeakCalculator::interpCoeff(double mag, double lnDist, QMap<QString, QVector<double> > &data, const QString &key) -> double {

    gsl_interp2d_init(
        _interp, data["mag"].data(), data["lnDist"].data(), data[key].data(), _nmags, _ndists);

    return gsl_interp2d_eval(
                _interp,
                data["mag"].data(), data["lnDist"].data(), data[key].data(),
                mag, lnDist, _magAcc, _lnDistAcc);
}

void BooreThompsonPeakCalculator::setScenario(double mag, double dist, AbstractRvtMotion::Region region)
{
    Q_ASSERT(region != AbstractRvtMotion::Unknown);
    
    _mag = mag;
    _dist = dist;
    _region = region;

    QString regionStr = region == AbstractRvtMotion::CEUS ? "cena" : "wna";
    auto data = _tabularData[regionStr];

    double lnDist = log(dist);

    // Clip to the provided values
    mag = std::max(data["mag"].first(), std::min(mag, data["mag"].last()));
    lnDist = std::max(data["lnDist"].first(), std::min(lnDist, data["lnDist"].last()));

    for (const QString &key : {"c1", "c2", "c3", "c4", "c5", "c6", "c7"}) {
        _interped[key] = interpCoeff(mag, lnDist, data, key);
    }
}

auto BooreThompsonPeakCalculator::calcDurationRms(double duration, double oscFreq, double oscDamping, const QVector<std::complex<double> > &siteTransFunc) -> double
{
    Q_UNUSED(siteTransFunc);
    if (oscFreq > 0 && oscDamping > 0) {
        double foo = 1 / (oscFreq * duration);
        double durRatio = ((_interped["c1"] + _interped["c2"] *
                            (1 - pow(foo, _interped["c3"])) / (1 + pow(foo, _interped["c3"]))) *
                           (1 + _interped["c4"] / (2 * M_PI * oscDamping / 100) *
                            pow(foo / (1 + _interped["c5"] * pow(foo, _interped["c6"])), _interped["c7"])));
        duration *= durRatio;
    }
    return duration;
}
