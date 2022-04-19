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

#include "AbstractCalculator.h"
#include "OutputCatalog.h"

#include <qwt_scale_engine.h>

AbstractTimeSeriesOutput::AbstractTimeSeriesOutput(OutputCatalog *catalog)
    : AbstractLocationOutput(catalog) {}

auto AbstractTimeSeriesOutput::fullName() const -> QString {
  QString s = tr("Time Series -- %1 -- %2").arg(prefix()).arg(name());

  if (!suffix().isEmpty())
    s += " -- " + suffix();

  return s;
}

auto AbstractTimeSeriesOutput::needsTime() const -> bool { return true; }

auto AbstractTimeSeriesOutput::baselineCorrect() const -> bool {
  return _baselineCorrect;
}

void AbstractTimeSeriesOutput::setBaselineCorrect(bool baseLineCorrect) {
  if (_baselineCorrect != baseLineCorrect) {
    _baselineCorrect = baseLineCorrect;

    emit baselineCorrectChanged(_baselineCorrect);
    emit wasModified();
  }
}

auto AbstractTimeSeriesOutput::fileName(int motion) const -> QString {
  QString s = prefix() + "-" + shortName();

  if (!suffix().isEmpty())
    s += "-" + suffix();

  s += QString("-M%1").arg(motion + 1, (int)ceil(log10(motionCount() + 1)), 10,
                           QChar('0'));

  return s;
}

auto AbstractTimeSeriesOutput::xScaleEngine() const -> QwtScaleEngine * {
  return new QwtLinearScaleEngine;
}

auto AbstractTimeSeriesOutput::yScaleEngine() const -> QwtScaleEngine * {
  auto *scaleEngine = new QwtLinearScaleEngine;
  scaleEngine->setAttribute(QwtScaleEngine::Symmetric, true);

  return scaleEngine;
}

auto AbstractTimeSeriesOutput::xLabel() const -> const QString {
  return tr("Time (s)");
}

auto AbstractTimeSeriesOutput::ref(int motion) const
    -> const QVector<double> & {
  return _catalog->time(motion);
}

auto AbstractTimeSeriesOutput::suffix() const -> const QString {
  return (_baselineCorrect ? "corrected" : "");
}

auto AbstractTimeSeriesOutput::fieldWidth() const -> int {
  return static_cast<int>(ceil(log10(motionCount() + 1)));
}

void AbstractTimeSeriesOutput::fromJson(const QJsonObject &json) {
  AbstractLocationOutput::fromJson(json);
  _baselineCorrect = json["baselineCorrect"].toBool();
}

auto AbstractTimeSeriesOutput::toJson() const -> QJsonObject {
  QJsonObject json = AbstractLocationOutput::toJson();
  json["baselineCorrect"] = _baselineCorrect;
  return json;
}

auto operator<<(QDataStream &out, const AbstractTimeSeriesOutput *atso)
    -> QDataStream & {
  out << static_cast<quint8>(2);
  out << atso->_baselineCorrect
      << qobject_cast<const AbstractLocationOutput *>(atso);

  return out;
}

auto operator>>(QDataStream &in, AbstractTimeSeriesOutput *atso)
    -> QDataStream & {
  quint8 ver;
  in >> ver;

  in >> atso->_baselineCorrect;

  if (ver == 1) {
    // Version 1 did not save as AbstractTimeSeriesOutput
    in >> qobject_cast<AbstractOutput *>(atso);
  } else {
    in >> qobject_cast<AbstractLocationOutput *>(atso);
  }

  return in;
}
