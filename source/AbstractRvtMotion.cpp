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
#include "SourceTheoryRvtMotion.h"
#include "Units.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonValue>

#include <qwt_scale_engine.h>
#include <qwt_text.h>

AbstractRvtMotion::AbstractRvtMotion(QObject * parent) : AbstractMotion(parent)
{    
    _oscCorrection = LiuPezeshk;
    _duration = 0;
    _okToContinue = false;
    _workspace = gsl_integration_workspace_alloc(1024);
}

AbstractRvtMotion::~AbstractRvtMotion()
{
    gsl_integration_workspace_free(_workspace);
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

const QVector<double> & AbstractRvtMotion::fourierAcc() const
{
    return _fourierAcc;
}

double AbstractRvtMotion::max(const QVector<std::complex<double> >& tf ) const
{
    return calcMax(absFourierAcc(tf), _duration);
}

double AbstractRvtMotion::maxVel(const QVector<std::complex<double> >& tf) const
{   
    return Units::instance()->tsConv() * calcMax(absFourierVel(tf), _duration);
}

double AbstractRvtMotion::maxDisp(const QVector<std::complex<double> >& tf) const
{
    return Units::instance()->tsConv() * calcMax(absFourierDisp(tf), _duration);
}

QVector<double> AbstractRvtMotion::computeSa(const QVector<double> &period, double damping, const QVector<std::complex<double> >& tf )
{
    QVector<double> sa(period.size());
    QVector<double> fas = _fourierAcc;

    // Apply the transfer function to the motion
    if (!tf.isEmpty()) {
        Q_ASSERT(fas.size() == tf.size());

        for (int i = 0; i < fas.size(); ++i) {
            fas[i] *= abs(tf.at(i));
        }
    }

    // Compute the response at each period
    for ( int i = 0; i < sa.size(); ++i ) {
        sa[i] = calcOscillatorMax(fas, period.at(i), damping);
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

    for (int i = 0; i < fa.size(); ++i)
        fa[i] /= angFreqAt(i);

    return fa;
}

const QVector<double> AbstractRvtMotion::absFourierDisp(const QVector<std::complex<double> >& tf) const
{
    QVector<double> fa = absFourierAcc(tf);

    for (int i = 0; i < fa.size(); ++i)
        fa[i] /= pow(angFreqAt(i), 2);

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

QString AbstractRvtMotion::name() const
{
    return _name;
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

void AbstractRvtMotion::setOscCorrection(OscillatorCorrection oscCorrection)
{
    if (_oscCorrection != oscCorrection) {
         _oscCorrection = oscCorrection;

         emit oscCorrectionChanged(_oscCorrection);
         emit wasModified();
    }
}

void AbstractRvtMotion::setOscCorrection(int oscCorrection)
{
    setOscCorrection((OscillatorCorrection)oscCorrection);
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

struct peakFactorParams
{
    double bandWidth;
    double numExtrema;
};

double AbstractRvtMotion::calcMax( const QVector<double> & fas, double durationRms ) const
{
    if ( durationRms < 0 ) {
        durationRms = _duration;
    }

    // Square the Fourier amplitude spectrum
    QVector<double> fasSqr(fas.size());
    for ( int i = 0; i < fasSqr.size(); ++i ) {
        fasSqr[i] = fas.at(i) * fas.at(i);
    }

    // The zero moment is the area of the spectral density computed by the
    // trapezoid rule.
    double m0 = moment( 0, fasSqr);
    // double m1 = moment( 1, fasSqr); // FIXME Remove this
    double m2 = moment( 2, fasSqr);
    double m4 = moment( 4, fasSqr);

    // Compute the bandwidth
    double bandWidth = sqrt((m2 * m2 )/( m0 * m4));

    // Compute the number extrema
    double numExtrema = sqrt(m4/m2) * _duration / M_PI;

    // If the number of extrema is less than 2, increase to 2.  There must be
    // one full cycle (two peaks)
    if (numExtrema <2) {
        numExtrema = 2;
    }

    // Use GSL to integrate the peak factor equation
    struct peakFactorParams params = { bandWidth, numExtrema };
    gsl_function F = { &peakFactorEqn, &params};
    double result;
    double error;

    // Try the adaptive integration without bounds
    if ( bandWidth != bandWidth || numExtrema != numExtrema ) {
        // We have issues!
        QString fileName;
        int i = 0;

        do {
            fileName = QString("rvtInfo-%1.dat").arg(i);
        } while (QFile::exists(fileName));

        QFile file(fileName);

        if (file.open(QFile::WriteOnly)) {
            QTextStream fout(&file);

            fout << "durationGm: " << _duration << endl;
            fout << "durationRms: " << durationRms << endl;
            fout << "m0: " << m0 << endl;
            fout << "m2: " << m2 << endl;
            fout << "m4: " << m4 << endl;
            fout << "bandWidth: " << bandWidth << endl;
            fout << "numExtrema: " << numExtrema << endl;

            for (int i = 0; i < fas.size(); ++i) {
                fout << i << " " << fas.at(i) << endl;
            }
        }

        result = 1.0;
    } else {
        gsl_integration_qagiu( &F, 0, 0, 1e-7, 1000, _workspace, &result, &error);
    }

    const double peakFactor = sqrt(2) * result;

    // fout << bandWidth << ","
    //     << sqrt(1 - m1 * m1 / m0 / m2 ) << ","
    //     << numExtrema << ","
    //     << peakFactor << ","
    //     << sqrt(m0/durationRms) << endl;

    // FIXME Compute the peak factor using the asympototic solution
    // double numZero = sqrt(m2/m0) * durationGm / M_PI;
    // double peakFactor_asym = sqrt(2 * log(numZero)) + 0.5772 / sqrt(2 * log(numZero));

    // Return the peak value which is found by multiplying the variation by the peakfactor
    return sqrt(m0/durationRms) * peakFactor;
}

double AbstractRvtMotion::calcOscillatorMax(QVector<double> fas, const double period, const double damping) const
{
    const QVector<std::complex<double> > tf = calcSdofTf(period, damping);

    Q_ASSERT(tf.size() == fas.size());

    for (int i = 0; i < fas.size(); ++i) {
        fas[i] *= abs(tf.at(i));
    }

    return calcMax(fas, calcRmsDuration(period, damping, fas));
}

double AbstractRvtMotion::moment( int power, const QVector<double> & fasSqr) const
{
    // The moment is found by:
    //           /
    // _n = 2 * | ( 2 * pi * freq )^n * FAS^2 * df
    //           /
    double sum = 0;
    double dFreq = 0;
    double last = pow( 2 * M_PI * freq().at(0), power) * fasSqr.at(0);
    double current = 0.0;
    // Integrate using the trapezoid rule
    for (int i=1; i < fasSqr.size(); ++i) {
        // Compute the current piece
        current = pow( 2 * M_PI * freq().at(i), power) * fasSqr.at(i);

        // Frequency may be increasing or decreasing, just want the difference
        dFreq = fabs(freq().at(i) - freq().at(i-1));

        // Compute the area of the trapezoid defined by the current and last value
        sum += dFreq * ( current + last ) / 2;
        // Save the current piece
        last = current;
    }

    return 2 * sum;
}

double AbstractRvtMotion::peakFactorEqn(double z, void * p)
{
    struct peakFactorParams * params = (struct peakFactorParams *)p;

    return 1 - pow( 1 - params->bandWidth * exp(-z*z), params->numExtrema );
}

double AbstractRvtMotion::calcRmsDuration(const double period, const double damping, const QVector<double> & fas) const
{
    // Use BooreJoyner if there is no FAS defined
    OscillatorCorrection oscCorrection = fas.isEmpty() ? BooreJoyner : _oscCorrection;

    // Duration of the oscillator
    const double durOsc = period / (2 * M_PI * damping / 100);

    int power = 0;
    double bar = 0;

    switch (oscCorrection)  {
    case BooreJoyner:
        power = 3;
        bar = 1. / 3.;
        break;

    case LiuPezeshk:
        QVector<double> fasSqr(fas.size());

        for (int i = 0; i < fas.size(); ++i)
            fasSqr[i] = fas.at(i) * fas.at(i);

        const double m0 = moment(0, fasSqr);
        const double m1 = moment(1, fasSqr);
        const double m2 = moment(2, fasSqr);

        power = 2;
        bar = sqrt( 2 * M_PI * ( 1 - (m1 * m1)/(m0 * m2)));
        break;
    }

    const double foo = pow(_duration / period, power);

    const double durationRms = _duration + durOsc * (foo / (foo + bar));

    return durationRms;
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
    bool oscCorrection = json["oscCorrection"].toBool();
    setOscCorrection(oscCorrection);
    _duration = json["duration"].toDouble();
    _name = json["name"].toString();

    _fourierAcc.clear();
    foreach (const QJsonValue &v, json["fourierAcc"].toArray())  {
        _fourierAcc << v.toDouble();
    }

    endResetModel();
}

QJsonObject AbstractRvtMotion::toJson() const
{
    QJsonObject json = AbstractMotion::toJson();
    json["oscCorrection"] = _oscCorrection;
    json["duration"] = _duration;
    json["name"] = _name;

    QJsonArray fourierAcc;
    foreach (const double &d, _fourierAcc) {
        fourierAcc << QJsonValue(d);
    }
    json["fourierAcc"] = fourierAcc;

    return json;
}


QDataStream & operator<< (QDataStream & out, const AbstractRvtMotion* arm)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractMotion*>(arm);

    out << (int)arm->_oscCorrection
            << arm->_fourierAcc
            << arm->_duration
            << arm->_name;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractRvtMotion* arm)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractMotion*>(arm);

    int oscCorrection;

    arm->beginResetModel();
    in >> oscCorrection
            >> arm->_fourierAcc
            >> arm->_duration
            >> arm->_name;

    arm->setOscCorrection(oscCorrection);
    arm->endResetModel();

    return in;
}

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

    AbstractRvtMotion *rvtMotion = 0;
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
