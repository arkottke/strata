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

#ifndef LAYER_THICKNESS_VARIATION_H
#define LAYER_THICKNESS_VARIATION_H

#include <QDataStream>
#include <QJsonObject>
#include <QObject>
#include <QStringList>

#include <gsl/gsl_rng.h>

class ProfileRandomizer;

/*! Class for generating layer thicknesses from a non-homogeneous Poisson process.
 *   The layering of the profile is computed from a Poisson process with
 *  depth dependent activity rate, defined as:
 *  \f[ \lambda(d) = a ( d + b ) ^ c \f]
 *  where \f$a\f$, \f$b\f$, and \f$c\f$ are referred to as the layering
 *  coefficient, initial value, and exponent, respectively.
 */

class LayerThicknessVariation : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const LayerThicknessVariation* ltv) -> QDataStream &;
    friend auto operator>> (QDataStream & in, LayerThicknessVariation* ltv) -> QDataStream &;

public:
    explicit LayerThicknessVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer);

    enum Model{
        Custom, //!< Custom values
        Default //!< Default value
    };

    static auto modelList() -> QStringList;

    auto enabled() const -> bool;
    auto model() -> Model;
    void setModel(Model model);

    auto customEnabled() const -> bool;

    auto coeff() const -> double;
    auto initial() const -> double;
    auto exponent() const -> double;

    void reset();

    auto vary(double depthToBedrock) const -> QList<double>;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

signals:
    void enabledChanged(bool enabled);
    void modelChanged(int model);
    void customEnabledChanged(bool customEnabled);
    void coeffChanged(double coeff);
    void initialChanged(double initial);
    void exponentChanged(double exponent);

    void wasModified();
public slots:
    void setEnabled(bool enabled);
    void setModel(int model);
    void setCoeff(double coeff);
    void setInitial(double initial);
    void setExponent(double exponent);


protected slots:
    void updateEnabled();

protected:
    //! If the model is to be used
    bool _enabled;

    //! Model to use
    Model _model;

    //@{ Model parameters
    double _coeff;
    double _initial;
    double _exponent;
    //@}

    //! Random number generator
    gsl_rng* _rng;

    //! Reference to the parent class for controlling if the model is to be used
    ProfileRandomizer* _profileRandomizer;
};

#endif // LAYERTHICKNESSVARIATION_H
