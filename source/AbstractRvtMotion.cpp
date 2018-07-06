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

#include "AbstractRvtMotion.h"

#include "CompatibleRvtMotion.h"
#include "ResponseSpectrum.h"
#include "RvtMotion.h"
#include "Serialize.h"
#include "SourceTheoryRvtMotion.h"
#include "Units.h"
#include "WangRathjePeakCalculator.h"

#include <QApplication>
#include <QDebug>

#include <qwt_scale_engine.h>
#include <qwt_text.h>

AbstractRvtMotion::AbstractRvtMotion(QObject * parent) 
    : AbstractMotion(parent),
    _duration(0),
    _magnitude(6),
    _distance(20),
    _okToContinue(false),
    _peakCalculator(nullptr),
    _region(AbstractRvtMotion::Unknown)
{
    _peakCalculator = new WangRathjePeakCalculator;
    setRegion(WUS);
}

AbstractRvtMotion::~AbstractRvtMotion()
{
    delete _peakCalculator;
}

QStringList AbstractRvtMotion::regionList()
{
    return {tr("Western NA"), tr("Eastern NA")}; //, tr("Unknown")};
}

int AbstractRvtMotion::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return qMin(freq().size(), _fourierAcc.size());
}

int AbstractRvtMotion::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent)
    return 2;
}

QVariant AbstractRvtMotion::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex()) {
        return QVariant();
    }

    if ( role==Qt::DisplayRole || role==Qt::EditRole || role==Qt::UserRole) {
        switch (index.column()) {
            case FrequencyColumn:
                return QString::number(freq().at(index.row()), 'e', 2);
            case AmplitudeColumn:
                return QString::number(_fourierAcc.at(index.row()), 'e', 2);
        }
    }

    return QVariant();
}

QVariant AbstractRvtMotion::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation) {
        case Qt::Horizontal:
            switch (section) {
                case FrequencyColumn:
                    return tr("Frequency (Hz)");
                case AmplitudeColumn:
                    return tr("Amplitude (g-s)");
            }
        case Qt::Vertical:
            return section + 1;
    }

    return QVariant();
}

AbstractRvtMotion::Region AbstractRvtMotion::region() const
{
    return _region;
}

void AbstractRvtMotion::setRegion(AbstractRvtMotion::Region region) {
    if (region != _region) {
        _region = region;
        updatePeakCalculatorScenario();
        emit regionChanged((int)region);
        emit wasModified();
    }
}

void AbstractRvtMotion::setRegion(int region) {
    setRegion((Region)region);

}

double AbstractRvtMotion::magnitude() const {return _magnitude;}

void AbstractRvtMotion::setMagnitude(double magnitude) {
    if (fabs(_magnitude - magnitude) > 1E-4) {
        _magnitude = magnitude;
        updatePeakCalculatorScenario();
        emit magnitudeChanged(magnitude);
        emit wasModified();
    }
}

const QVector<double> & AbstractRvtMotion::fourierAcc() const
{
    return _fourierAcc;
}

double AbstractRvtMotion::max(const QVector<std::complex<double> >& tf ) const
{
    return calcMax(absFourierAcc(tf));
}

double AbstractRvtMotion::maxVel(const QVector<std::complex<double> >& tf) const
{
    return Units::instance()->tsConv() * calcMax(absFourierVel(tf));
}

double AbstractRvtMotion::maxDisp(const QVector<std::complex<double> >& tf) const
{
    return Units::instance()->tsConv() * calcMax(absFourierDisp(tf));
}

QVector<double> AbstractRvtMotion::computeSa(const QVector<double> &period, double damping,
                                             const QVector<std::complex<double> >& accelTf )
{
    // Compute the response at each period
    updatePeakCalculatorScenario();
    QVector<double> sa;
    QVector<double> fourierAcc(_fourierAcc.size());
    for (double oscPeriod : period ) {
        const QVector<std::complex<double> > sdofTf = calcSdofTf(oscPeriod, damping);
        Q_ASSERT(sdofTf.size() == fourierAcc.size());
        
        for (int i = 0; i < fourierAcc.size(); ++i) {
            fourierAcc[i] = abs(sdofTf.at(i)) * _fourierAcc.at(i);
            if (accelTf.size()) {
                fourierAcc[i] *= abs(accelTf.at(i));
            }
        }

        sa << _peakCalculator->calcPeak(
                _duration, freq(), fourierAcc, (1 / oscPeriod), damping, accelTf);
    }

    return sa;
}

const QVector<double> AbstractRvtMotion::absFourierAcc(const QVector<std::complex<double> >& tf) const
{
    QVector<double> absFas(_fourierAcc.size());

    if (!tf.isEmpty()) {
        // Apply the transfer function to the fas
        for (int i = 0; i < _fourierAcc.size(); ++i)
            absFas[i] = abs(tf.at(i)) * _fourierAcc.at(i);
    } else {
        absFas = _fourierAcc;
    }

    return absFas;
}

const QVector<double> AbstractRvtMotion::absFourierVel(const QVector<std::complex<double> >& tf) const
{
    QVector<double> fa = absFourierAcc(tf);

    for (int i = 0; i < fa.size(); ++i) {
        fa[i] /= angFreqAt(i);
    }

    return fa;
}

const QVector<double> AbstractRvtMotion::absFourierDisp(const QVector<std::complex<double> >& tf) const
{
    QVector<double> fa = absFourierAcc(tf);

    for (int i = 0; i < fa.size(); ++i) {
        fa[i] /= pow(angFreqAt(i), 2);
    }

    return fa;
}

double AbstractRvtMotion::calcMaxStrain(const QVector<std::complex<double> >& tf) const
{
    // Remove the scale factor from the maximum velocity
    return maxVel(tf) / Units::instance()->tsConv();
}

double AbstractRvtMotion::freqMax() const
{
    return freq().last();
}

double AbstractRvtMotion::duration() const
{
    return _duration;
}

const QString& AbstractRvtMotion::nameTemplate() const
{
    return _name;
}

QString AbstractRvtMotion::name() const
{
    return QString(_name)
            .replace("$mag",
                         QString::number(_magnitude, 'f', 1),
                         Qt::CaseSensitive)
            .replace("$dist",
                     QString::number(_distance, 'f', 1),
                     Qt::CaseSensitive);
}


void AbstractRvtMotion::setName(const QString &name)
{
    if (_name != name) {
        _name = name;

        emit wasModified();
    }
}

void AbstractRvtMotion::stop()
{
    _okToContinue = false;
}

bool AbstractRvtMotion::loadFromTextStream(QTextStream &stream, double scale)
{
    Q_UNUSED(scale);
    _name = extractColumn(stream.readLine(), 1);
    _description = extractColumn(stream.readLine(), 1);

    bool ok;
    _type = variantToType(extractColumn(stream.readLine(), 1), &ok);
    if (!ok) {
        qWarning() << "Unabled to parse motion type";
        return false;
    }

    _duration = extractColumn(stream.readLine(), 1).toFloat(&ok);
    if (!ok) {
        qWarning() << "Unable to parse duration!";
        return false;
    }

    return true;
}

void AbstractRvtMotion::calculate()
{
    // Compute the PGA and PGV
    setPga(max());
    setPgv(maxVel());

    // Compute the response spectra
    _respSpec->setSa(computeSa(
            _respSpec->period(), _respSpec->damping()));
}

double AbstractRvtMotion::calcMax(const QVector<double> &fourierAmps) const
{
    return _peakCalculator->calcPeak(
            _duration,
            freq(),
            fourierAmps
    );
}

void AbstractRvtMotion::updatePeakCalculatorScenario() {
    if (BooreThompsonPeakCalculator *btpc =
            dynamic_cast<BooreThompsonPeakCalculator*>(_peakCalculator)) {
        if (fabs(_magnitude - btpc->mag()) > 0.01 ||
            fabs(_distance - btpc->dist()) > 0.01 ||
                _region != btpc->region()) {
            btpc->setScenario(_magnitude, _distance, _region);
        }
    }
}

QString AbstractRvtMotion::extractColumn(const QString &line, int column)
{
    QStringList parts = line.split(",");

    if (parts.size() < column) {
        qWarning() << "Improperly formatted line:" << line;
        return "";
    } else
        return parts.at(column);
}

void AbstractRvtMotion::fromJson(const QJsonObject &json)
{
    beginResetModel();

    AbstractMotion::fromJson(json);
    _duration = json["duration"].toDouble();

    _name = json["name"].toString();

    _magnitude = json["magnitude"].toDouble();
    _distance = json["distance"].toDouble();
    setRegion(json["region"].toInt());

    Serialize::toDoubleVector(json["fourierAcc"], _fourierAcc);

    endResetModel();
}

QJsonObject AbstractRvtMotion::toJson() const
{
    QJsonObject json = AbstractMotion::toJson();
    json["duration"] = _duration;
    json["name"] = _name;
    json["region"] = (int)_region;
    json["magnitude"] = _magnitude;
    json["distance"] = _distance;
    json["fourierAcc"] = Serialize::toJsonArray(_fourierAcc);
    return json;
}


QDataStream & operator<< (QDataStream & out, const AbstractRvtMotion* arm)
{
    out << (quint8)3;

    out << qobject_cast<const AbstractMotion*>(arm);

    out << arm->_fourierAcc
        << arm->_duration
        << arm->_name
        << (int)arm->_region
        << arm->_magnitude
        << arm->_distance;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractRvtMotion* arm)
{
    quint8 ver;
    in >> ver;
    in >> qobject_cast<AbstractMotion*>(arm);

    arm->beginResetModel();
    if (ver == 1) {
        int i;
        in >> i;
        Q_UNUSED(i);
    }
    in >> arm->_fourierAcc
       >> arm->_duration
       >> arm->_name;

    if (ver > 1) {
        int region;
        in >> region
            >> arm->_magnitude
            >> arm->_distance;
        arm->setRegion(region);
    }
    arm->endResetModel();
    return in;
}

double AbstractRvtMotion::distance() const {
    return _distance;
}

void AbstractRvtMotion::setDistance(double distance) {
    if (fabs(_distance - distance) > 1E-4) {
        _distance = distance;
        updatePeakCalculatorScenario();
        emit distanceChanged(distance);
        emit wasModified();
    }
}

// FIXME: Might be out of date.
AbstractRvtMotion* loadRvtMotionFromTextFile(const QString &fileName, double scale)
{
    QFile data(fileName);

    if (!data.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open:" << fileName;
        return 0;
    }

    QTextStream in(&data);

    QString line = in.readLine();

    QString className = line.split(',').at(1);

    AbstractRvtMotion *rvtMotion = nullptr;
    if (className == "RvtMotion") {
        rvtMotion = new RvtMotion;
    } else if (className == "CompatibleRvtMotion") {
        rvtMotion = new CompatibleRvtMotion;
    } else if (className == "SourceTheoryRvtMotion") {
        rvtMotion = new SourceTheoryRvtMotion;
    } else {
        qWarning() << "Unrecogized className:" << className;
        return 0;
    }

    if (rvtMotion->loadFromTextStream(in, scale)) {
        return rvtMotion;
    } else {
        return 0;
    }
}
