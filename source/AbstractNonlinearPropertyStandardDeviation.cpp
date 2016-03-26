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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "AbstractNonlinearPropertyStandardDeviation.h"

#include <QtAlgorithms>
#include <QDebug>

#include <cfloat>
#include <cmath>

AbstractNonlinearPropertyStandardDeviation::AbstractNonlinearPropertyStandardDeviation(QObject *parent) :
    QObject(parent)
{
}

double AbstractNonlinearPropertyStandardDeviation::limit(double value) const
{
    return qBound(m_min, value, m_max);
}

double AbstractNonlinearPropertyStandardDeviation::min() const
{
    return m_min;
}

double AbstractNonlinearPropertyStandardDeviation::max() const
{
    return m_max;
}

const QString& AbstractNonlinearPropertyStandardDeviation::function() const
{
    return m_function;
}

bool AbstractNonlinearPropertyStandardDeviation::okSyntax() const
{
    return m_engine.checkSyntax(m_function).state() == QScriptSyntaxCheckResult::Valid;
}

void AbstractNonlinearPropertyStandardDeviation::setMin(double min)
{
    if (fabs(m_min - min) > DBL_EPSILON) {
        m_min = min;

        emit minChanged(m_min);
        emit wasModified();
    }
}

void AbstractNonlinearPropertyStandardDeviation::setMax(double max)
{
    if (fabs(m_max - max) > DBL_EPSILON) {
        m_max = max;

        emit maxChanged(m_max);
        emit wasModified();
    }
}

void AbstractNonlinearPropertyStandardDeviation::setFunction(const QString& function)
{
    if (m_function != function) {
        m_function = function;

        emit functionChanged(m_function);
        emit canEvaluateChanged(okSyntax());
        emit wasModified();
    }
}

double AbstractNonlinearPropertyStandardDeviation::calculate(NonlinearPropertyRandomizer::Model model, double strain, double property)
{
    if (model == NonlinearPropertyRandomizer::Custom) {
        m_engine.globalObject().setProperty("strain", strain);
        setPropertyValue(property);

        QScriptValue result = m_engine.evaluate(m_function);
        if (m_engine.hasUncaughtException()) {
            // FIXME: Remove support for the script engine.
            // int line = m_engine.uncaughtExceptionLineNumber();
            return 0;
        } else {
            return result.toNumber();
        }
    } else {
        return 0;
    }
}

void AbstractNonlinearPropertyStandardDeviation::fromJson(const QJsonObject &json)
{
    m_min = json["min"].toDouble();
    m_max = json["max"].toDouble();
    m_function = json["function"].toString();
}

QJsonObject AbstractNonlinearPropertyStandardDeviation::toJson() const
{
    QJsonObject json;
    json["min"] = m_min;
    json["max"] = m_max;
    json["function"] = m_function;
    return json;
}


QDataStream& operator<< (QDataStream & out, const AbstractNonlinearPropertyStandardDeviation* anpsd)
{
    out << (quint8)1;

    out << anpsd->m_min
            << anpsd->m_max
            << anpsd->m_function;

    return out;
}

QDataStream& operator>> (QDataStream & in, AbstractNonlinearPropertyStandardDeviation* anpsd)
{
    quint8 ver;
    in >> ver;

    in >> anpsd->m_min
            >> anpsd->m_max
            >> anpsd->m_function;

    return in;
}
