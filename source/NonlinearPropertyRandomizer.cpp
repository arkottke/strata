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

#define _USE_MATH_DEFINES
#include <cmath>

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

NonlinearPropertyRandomizer::NonlinearPropertyRandomizer(gsl_rng* rng, SoilProfile* siteProfile)
    : QObject(siteProfile), _rng(rng), _siteProfile(siteProfile)
{
    connect(_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateEnabled()));
    connect(_siteProfile, SIGNAL(isVariedChanged(bool)),
            this, SLOT(updateBedrockIsEnabled()));

    _modulusUncert = new NonlinearPropertyUncertainty(0.15, 0.001, 1.);
    _dampingUncert = new NonlinearPropertyUncertainty(0.30, 0.001, 20);

    _enabled = false;
    _bedrockIsEnabled = false;
    _correl = -0.50;
    setModel(Darendeli);
}

NonlinearPropertyRandomizer::~NonlinearPropertyRandomizer()
{
    _modulusUncert->deleteLater();
    _dampingUncert->deleteLater();
}


QStringList NonlinearPropertyRandomizer::modelList()
{
    return QStringList() << tr("SPID") << tr("Darendeli");
}

bool NonlinearPropertyRandomizer::enabled() const
{
    return _siteProfile->isVaried() && _enabled;
}

void NonlinearPropertyRandomizer::setEnabled(bool enabled)
{
    if ( _enabled != enabled ) {
        _enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();        
    }
}

NonlinearPropertyRandomizer::Model NonlinearPropertyRandomizer::model() const
{
    return _model;
}

void NonlinearPropertyRandomizer::setModel(int model)
{
    setModel((Model)model);
}

void NonlinearPropertyRandomizer::setModel(Model model)
{
    if ( _model != model ) {
        _model = model;
        emit modelChanged(_model);
        emit wasModified();
    }
}
    
bool NonlinearPropertyRandomizer::bedrockIsEnabled() const
{
    return _siteProfile->isVaried() && _bedrockIsEnabled;
}

void NonlinearPropertyRandomizer::setBedrockIsEnabled(bool enabled)
{
    if ( _bedrockIsEnabled != enabled ) {
        _bedrockIsEnabled = enabled;

        emit bedrockIsEnabledChanged(bedrockIsEnabled());
        emit wasModified();
    }
}

double NonlinearPropertyRandomizer::correl() const
{
    return _correl;
}

void NonlinearPropertyRandomizer::setCorrel(double correl)
{
    if ( _correl != correl ) {
        emit wasModified();
    }

    _correl = correl;
}

NonlinearPropertyUncertainty* NonlinearPropertyRandomizer::dampingUncert()
{
    return _dampingUncert;
}

NonlinearPropertyUncertainty* NonlinearPropertyRandomizer::modulusUncert()
{
    return _modulusUncert;
}

void NonlinearPropertyRandomizer::vary(SoilType* soilType)
{
    // Generate correlated random variables
    double randG;
    double randD;
    gsl_ran_bivariate_gaussian(_rng, 1.0, 1.0, _correl, &randG, &randD);

    // Vary the shear modulus
    _modulusUncert->vary(_model, soilType->modulusModel(), randG);
    _dampingUncert->vary(_model, soilType->dampingModel(), randD);
}

void NonlinearPropertyRandomizer::vary(RockLayer* bedrock)
{
    bedrock->setDamping(
                _dampingUncert->variedDamping(_model, bedrock->avgDamping(), gsl_ran_gaussian(_rng, 1)));
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
    _enabled = json["enabled"].toBool();
    _model = (NonlinearPropertyRandomizer::Model) json["model"].toInt();
    _bedrockIsEnabled = json["bedrockIsEnabled"].toBool();
    _correl = json["correl"].toDouble();

    _modulusUncert->fromJson(json["modulusUncert"].toObject());
    _dampingUncert->fromJson(json["dampingUncert"].toObject());

    setModel(_model);
}

QJsonObject NonlinearPropertyRandomizer::toJson() const
{
    QJsonObject json;
    json["enabled"] = _enabled;
    json["model"] = (int) _model;
    json["bedrockIsEnabled"] = _bedrockIsEnabled;
    json["correl"] = _correl;
    json["modulusUncert"] = _modulusUncert->toJson();
    json["dampingUncert"] = _dampingUncert->toJson();
    return json;
}

QDataStream& operator<< (QDataStream & out, const NonlinearPropertyRandomizer* npv)
{
    out << (quint8)2;

    out << npv->_enabled
            << (int)npv->_model
            << npv->_bedrockIsEnabled
            << npv->_correl
            << npv->_modulusUncert
            << npv->_dampingUncert;

    return out;
}

QDataStream& operator>> (QDataStream & in, NonlinearPropertyRandomizer* npv)
{
    quint8 ver;
    in >> ver;

    int model;

    in >> npv->_enabled
            >> model
            >> npv->_bedrockIsEnabled
            >> npv->_correl;

    if (ver < 2) {
        // Read information previously stored in the NonlinearPropertyStandardDeviation
        quint8 ui;
        double d;
        QString s;
        for (int i = 0; i < 2; ++i) {
            in >> ui >> d >> d >> s;
        }
    } else {
        in >> npv->_modulusUncert >> npv->_dampingUncert;
    }
    npv->_model = (NonlinearPropertyRandomizer::Model)model;
    return in;
}
