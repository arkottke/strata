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

#include "NonlinearPropertyRandomizer.h"

#include "NonlinearPropertyUncertainty.h"
#include "SoilProfile.h"
#include "SoilType.h"
#include "RockLayer.h"

#include <QDebug>
#include <QObject>
#include <QString>
#include <QStringList>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <cmath>

NonlinearPropertyRandomizer::NonlinearPropertyRandomizer(gsl_rng* rng, SoilProfile* siteProfile)
    : QObject(siteProfile), m_rng(rng), m_siteProfile(siteProfile)
{
    connect(m_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateEnabled()));
    connect(m_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateBedrockIsEnabled()));

    m_modulusUncert = new NonlinearPropertyUncertainty(0.15);
    m_dampingUncert = new NonlinearPropertyUncertainty(0.30);

    m_enabled = false;
    m_bedrockIsEnabled = false;
    m_correl = -0.50;
    setModel(Darendeli);
}

NonlinearPropertyRandomizer::~NonlinearPropertyRandomizer()
{
    m_modulusUncert->deleteLater();
    m_dampingUncert->deleteLater();
}


QStringList NonlinearPropertyRandomizer::modelList()
{
    return QStringList() << tr("SPID") << tr("Darendeli");
}

bool NonlinearPropertyRandomizer::enabled() const
{
    return m_siteProfile->isVaried() && m_enabled;
}

void NonlinearPropertyRandomizer::setEnabled(bool enabled)
{
    if ( m_enabled != enabled ) {
        m_enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();        
    }
}

NonlinearPropertyRandomizer::Model NonlinearPropertyRandomizer::model() const
{
    return m_model;
}

void NonlinearPropertyRandomizer::setModel(int model)
{
    setModel((Model)model);
}

void NonlinearPropertyRandomizer::setModel(Model model)
{
    if ( m_model != model ) {
        m_model = model;
        emit modelChanged(m_model);
        emit wasModified();
    }
}
    
bool NonlinearPropertyRandomizer::bedrockIsEnabled() const
{
    return m_siteProfile->isVaried() && m_bedrockIsEnabled;
}

void NonlinearPropertyRandomizer::setBedrockIsEnabled(bool enabled)
{
    if ( m_bedrockIsEnabled != enabled ) {
        m_bedrockIsEnabled = enabled;

        emit bedrockIsEnabledChanged(bedrockIsEnabled());
        emit wasModified();
    }
}

double NonlinearPropertyRandomizer::correl() const
{
    return m_correl;
}

void NonlinearPropertyRandomizer::setCorrel(double correl)
{
    if ( m_correl != correl ) {
        emit wasModified();
    }

    m_correl = correl;
}

NonlinearPropertyUncertainty* NonlinearPropertyRandomizer::dampingUncert()
{
    return m_dampingUncert;
}

NonlinearPropertyUncertainty* NonlinearPropertyRandomizer::modulusUncert()
{
    return m_modulusUncert;
}

void NonlinearPropertyRandomizer::vary(SoilType* soilType)
{
    // Generate correlated random variables
    double randG;
    double randD;
    gsl_ran_bivariate_gaussian(m_rng, 1.0, 1.0, m_correl, &randG, &randD);

    // Vary the shear modulus
    m_modulusUncert->vary(m_model, soilType->modulusModel(), randG);
    m_dampingUncert->vary(m_model, soilType->dampingModel(), randD);
}

void NonlinearPropertyRandomizer::vary(RockLayer* bedrock)
{
    bedrock->setDamping(
                m_dampingUncert->variedDamping(m_model, bedrock->avgDamping(), gsl_ran_gaussian(m_rng, 1)));
}

void NonlinearPropertyRandomizer::updateEnabled()
{
    emit enabledChanged(enabled());
}

void NonlinearPropertyRandomizer::updateBedrockIsEnabled()
{
    emit bedrockIsEnabledChanged(bedrockIsEnabled());
}

void NonlinearPropertyRandomizer::fromJson(const QJsonObject &json)
{
    m_enabled = json["enabled"].toBool();
    m_model = (NonlinearPropertyRandomizer::Model) json["model"].toInt();
    m_bedrockIsEnabled = json["bedrockIsEnabled"].toBool();
    m_correl = json["correl"].toDouble();

    m_modulusUncert->fromJson(json["modulusUncert"].toObject());
    m_dampingUncert->fromJson(json["dampingUncert"].toObject());

    setModel(m_model);
}

QJsonObject NonlinearPropertyRandomizer::toJson() const
{
    QJsonObject json;
    json["enabled"] = m_enabled;
    json["model"] = (int) m_model;
    json["bedrockIsEnabled"] = m_bedrockIsEnabled;
    json["correl"] = m_correl;
    json["modulusUncert"] = m_modulusUncert->toJson();
    json["dampingUncert"] = m_dampingUncert->toJson();
    return json;
}

QDataStream& operator<< (QDataStream & out, const NonlinearPropertyRandomizer* npv)
{
    out << (quint8)2;

    out << npv->m_enabled
            << (int)npv->m_model
            << npv->m_bedrockIsEnabled
            << npv->m_correl
            << npv->m_modulusUncert
            << npv->m_dampingUncert;

    return out;
}

QDataStream& operator>> (QDataStream & in, NonlinearPropertyRandomizer* npv)
{
    quint8 ver;
    in >> ver;

    int model;

    in >> npv->m_enabled
            >> model
            >> npv->m_bedrockIsEnabled
            >> npv->m_correl;

    if (ver < 2) {
        // Read information previously stored in the NonlinearPropertyStandardDeviation
        quint8 ui;
        double d;
        QString s;
        for (int i = 0; i < 2; ++i) {
            in >> ui >> d >> d >> s;
        }
    } else {
        in >> npv->m_modulusUncert >> npv->m_dampingUncert;
    }
    npv->m_model = (NonlinearPropertyRandomizer::Model)model;
    return in;
}
