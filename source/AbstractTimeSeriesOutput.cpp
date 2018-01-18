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

#include "AbstractTimeSeriesOutput.h"

#include "OutputCatalog.h"
#include "AbstractCalculator.h"

#include <qwt_scale_engine.h>

AbstractTimeSeriesOutput::AbstractTimeSeriesOutput(OutputCatalog* catalog)
    : AbstractLocationOutput(catalog)
{
}

 QString AbstractTimeSeriesOutput::fullName() const
 {
     QString s = tr("Time Series -- %1 -- %2")
                 .arg(prefix())
                 .arg(name());

     if (!suffix().isEmpty())
         s += " -- " + suffix();

     return s;
 }

 bool AbstractTimeSeriesOutput::needsTime() const
 {
     return true;
 }

bool AbstractTimeSeriesOutput::baselineCorrect() const
{
    return _baselineCorrect;
}

void AbstractTimeSeriesOutput::setBaselineCorrect(bool baseLineCorrect)
{
    if (_baselineCorrect != baseLineCorrect) {
        _baselineCorrect = baseLineCorrect;

        emit baselineCorrectChanged(_baselineCorrect);
        emit wasModified();
    }
}

QString AbstractTimeSeriesOutput::fileName(int motion) const
{
    QString s = prefix() + "-" + shortName();

    if (!suffix().isEmpty())
        s += "-" + suffix();

    s += QString("-M%1").arg(motion+1, (int)ceil(log10(motionCount()+1)), 10, QChar('0'));

    return s;
}

QwtScaleEngine* AbstractTimeSeriesOutput::xScaleEngine() const
{
    return new QwtLinearScaleEngine;
}

QwtScaleEngine* AbstractTimeSeriesOutput::yScaleEngine() const
{
    QwtLinearScaleEngine* scaleEngine = new QwtLinearScaleEngine;
    scaleEngine->setAttribute(QwtScaleEngine::Symmetric, true);

    return scaleEngine;
}

const QString AbstractTimeSeriesOutput::xLabel() const
{
    return tr("Time (s)");
}

const QVector<double>& AbstractTimeSeriesOutput::ref(int motion) const
{

    return _catalog->time(motion);
}

const QString AbstractTimeSeriesOutput::suffix() const
{
    return (_baselineCorrect ? "corrected" : "");
}

int AbstractTimeSeriesOutput::fieldWidth() const
{
    return (int)ceil(log10(motionCount()+1));
}

void AbstractTimeSeriesOutput::fromJson(const QJsonObject &json)
{
    AbstractLocationOutput::fromJson(json);
    _baselineCorrect = json["baselineCorrect"].toBool();
}

QJsonObject AbstractTimeSeriesOutput::toJson() const
{
    QJsonObject json = AbstractLocationOutput::toJson();
    json["baselineCorrect"] = _baselineCorrect;
    return json;
}


QDataStream & operator<< (QDataStream & out, const AbstractTimeSeriesOutput* atso)
{
    out << (quint8)2;

    out << atso->_baselineCorrect << qobject_cast<const AbstractLocationOutput*>(atso);

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractTimeSeriesOutput* atso)
{
    quint8 ver;
    in >> ver;

    in >> atso->_baselineCorrect;

    if (ver == 1) {
        // Version 1 did not save as AbstractTimeSeriesOutput
        in >> qobject_cast<AbstractOutput*>(atso);
    } else {
        in >> qobject_cast<AbstractLocationOutput*>(atso);
    }

    return in;
}
