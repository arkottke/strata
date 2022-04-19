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

#include "SoilTypeOutput.h"

#include "NonlinearPropertyOutput.h"
#include "SoilType.h"

#include <QDebug>

SoilTypeOutput::SoilTypeOutput(SoilType *soilType, OutputCatalog *catalog)
    : QObject(soilType), _soilType(soilType), _enabled(false) {
  _modulus = new NonlinearPropertyOutput(soilType->modulusModel(), catalog);

  _modulus->setSoilName(soilType->name());
  connect(_soilType, SIGNAL(nameChanged(QString)), _modulus,
          SLOT(setSoilName(QString)));
  connect(_soilType, SIGNAL(modulusModelChanged(NonlinearProperty *)), _modulus,
          SLOT(setNonlinearProperty(NonlinearProperty *)));

  _damping = new NonlinearPropertyOutput(soilType->dampingModel(), catalog);
  _damping->setSoilName(soilType->name());
  connect(_soilType, SIGNAL(nameChanged(QString)), _damping,
          SLOT(setSoilName(QString)));
  connect(_soilType, SIGNAL(dampingModelChanged(NonlinearProperty *)), _damping,
          SLOT(setNonlinearProperty(NonlinearProperty *)));
}

auto SoilTypeOutput::soilType() const -> SoilType * { return _soilType; }

auto SoilTypeOutput::name() const -> QString { return _soilType->name(); }

auto SoilTypeOutput::modulus() -> NonlinearPropertyOutput * { return _modulus; }

auto SoilTypeOutput::damping() -> NonlinearPropertyOutput * { return _damping; }

auto SoilTypeOutput::enabled() const -> bool { return _enabled; }

void SoilTypeOutput::setEnabled(bool enabled) {
  _enabled = enabled;
  emit wasModified();
}

void SoilTypeOutput::fromJson(const QJsonObject &json) {
  _enabled = json["enabled"].toBool();
  _modulus->fromJson(json["modulus"].toObject());
  _damping->fromJson(json["damping"].toObject());
}

auto SoilTypeOutput::toJson() const -> QJsonObject {
  QJsonObject json;
  json["enabled"] = _enabled;
  json["modulus"] = _modulus->toJson();
  json["damping"] = _damping->toJson();
  return json;
}

auto operator<<(QDataStream &out, const SoilTypeOutput *sto) -> QDataStream & {
  out << (quint8)1;

  out << sto->_enabled << sto->_modulus << sto->_damping;

  return out;
}

auto operator>>(QDataStream &in, SoilTypeOutput *sto) -> QDataStream & {
  quint8 ver;
  in >> ver;

  in >> sto->_enabled >> sto->_modulus >> sto->_damping;

  return in;
}
