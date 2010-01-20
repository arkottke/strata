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

#include "NonlinearPropertyVariation.h"
#include <QObject>
#include <QDebug>
#include <cmath>
#include <gsl/gsl_randist.h>

NonlinearPropertyVariation::NonlinearPropertyVariation( QObject * parent )
    : QObject(parent)
{
    reset();
}

QStringList NonlinearPropertyVariation::modelList()
{
    QStringList list;

    list << QObject::tr("Darendeli") << QObject::tr("Custom");

    return list;
}

void NonlinearPropertyVariation::reset()
{
    m_enabled = true;
    setModel(Darendeli);

    m_bedrockIsEnabled = false;

    m_correl = -0.50;

    m_dampingMin = 0.20;
    m_dampingMax = 15.00;

    m_shearModMin = 0.10;
    m_shearModMax = 1.00;
}

bool NonlinearPropertyVariation::enabled() const
{
    return m_enabled;
}

void NonlinearPropertyVariation::setEnabled(bool enabled)
{
    if ( m_enabled != enabled ) {
        emit wasModified();
    }

    m_enabled = enabled;
}

NonlinearPropertyVariation::Model NonlinearPropertyVariation::model() const
{
    return m_model;
}

void NonlinearPropertyVariation::setModel(int model)
{
    setModel((Model)model);
}

void NonlinearPropertyVariation::setModel(Model model)
{
    if ( m_model != model ) {
        emit wasModified();
    }

    m_model = model;

    if ( m_model == Darendeli ) {
        m_shearModStdevFunc = "exp(-4.23) + sqrt( 0.25 / exp(3.62) - pow(shearMod - 0.5, 2) / exp(3.62))";
        m_dampingStdevFunc = "exp(-5) + exp(-0.25) * sqrt(damping)";
    }
}

bool NonlinearPropertyVariation::bedrockIsEnabled() const
{
    return m_bedrockIsEnabled;
}

void NonlinearPropertyVariation::setBedrockIsEnabled(bool enabled)
{
    if ( m_bedrockIsEnabled != enabled ) {
        emit wasModified();
    }

    m_bedrockIsEnabled = enabled;
}

double NonlinearPropertyVariation::correl() const
{
    return m_correl;
}

void NonlinearPropertyVariation::setCorrel(double correl)
{
    if ( m_correl != correl ) {
        emit wasModified();
    }

    m_correl = correl;
}

QString NonlinearPropertyVariation::shearModStdev() const
{
    return m_shearModStdevFunc;
}

void NonlinearPropertyVariation::setShearModStdev(QString shearModStdev)
{
    if ( m_shearModStdevFunc != shearModStdev ) {
        emit wasModified();
    }

    m_shearModStdevFunc = shearModStdev;
}

double NonlinearPropertyVariation::shearModMin() const
{
    return m_shearModMin;
}

void NonlinearPropertyVariation::setShearModMin(double min)
{
    if ( m_shearModMin != min ) {
        emit wasModified();
    }

    m_shearModMin = min;
}

double NonlinearPropertyVariation::shearModMax() const
{
    return m_shearModMax;
}

void NonlinearPropertyVariation::setShearModMax(double max)
{
    if ( m_shearModMax != max ) {
        emit wasModified();
    }

    m_shearModMax = max;
}

QString NonlinearPropertyVariation::dampingStdev() const
{
    return m_dampingStdevFunc;
}

void NonlinearPropertyVariation::setDampingStdev(QString dampingStdev)
{
    if ( m_dampingStdevFunc != dampingStdev ) {
        emit wasModified();
    }

    m_dampingStdevFunc = dampingStdev;
}

double NonlinearPropertyVariation::dampingMin() const
{
    return m_dampingMin;
}

void NonlinearPropertyVariation::setDampingMin(double min)
{
    if ( m_dampingMin != min ) {
        emit wasModified();
    }

    m_dampingMin = min;
}

double NonlinearPropertyVariation::dampingMax() const
{
    return m_dampingMax;
}

void NonlinearPropertyVariation::setDampingMax(double max)
{
    if ( m_dampingMax != max ) {
        emit wasModified();
    }

    m_dampingMax = max;
}

void NonlinearPropertyVariation::setRandomNumberGenerator(gsl_rng * rng)
{
    m_rng = rng;
}

QMap<QString, QVariant> NonlinearPropertyVariation::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("enabled", m_enabled);
    map.insert("model", m_model);
    map.insert("bedrockIsEnabled", m_bedrockIsEnabled);
    map.insert("correl", m_correl);

    map.insert("shearModStdev", m_shearModStdevFunc);
    map.insert("shearModMin", m_shearModMin);
    map.insert("shearModMax", m_shearModMax);

    map.insert("dampingStdev", m_dampingStdevFunc);
    map.insert("dampingMin", m_dampingMin);
    map.insert("dampingMax", m_dampingMax);

    return map;
}

void NonlinearPropertyVariation::fromMap( const QMap<QString, QVariant> & map )
{
    m_enabled = map.value("enabled").toBool();
    m_model = (Model)map.value("model").toInt();
    m_bedrockIsEnabled = map.value("bedrockIsEnabled").toBool();
    m_correl = map.value("correl").toDouble();

    m_shearModStdevFunc = map.value("shearModStdev").toString();
    m_shearModMin = map.value("shearModMin").toDouble();
    m_shearModMax = map.value("shearModMax").toDouble();

    m_dampingStdevFunc = map.value("dampingStdev").toString();
    m_dampingMin = map.value("dampingMin").toDouble();
    m_dampingMax = map.value("dampingMax").toDouble();
}

void NonlinearPropertyVariation::vary(SoilType & soilType)
{
    // Generate correlated random variables
    double randG;
    double randD;
    gsl_ran_bivariate_gaussian( m_rng, 1.0, 1.0, m_correl, &randG, &randD);

    // Clear the varied shear modulus
    soilType.normShearMod()->prop().clear();

    // Vary the shear modulus
    for (int i = 0; i < soilType.normShearMod()->avg().size(); ++i) {
        const double stdev = shearModStdev( soilType.normShearMod()->avg().at(i), soilType.normShearMod()->strain().at(i) );
        double shearMod = soilType.normShearMod()->avg().at(i) + stdev * randG;
        // Check if the value is less than the minimum
        if ( shearMod < m_shearModMin )
            shearMod = m_shearModMin;

        // Check if the value is greater than the maximum
        if ( shearMod > m_shearModMax )
            shearMod = m_shearModMax;

        // Create the value
        soilType.normShearMod()->prop() << shearMod;
    }

    // Clear the varied damping
    soilType.damping()->prop().clear();
    // Vary the damping
    for (int i = 0; i < soilType.damping()->avg().size(); ++i) {
        const double stdev =dampingStdev( soilType.damping()->avg().at(i), soilType.damping()->strain().at(i) );
        double damping = soilType.damping()->avg().at(i) + stdev * randD;
        // Check if the value is less than the minimum
        if ( damping < m_dampingMin ) {
            damping = m_dampingMin;
        }
        
        // Check if the value is greater than the maximum
        if ( damping > m_dampingMax ) {
            damping = m_dampingMax;
        }

        // Create the value
        soilType.damping()->prop() << damping;
    }
}

void NonlinearPropertyVariation::vary( RockLayer & bedrock )
{
    bedrock.setDamping( bedrock.avgDamping() + gsl_ran_gaussian( m_rng,
                dampingStdev( bedrock.avgDamping() ) ) );
}

bool NonlinearPropertyVariation::isValid( const QString & function ) const
{
    return m_engine.canEvaluate( function );
}

double NonlinearPropertyVariation::shearModStdev( double normShearMod, double strain )
{
    // The standard deviation model is based on the work by Stokoe and
    // Darendeli.  The standard deviation definition is found on page 175
    // of Darendeli's PhD dissertation and the model parameters are found
    // on page 214.
    double stdev = 0;
    if ( m_model == Darendeli ) {
        // Darendeli Model -- hardcoded
        stdev = exp(-4.23) + sqrt( 0.25 / exp(3.62) -
                pow(normShearMod - 0.5, 2)/exp(3.62));
    } else if ( m_model == Custom ) {
        // Evaluate the custom model with the Qt Script Engine
        m_engine.globalObject().setProperty( "strain", QScriptValue( &m_engine,
                    strain ) );
        m_engine.globalObject().setProperty( "shearMod", QScriptValue( &m_engine, 
                    normShearMod ) );

        if ( m_engine.canEvaluate(m_shearModStdevFunc) ) {
            stdev = m_engine.evaluate(m_shearModStdevFunc).toNumber();
        } else {
            qCritical() << QObject::tr("Shear modulus standard deviation cannot be evaulated!");
            return -1;
        }
    }

    return stdev;
}    

double NonlinearPropertyVariation::dampingStdev( double damping, double strain )
{
    // The standard deviation model is based on the work by Stokoe and
    // Darendeli.  The standard deviation definition is found on page 177
    // of Darendeli's PhD dissertation and the model parameters are found
    // on page 214. 
    double stdev = 0;
    if ( m_model == Darendeli ) {
        // Darendeli Model -- hardcoded
        stdev = exp(-5) + exp(-0.25) * sqrt(damping);
    } else if ( m_model == Custom ) {
        // Evaluate the custom model with the Qt Script Engine
        m_engine.globalObject().setProperty( "strain", QScriptValue( &m_engine, strain));
        m_engine.globalObject().setProperty( "damping", QScriptValue( &m_engine, damping));

        if ( m_engine.canEvaluate(m_dampingStdevFunc) )
            stdev = m_engine.evaluate(m_dampingStdevFunc).toNumber();
        else {
            qCritical() << QObject::tr("Shear modulus standard deviation cannot be evaulated!");
            return -1;
        } 
    }

    return stdev;
}    


