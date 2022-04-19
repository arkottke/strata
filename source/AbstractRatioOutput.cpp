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

#include "AbstractRatioOutput.h"

#include "Algorithms.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"

#include <QDebug>

AbstractRatioOutput::AbstractRatioOutput(OutputCatalog *catalog)
    : AbstractOutput(catalog) {
  _inDepth = -1;
  _inType = AbstractMotion::Outcrop;
  _outDepth = 0;
  _outType = AbstractMotion::Outcrop;

  _statistics = new OutputStatistics(this);
  connect(_statistics, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
}

auto AbstractRatioOutput::fullName() const -> QString {
  return tr("Ratio -- %1 -- %2").arg(prefix()).arg(name());
}

auto AbstractRatioOutput::inDepth() const -> double { return _inDepth; }

auto AbstractRatioOutput::inType() const -> AbstractMotion::Type {
  return _inType;
}

void AbstractRatioOutput::setInType(AbstractMotion::Type inType) {
  if (_inType != inType) {
    _inType = inType;

    emit inTypeChanged(_inType);
    emit wasModified();
  }
}

auto AbstractRatioOutput::outDepth() const -> double { return _outDepth; }

auto AbstractRatioOutput::outType() const -> AbstractMotion::Type {
  return _outType;
}

void AbstractRatioOutput::setOutType(AbstractMotion::Type outType) {
  if (_outType != outType) {
    _outType = outType;

    emit outTypeChanged(_outType);
    emit wasModified();
  }
}

void AbstractRatioOutput::setInDepth(double inDepth) {
  if (_inDepth != inDepth) {
    _inDepth = inDepth;

    emit inDepthChanged(_inDepth);
    emit wasModified();
  }
}

void AbstractRatioOutput::setInType(int inType) {
  setInType((AbstractMotion::Type)inType);
}

void AbstractRatioOutput::setOutDepth(double outDepth) {
  if (_outDepth != outDepth) {
    _outDepth = outDepth;

    emit outDepthChanged(_outDepth);
    emit wasModified();
  }
}

void AbstractRatioOutput::setOutType(int outType) {
  setOutType((AbstractMotion::Type)outType);
}

auto AbstractRatioOutput::fileName(int motion) const -> QString {
  Q_UNUSED(motion);

  return prefix() + '-' + shortName();
}

auto AbstractRatioOutput::prefix() const -> const QString {
  return QString("%1 (%2) from %3 (%4)")
      .arg(locationToString(_outDepth))
      .arg(AbstractMotion::typeList().at(_outType))
      .arg(locationToString(_inDepth))
      .arg(AbstractMotion::typeList().at(_inType));
}

void AbstractRatioOutput::fromJson(const QJsonObject &json) {
  AbstractOutput::fromJson(json);
  _outType = (AbstractMotion::Type)json["outType"].toInt();
  _inType = (AbstractMotion::Type)json["inType"].toInt();
  _outDepth = json["outDepth"].toDouble();
  _inDepth = json["inDepth"].toDouble();
}

auto AbstractRatioOutput::toJson() const -> QJsonObject {
  QJsonObject json = AbstractOutput::toJson();
  json["outType"] = (int)_outType;
  json["inType"] = (int)_inType;
  json["outDepth"] = _outDepth;
  json["inDepth"] = _inDepth;

  return json;
}

auto operator<<(QDataStream &out, const AbstractRatioOutput *aro)
    -> QDataStream & {
  out << (quint8)1;

  out << static_cast<const AbstractOutput *>(aro) << (int)aro->_outType
      << aro->_outDepth << (int)aro->_inType << aro->_inDepth;

  return out;
}

auto operator>>(QDataStream &in, AbstractRatioOutput *aro) -> QDataStream & {
  quint8 ver;
  in >> ver;

  int outType;
  int inType;

  in >> static_cast<AbstractOutput *>(aro) >> outType >> aro->_outDepth >>
      inType >> aro->_inDepth;

  aro->_outType = (AbstractMotion::Type)outType;
  aro->_inType = (AbstractMotion::Type)inType;

  return in;
}
