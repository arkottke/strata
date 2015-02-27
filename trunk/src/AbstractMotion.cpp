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

#include "AbstractMotion.h"

#include "Dimension.h"
#include "ResponseSpectrum.h"
#include "Units.h"

#include <QDebug>

#include <cmath>
#include <iostream>

AbstractMotion::AbstractMotion(QObject * parent)
    : MyAbstractTableModel(parent)
{
    m_enabled = true;
    m_type = Outcrop;
    m_respSpec = new ResponseSpectrum;    
    // Default characteristics of the response spectrum
    m_respSpec->setDamping(5.0);
    m_respSpec->setPeriod(Dimension::logSpace(0.01, 5, 60));

    connect(m_respSpec, SIGNAL(wasModified()), SLOT(setModified()));
    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(scaleProperties()));
}

AbstractMotion::~AbstractMotion()
{
    m_respSpec->deleteLater();
}

QStringList AbstractMotion::typeList()
{
#ifdef ADVANCED_OPTIONS
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
    return m_modified;
}

void AbstractMotion::setModified(bool modified)
{
    m_modified = modified;

    if (m_modified)
        emit wasModified();
}

AbstractMotion::Type AbstractMotion::type() const
{ 
    return m_type;
}

void AbstractMotion::setType(int type)
{
    setType((Type)type);
    setModified(true);
}

void AbstractMotion::setType(AbstractMotion::Type type)
{
    m_type = type;
    setModified(true);
}


QString AbstractMotion::description() const
{
    return m_description;
}

void AbstractMotion::setDescription(QString s)
{
    if (m_description != s) {
        emit descriptionChanged(s);
        setModified(true);
    }

    m_description = s;
}

bool AbstractMotion::enabled() const
{
    return m_enabled;
}

void AbstractMotion::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

ResponseSpectrum* AbstractMotion::respSpec()
{
    return m_respSpec;
}

double AbstractMotion::pga() const
{
    return m_pga;
}

double AbstractMotion::pgv() const
{
    return m_pgv;
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
        setPgv(m_pgv * 2.54);
        break;
    case Units::English:
        // Units changed from English to Metric

        // Convert PGV from cm/sec to in/sec
        setPgv(m_pgv * 0.393700787);
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
    m_pga = pga;
    emit pgaChanged(m_pga);
}

void AbstractMotion::setPgv(double pgv)
{
    m_pgv = pgv;
    emit pgvChanged(m_pgv);
}

void AbstractMotion::ptRead(const ptree &pt)
{
    int type = pt.get<int>("type");
    m_description = QString::fromStdString(pt.get<std::string>("description"));
    m_enabled = pt.get<bool>("enabled");

    setType(type);
}

void AbstractMotion::ptWrite(ptree &pt) const
{
    pt.put("type", (int) m_type);
    pt.put("description", m_description.toStdString());
    pt.put("enabled", m_enabled);
}

QDataStream & operator<< (QDataStream & out, const AbstractMotion* m)
{
    out << (quint8)1;

    out << (int)m->m_type
            << m->m_description
            << m->m_enabled;


    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractMotion* m)
{
    quint8 ver;
    in >> ver;

    int type;

    in >> type
            >> m->m_description
            >> m->m_enabled;

    m->setType(type);

    return in;
}
