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

#include "DampingStandardDeviation.h"
#include "ModulusStandardDeviation.h"
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

    m_modulusStdev = new ModulusStandardDeviation;
    m_dampingStdev = new DampingStandardDeviation;

    m_enabled = false;
    m_bedrockIsEnabled = false;
    m_correl = -0.50;

    m_model = Custom;
    setModel(Darendeli);
}

NonlinearPropertyRandomizer::~NonlinearPropertyRandomizer()
{
    m_modulusStdev->deleteLater();
    m_dampingStdev->deleteLater();
}


QStringList NonlinearPropertyRandomizer::modelList()
{
    return QStringList() << tr("Custom") << tr("Darendeli");
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
        
        m_modulusStdev->setModel(m_model);
        m_dampingStdev->setModel(m_model);

        emit modelChanged(m_model);
        emit customEnabledChanged(customEnabled());
        emit wasModified();
    }
}
    
bool NonlinearPropertyRandomizer::customEnabled() const
{
    return (m_model == Custom);
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

DampingStandardDeviation* NonlinearPropertyRandomizer::dampingStdev()
{
    return m_dampingStdev;
}

ModulusStandardDeviation* NonlinearPropertyRandomizer::modulusStdev()
{
    return m_modulusStdev;
}

void NonlinearPropertyRandomizer::vary(SoilType* soilType)
{
    // Generate correlated random variables
    double randG;
    double randD;
    gsl_ran_bivariate_gaussian(m_rng, 1.0, 1.0, m_correl, &randG, &randD);

    // Vary the shear modulus
    varyProperty(
            m_modulusStdev,
            soilType->modulusModel(),
            randG);

    varyProperty(
            m_dampingStdev,
            soilType->dampingModel(),
            randD);
}

void NonlinearPropertyRandomizer::vary(RockLayer* bedrock)
{
    const double stdev = m_dampingStdev->calculate(m_model,
                                                   0.0001, // FIXME can't actually calculate a strain and shouldn't really be used
                                                   bedrock->avgDamping());

    bedrock->setDamping(m_dampingStdev->limit(gsl_ran_gaussian(m_rng, stdev)));
}

void NonlinearPropertyRandomizer::updateEnabled()
{
    emit enabledChanged(enabled());
}

void NonlinearPropertyRandomizer::updateBedrockIsEnabled()
{
    emit bedrockIsEnabledChanged(bedrockIsEnabled());
}

void NonlinearPropertyRandomizer::varyProperty(
        AbstractNonlinearPropertyStandardDeviation* stdevModel, NonlinearProperty *property, double randVar)
{
    QVector<double> varied(property->average().size());

    for (int i = 0; i < property->average().size(); ++i) {
        const double stdev = stdevModel->calculate(
                m_model,
                property->strain().at(i),
                property->average().at(i));

        varied[i] = stdevModel->limit(property->average().at(i) + stdev * randVar);
    }
    property->setVaried(varied);
}

void NonlinearPropertyRandomizer::ptRead(const ptree &pt)
{
    m_enabled = pt.get<bool>("enabled");
    m_model = (NonlinearPropertyRandomizer::Model) pt.get<int>("model");
    m_bedrockIsEnabled = pt.get<bool>("bedrockIsEnabled");
    m_correl = pt.get<double>("correl");

    ptree modulusStdev = pt.get_child("modulusStdev");
    m_modulusStdev->ptRead(modulusStdev);

    ptree dampingStdev = pt.get_child("dampingStdev");
    m_dampingStdev->ptRead(dampingStdev);

    setModel(m_model);
}

void NonlinearPropertyRandomizer::ptWrite(ptree &pt) const
{
    pt.put("enabled", m_enabled);
    pt.put("model", (int) m_model);
    pt.put("bedrockIsEnabled", m_bedrockIsEnabled);
    pt.put("correl", m_correl);

    ptree modulusStdev;
    m_modulusStdev->ptWrite(modulusStdev);
    pt.put_child("modulusStdev", modulusStdev);

    ptree dampingStdev;
    m_dampingStdev->ptWrite(dampingStdev);
    pt.put_child("dampingStdev", dampingStdev);
}

QDataStream& operator<< (QDataStream & out, const NonlinearPropertyRandomizer* npv)
{
    out << (quint8)1;

    out << npv->m_enabled
            << (int)npv->m_model
            << npv->m_bedrockIsEnabled
            << npv->m_correl
            << npv->m_modulusStdev
            << npv->m_dampingStdev;

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
            >> npv->m_correl
            >> npv->m_modulusStdev
            >> npv->m_dampingStdev;

    npv->m_model = (NonlinearPropertyRandomizer::Model)model;
    return in;
}
