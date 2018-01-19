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

#include "AbstractMotion.h"

#include "Dimension.h"
#include "ResponseSpectrum.h"
#include "Units.h"

#include <QDataStream>
#include <QDebug>

#include <gsl/gsl_math.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

AbstractMotion::AbstractMotion(QObject * parent)
    : MyAbstractTableModel(parent)
{
    _enabled = true;
    _type = Outcrop;
    _respSpec = new ResponseSpectrum;    
    // Default characteristics of the response spectrum
    _respSpec->setDamping(5.0);
    _respSpec->setPeriod(Dimension::logSpace(0.01, 5, 60));

    connect(_respSpec, SIGNAL(wasModified()), SLOT(setModified()));
    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(scaleProperties()));
}

AbstractMotion::~AbstractMotion()
{
    _respSpec->deleteLater();
}

QStringList AbstractMotion::typeList()
{
#ifdef ADVANCED_FEATURES
    return QStringList() << tr("Outcrop (2A)")
            << tr("Within (A+B)")
            << tr("Incoming Only (A)");
#else
    return QStringList() << tr("Outcrop")
            << tr("Within");
#endif
}

AbstractMotion::Type AbstractMotion::variantToType(QVariant variant, bool* ok)
{
    *ok = false;
    Type type = Outcrop;

    bool _ok;
    int index = variant.toInt(&_ok);

    if (!_ok) {
        // Try to figure out the type as a string
        index = 0;
        QString s = variant.toString();

        // Need to split about the type because when advance options are turned on the name includes the wave parts
        foreach (QString possibleType, typeList()) {
            if (possibleType.startsWith(s))
                break;

            ++index;
        }
    }

    // Check if everything is valid
    if (index >= 0 && index < typeList().size()) {
        type = (Type)index;
        *ok = true;
    }

    return type;
}

bool AbstractMotion::modified() const
{
    return _modified;
}

void AbstractMotion::setModified(bool modified)
{
    _modified = modified;

    if (_modified)
        emit wasModified();
}

AbstractMotion::Type AbstractMotion::type() const
{ 
    return _type;
}

void AbstractMotion::setType(int type)
{
    setType((Type)type);
    setModified(true);
}

void AbstractMotion::setType(AbstractMotion::Type type)
{
    _type = type;
    setModified(true);
}


QString AbstractMotion::description() const
{
    return _description;
}

void AbstractMotion::setDescription(QString s)
{
    if (_description != s) {
        emit descriptionChanged(s);
        setModified(true);
    }

    _description = s;
}

bool AbstractMotion::enabled() const
{
    return _enabled;
}

void AbstractMotion::setEnabled(bool enabled)
{
    _enabled = enabled;
}

ResponseSpectrum* AbstractMotion::respSpec()
{
    return _respSpec;
}

double AbstractMotion::pga() const
{
    return _pga;
}

double AbstractMotion::pgv() const
{
    return _pgv;
}

double AbstractMotion::freqMin() const
{
    return freq().first();
}

int AbstractMotion::freqCount() const
{
    return freq().size();
}

double AbstractMotion::angFreqAt( int i ) const
{
    return 2 * M_PI * freqAt(i);
}

double AbstractMotion::freqAt( int i ) const
{
    return freq().at(i);
}

void AbstractMotion::scaleProperties()
{
    switch (Units::instance()->system()) {
    case Units::Metric:
        // Units changed from English to Metric

        // Convert PGV from in/sec to cm/sec
        setPgv(_pgv * 2.54);
        break;
    case Units::English:
        // Units changed from English to Metric

        // Convert PGV from cm/sec to in/sec
        setPgv(_pgv * 0.393700787);
        break;
    }
}

QVector<std::complex<double> > AbstractMotion::calcSdofTf(const double period, const double damping) const
{
    QVector<std::complex<double> > tf(freq().size(), std::complex<double>(0., 0.));

    // Natural frequency of the oscillator
    const double fn = 1 / period;

    for (int i = 0; i < freqCount(); i++) {
        //
        // The single degree of freedom transfer function
        //                          - fn^2
        //  H = -------------------------------------------------
        //       ( f^2 - fn^2 ) - 2 * sqrt(-1) * damping * fn * f
        //
        tf[i] = (-fn * fn) / std::complex<double>(freqAt(i) * freqAt(i) - fn * fn, -2 * (damping / 100) * fn * freqAt(i));
    }

    return tf;
}

void AbstractMotion::setPga(double pga)
{
    _pga = pga;
    emit pgaChanged(_pga);
}

void AbstractMotion::setPgv(double pgv)
{
    _pgv = pgv;
    emit pgvChanged(_pgv);
}

void AbstractMotion::fromJson(const QJsonObject &json)
{
    _description = json["description"].toString();
    _enabled = json["enabled"].toVariant().toBool();
    // _enabled = json["enabled"].toBool();

    setType(json["type"].toInt());
}

QJsonObject AbstractMotion::toJson() const
{
    QJsonObject json;

    json["className"] = metaObject()->className();
    json["type"] = (int) _type;
    json["description"] = _description;
    json["enabled"] = _enabled;

    return json;
}

QDataStream & operator<< (QDataStream & out, const AbstractMotion* m)
{
    out << (quint8)1;

    out << (int)m->_type
        << m->_description
        << m->_enabled;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractMotion* m)
{
    quint8 ver;
    in >> ver;

    int type;
    in >> type
       >> m->_description
       >> m->_enabled;

    m->setType(type);

    return in;
}
