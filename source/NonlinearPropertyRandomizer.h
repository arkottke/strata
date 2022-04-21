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

#ifndef NONLINEAR_PROPERTY_VARIATION_H_
#define NONLINEAR_PROPERTY_VARIATION_H_

#include <QDataStream>
#include <QJsonObject>
#include <QObject>
#include <QStringList>
#include <QTextStream>

#include <gsl/gsl_rng.h>

class NonlinearPropertyUncertainty;
class NonlinearProperty;
class RockLayer;
class SoilProfile;
class SoilType;

class NonlinearPropertyRandomizer : public QObject {
  Q_OBJECT

  friend auto operator<<(QDataStream &out,
                         const NonlinearPropertyRandomizer *npr)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, NonlinearPropertyRandomizer *npr)
      -> QDataStream &;

public:
  NonlinearPropertyRandomizer(gsl_rng *rng, SoilProfile *siteProfile);
  ~NonlinearPropertyRandomizer();

  //! Model for the standard deviation
  enum Model {
    SPID,     //!< EPRI SPID model
    Darendeli //!< Defined by Stokoe and Darendeli
  };

  static auto modelList() -> QStringList;

  auto enabled() const -> bool;

  auto model() const -> Model;
  void setModel(Model model);

  auto customEnabled() const -> bool;

  auto bedrockIsEnabled() const -> bool;
  auto correl() const -> double;

  auto dampingUncert() -> NonlinearPropertyUncertainty *;
  auto modulusUncert() -> NonlinearPropertyUncertainty *;

  //! Vary the properties of a given layer
  void vary(SoilType *soilType);

  //! Vary the damping of the bedrock
  void vary(RockLayer *bedrock);

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

public slots:
  void setEnabled(bool enabled);
  void setModel(int model);
  void setBedrockIsEnabled(bool enabled);
  void setCorrel(double correl);

signals:
  void enabledChanged(bool enabled);
  void modelChanged(int model);
  void customEnabledChanged(bool customEnabled);
  void bedrockIsEnabledChanged(bool enabled);
  void correlChanged(double correl);

  void wasModified();

protected slots:
  void updateEnabled();
  void updateBedrockIsEnabled();

private:
  //! If the variation is enabled.
  bool _enabled;

  //! Model for the standard deviation
  Model _model;

  //! If the damping of the bedrock is enabled
  bool _bedrockIsEnabled;

  //! Correlation between shear modulus and damping
  double _correl;

  //! Uncertainty model for the shear modulus reduction
  NonlinearPropertyUncertainty *_modulusUncert;

  //! Uncertainty model for the damping ratio
  NonlinearPropertyUncertainty *_dampingUncert;

  //! Random number generator
  gsl_rng *_rng;

  //! Site response model
  SoilProfile *_siteProfile;
};
#endif
