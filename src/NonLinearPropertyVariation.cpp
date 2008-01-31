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

#include "NonLinearPropertyVariation.h"
#include <QObject>
#include <QDebug>
#include <cmath>
#include <gsl/gsl_randist.h>

NonLinearPropertyVariation::NonLinearPropertyVariation( QObject * parent )
    : QObject(parent)
{
    reset();
}

QStringList NonLinearPropertyVariation::modelList()
{
    QStringList list;

    list << QObject::tr("Darendeli") << QObject::tr("Custom");
        
    return list;
}
        
void NonLinearPropertyVariation::reset()
{
    m_enabled = true;
    setModel(Darendeli);

    m_bedrockIsEnabled = false;

    m_correl = -0.50;
    m_dampingMin = 0.20;
    m_shearModMin = 0.10;
    m_shearModMax = 1.00;
}

bool NonLinearPropertyVariation::enabled() const
{
    return m_enabled;
}

void NonLinearPropertyVariation::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
        
NonLinearPropertyVariation::Model NonLinearPropertyVariation::model() const
{
    return m_model;
}

void NonLinearPropertyVariation::setModel(Model model)
{
    m_model = model;

    if ( m_model == Darendeli ) {
        m_shearModStdev = "exp(-4.23) + sqrt( 0.25 / exp(3.62) - pow(shearMod - 0.5, 2) / exp(3.62))";
        m_dampingStdev = "exp(-5) + exp(-0.25) * sqrt(100*damping/100)";
    }
}

bool NonLinearPropertyVariation::bedrockIsEnabled() const
{
    return m_bedrockIsEnabled;
}

void NonLinearPropertyVariation::setBedrockIsEnabled(bool enabled)
{
    m_bedrockIsEnabled = enabled;
}

double NonLinearPropertyVariation::correl() const
{
    return m_correl;
}

void NonLinearPropertyVariation::setCorrel(double correl)
{
    m_correl = correl;
}
        
QString NonLinearPropertyVariation::shearModStdev() const
{
    return m_shearModStdev;
}

void NonLinearPropertyVariation::setShearModStdev(QString shearModStdev)
{
    m_shearModStdev = shearModStdev;
}

double NonLinearPropertyVariation::shearModMin() const
{
    return m_shearModMin;
}

void NonLinearPropertyVariation::setShearModMin(double min)
{
    m_shearModMin = min;
}

double NonLinearPropertyVariation::shearModMax() const
{
    return m_shearModMax;
}

void NonLinearPropertyVariation::setShearModMax(double max)
{
    m_shearModMax = max;
}

QString NonLinearPropertyVariation::dampingStdev() const
{
    return m_dampingStdev;
}

void NonLinearPropertyVariation::setDampingStdev(QString dampingStdev)
{
    m_dampingStdev = dampingStdev;
}

double NonLinearPropertyVariation::dampingMin() const
{
    return m_dampingMin;
}

void NonLinearPropertyVariation::setDampingMin(double min)
{
    m_dampingMin = min;
}

void NonLinearPropertyVariation::setRandomNumberGenerator(gsl_rng * rng)
{
    m_rng = rng;
}

QMap<QString, QVariant> NonLinearPropertyVariation::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("enabled", m_enabled);
    map.insert("model", m_model);
    map.insert("bedrockIsEnabled", m_bedrockIsEnabled);
    map.insert("correl", m_correl);

    map.insert("shearModStdev", m_shearModStdev);
    map.insert("shearModMin", m_shearModMin);
    map.insert("shearModMax", m_shearModMax);
   
    map.insert("dampingStdev", m_dampingStdev);
    map.insert("dampingMin", m_dampingMin);
    
    return map;
}

void NonLinearPropertyVariation::fromMap( const QMap<QString, QVariant> & map )
{
	m_enabled = map.value("enabled").toBool();
    m_model = (Model)map.value("model").toInt();
    m_bedrockIsEnabled = map.value("bedrockIsEnabled").toBool();
    m_correl = map.value("correl").toDouble();

    m_shearModStdev = map.value("shearModStdev").toString();
    m_shearModMin = map.value("shearModMin").toDouble();
    m_shearModMax = map.value("shearModMax").toDouble();

    m_dampingStdev = map.value("dampingStdev").toString();
    m_dampingMin = map.value("dampingMin").toDouble();
}

void NonLinearPropertyVariation::vary(SoilType & soilType)
{
    // Generate correlated random variables
    double randG;
    double randD;
    gsl_ran_bivariate_gaussian( m_rng, 1.0, 1.0, m_correl, &randG, &randD);

    // Clear the varied shear modulus
    soilType.normShearMod().prop().clear();
    
    // Vary the shear modulus
    for (int i = 0; i < soilType.normShearMod().avg().size(); ++i) {
        // The standard deviation model is based on the work by Stokoe and
        // Darendeli.  The standard deviation definition is found on page 175
        // of Darendeli's PhD dissertation and the model parameters are found
        // on page 214.
        double stdev = 0;
        if ( m_model == Darendeli ) {
            // Darendeli Model -- hardcoded
            stdev = exp(-4.23) + sqrt( 0.25 / exp(3.62) -
                    pow(soilType.normShearMod().avg().at(i) - 0.5, 2)/exp(3.62));
        } else if ( m_model == Custom ) {
            // Evaluate the custom model with the Qt Script Engine
            m_engine.globalObject().setProperty( "strain", QScriptValue( &m_engine, 
                        soilType.normShearMod().strain().at(i)));
            m_engine.globalObject().setProperty( "shearMod", QScriptValue( &m_engine, 
                        soilType.normShearMod().avg().at(i)));
          
            if ( m_engine.canEvaluate(m_shearModStdev) ) 
                stdev = m_engine.evaluate(m_shearModStdev).toNumber();
            else {
                qCritical() << QString(tr("Shear modulus standard deviation for '%1' cannot be evaulated!")).arg(soilType.name());
                return;
            }
        }

        double shearMod = soilType.normShearMod().avg().at(i) + stdev * randG;
        // Check if the value is less than the minimum
        if ( shearMod < m_shearModMin )
            shearMod = m_shearModMin;

        // Check if the value is greater than the maximum
        if ( shearMod > m_shearModMax )
            shearMod = m_shearModMax;
            
        // Create the value
        soilType.normShearMod().prop() << shearMod;
    }
    
    // Clear the varied damping
    soilType.damping().prop().clear();
    // Vary the damping
    for (int i = 0; i < soilType.damping().avg().size(); ++i) {
        double stdev = computeDampingStdev( soilType.damping().avg().at(i), soilType.damping().strain().at(i) );
        double damping = soilType.damping().avg().at(i) + stdev * randD;
        // Check if the value is less than the minimum
        if ( damping < m_dampingMin )
            damping = m_dampingMin;
       
        // Create the value
        soilType.damping().prop() << damping;
    }
}

void NonLinearPropertyVariation::vary( RockLayer & bedrock )
{
    bedrock.setDamping( bedrock.avgDamping() + gsl_ran_gaussian( m_rng, 
                computeDampingStdev( bedrock.avgDamping() ) ) );
}

bool NonLinearPropertyVariation::isValid( const QString & function ) const
{
    return m_engine.canEvaluate( function );
}

double NonLinearPropertyVariation::computeDampingStdev( double damping, double strain )
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

        if ( m_engine.canEvaluate(m_dampingStdev) )
            stdev = m_engine.evaluate(m_dampingStdev).toNumber();
        else {
            qCritical() << QObject::tr("Shear modulus standard deviation cannot be evaulated!");
            return -1;
        } 
    }

    return stdev;
}    


