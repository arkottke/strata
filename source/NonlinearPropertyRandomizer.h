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

class NonlinearPropertyRandomizer : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const NonlinearPropertyRandomizer* npr);
    friend QDataStream & operator>> (QDataStream & in, NonlinearPropertyRandomizer* npr);

public:
    NonlinearPropertyRandomizer(gsl_rng* rng, SoilProfile* siteProfile);
    ~NonlinearPropertyRandomizer();

    //! Model for the standard deviation
    enum Model {
        SPID, //!< EPRI SPID model
        Darendeli //!< Defined by Stokoe and Darendeli
    };

    static QStringList modelList();

    bool enabled() const;

    Model model() const;
    void setModel(Model model);

    bool customEnabled() const;

    bool bedrockIsEnabled() const;
    double correl() const;

    NonlinearPropertyUncertainty* dampingUncert();
    NonlinearPropertyUncertainty* modulusUncert();

    //! Vary the properties of a given layer
    void vary(SoilType* soilType);

    //! Vary the damping of the bedrock
    void vary(RockLayer* bedrock);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

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
    int m_enabled;

    //! Model for the standard deviation
    Model m_model;

    //! If the damping of the bedrock is enabled
    bool m_bedrockIsEnabled;

    //! Correlation between shear modulus and damping
    double m_correl;

    //! Uncertainty model for the shear modulus reduction
    NonlinearPropertyUncertainty* m_modulusUncert;

    //! Uncertainty model for the damping ratio
    NonlinearPropertyUncertainty* m_dampingUncert;

    //! Random number generator
    gsl_rng * m_rng;

    //! Site response model
    SoilProfile* m_siteProfile;
};
#endif
