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

#include "ProfileVariation.h"

#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <cmath>

#include <QObject>
#include <QDebug>
        
ProfileVariation::ProfileVariation()
{
    reset();
}

QStringList ProfileVariation::velocityModelList()
{
    QStringList list;
    
    list << QObject::tr("Custom")
        << QObject::tr("GeoMatrix A&B")
        << QObject::tr("GeoMatrix C&D")
        << QObject::tr("USGS A&B")
        << QObject::tr("USGS C&D")
        << QObject::tr("USGS A")
        << QObject::tr("USGS B")
        << QObject::tr("USGS C")
        << QObject::tr("USGS D");

    return list;
}

QStringList ProfileVariation::layeringModelList()
{
    QStringList list;
    
    list << QObject::tr("Custom")
        << QObject::tr("Default (Toro 95)");

    return list;
}
        
void ProfileVariation::reset()
{
    m_enabled = true;

    m_isVelocityVaried = true;
    m_isLayeringVaried = false;
    m_isBedrockDepthVaried = false;

    m_stdevIsLayerSpecific = false;

    // Set default models 
    setStdevModel(USGS_C);
    setCorrelModel(USGS_C);
    setLayeringModel(DefaultLayering);
}

void ProfileVariation::setUnits(const Units * units)
{
    m_units = units;
}

bool ProfileVariation::enabled() const
{
    return m_enabled;
}

void ProfileVariation::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool ProfileVariation::isVelocityVaried() const
{
    return m_isVelocityVaried;
}

void ProfileVariation::setVaryVelocity(bool isVelocityVaried)
{
    m_isVelocityVaried = isVelocityVaried;
}

bool ProfileVariation::isLayeringVaried() const
{
    return m_isLayeringVaried;
}

void ProfileVariation::setVaryLayering(bool isLayeringVaried)
{
    m_isLayeringVaried = isLayeringVaried;
}

bool ProfileVariation::isBedrockDepthVaried() const
{
    return m_isBedrockDepthVaried;
}

void ProfileVariation::setVaryBedrockDepth(bool isBedrockDepthVaried)
{
    m_isBedrockDepthVaried = isBedrockDepthVaried;
}

ProfileVariation::VelocityModel ProfileVariation::stdevModel() const
{
    return m_stdevModel;
}

void ProfileVariation::setStdevModel(VelocityModel model)
{
    m_stdevModel = model;

    switch (model)
    {
    case Custom:
        break;
    case GeoMatrix_AB:
        m_stdev = 0.46;
        break;
    case GeoMatrix_CD:
        m_stdev = 0.38;
        break;
    case USGS_AB:
        m_stdev = 0.35;
        break;
    case USGS_CD:
        m_stdev = 0.36;
        break;
    case USGS_A:
        m_stdev = 0.36;
        break;
    case USGS_B:
        m_stdev = 0.27;
        break;
    case USGS_C:
        m_stdev = 0.31;
        break;
    case USGS_D:
        m_stdev = 0.37;
        break;
    }
}

bool ProfileVariation::stdevIsLayerSpecific() const
{
    return m_stdevIsLayerSpecific;
}

void ProfileVariation::setStdevIsLayerSpecific(bool stdevIsLayerSpecific)
{
   m_stdevIsLayerSpecific = stdevIsLayerSpecific;
}

double ProfileVariation::stdev() const
{
    return m_stdev;
}

void ProfileVariation::setStdev(double stdev)
{
    m_stdev = stdev;
}

ProfileVariation::VelocityModel ProfileVariation::correlModel() const
{
    return m_correlModel;
}

void ProfileVariation::setCorrelModel(VelocityModel model)
{
    m_correlModel = model;

    switch (model)
    {
    case Custom:
        break;
    case GeoMatrix_AB:
        m_correlInitial = 0.96;
        m_correlFinal = 0.96;
        m_correlDelta = 13.1;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.095;
        break;
    case GeoMatrix_CD:
        m_correlInitial = 0.99;
        m_correlFinal = 1.00;
        m_correlDelta = 8.0;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.160;
        break;
    case USGS_AB:
        m_correlInitial = 0.95;
        m_correlFinal = 1.00;
        m_correlDelta = 4.2;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.138;
        break;
    case USGS_CD:
        m_correlInitial = 0.99;
        m_correlFinal = 1.00;
        m_correlDelta = 3.9;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.293;
        break;
    case USGS_A:
        m_correlInitial = 0.95;
        m_correlFinal = 0.42;
        m_correlDelta = 3.4;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.063;
        break;
    case USGS_B:
        m_correlInitial = 0.97;
        m_correlFinal = 1.00;
        m_correlDelta = 3.8;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.293;
        break;
    case USGS_C:
        m_correlInitial = 0.99;
        m_correlFinal = 0.98;
        m_correlDelta = 3.9;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.344;
        break;
    case USGS_D:
        m_correlInitial = 0.00;
        m_correlFinal = 0.50;
        m_correlDelta = 5.0;
        m_correlIntercept = 0.0;
        m_correlExponent = 0.744;
        break;
    }
}

double ProfileVariation::correlInitial() const
{
    return m_correlInitial;
}

void ProfileVariation::setCorrelInitial(double correlInitial)
{
    m_correlInitial = correlInitial;
}

double ProfileVariation::correlFinal() const
{
    return m_correlFinal;
}

void ProfileVariation::setCorrelFinal(double correlFinal)
{
    m_correlFinal = correlFinal;
}

double ProfileVariation::correlDelta() const
{
    return m_correlDelta;
}

void ProfileVariation::setCorrelDelta(double correlDelta)
{
    m_correlDelta = correlDelta;
}

double ProfileVariation::correlIntercept() const
{
    return m_correlIntercept;
}

void ProfileVariation::setCorrelIntercept(double correlIntercept)
{
    m_correlIntercept = correlIntercept;
}

double ProfileVariation::correlExponent() const
{
    return m_correlExponent;
}

void ProfileVariation::setCorrelExponent(double correlExponent)
{
    m_correlExponent = correlExponent;
}

ProfileVariation::LayeringModel ProfileVariation::layeringModel() const
{
    return ProfileVariation::m_layeringModel;
}

void ProfileVariation::setLayeringModel(ProfileVariation::LayeringModel model)
{
    m_layeringModel = model;

    // Load the default values
    if ( m_layeringModel == DefaultLayering ) {
        m_layeringCoeff = 1.98;
        m_layeringInitial = 10.86;
        m_layeringExponent = -0.89;
    }
}

double ProfileVariation::layeringCoeff() const
{
    return m_layeringCoeff;
}

void ProfileVariation::setLayeringCoeff(double layeringCoeff)
{
    m_layeringCoeff = layeringCoeff;
}

double ProfileVariation::layeringInitial() const
{
    return m_layeringInitial;
}

void ProfileVariation::setLayeringInitial(double layeringInitial)
{
    m_layeringInitial = layeringInitial;
}

double ProfileVariation::layeringExponent() const
{
    return m_layeringExponent;
}

void ProfileVariation::setLayeringExponent(double layeringExponent)
{
    m_layeringExponent = layeringExponent;
}

Distribution & ProfileVariation::bedrockDepth() 
{
    return m_bedrockDepth;
}

void ProfileVariation::setRandomNumberGenerator(gsl_rng * rng)
{
    m_rng = rng;
    // Use the same random number generator for the bedrock distribution
    m_bedrockDepth.setRandomNumberGenerator(m_rng);
}

QList<double> ProfileVariation::varyLayering(double depthToBedrock) const
{
    // The thickness of the layers
    QList<double> thickness;

    double depth = 0;

    while ( depth < depthToBedrock ) {
        // Compute the depth dependent rate
        double rate = m_layeringCoeff * pow( depth + m_layeringInitial, m_layeringExponent );
        // Using a random probability, the thickness is computing using the inverse CDF.
        thickness << log( 1 - gsl_rng_uniform(m_rng)) / -rate;
        
        depth += thickness.last();
    }

    // Correct the last layer of thickness so that the total depth is equal to the maximum depth
    thickness.last() = thickness.last() - (depth - depthToBedrock);
        
    return thickness;
}
  
void ProfileVariation::varyVelocity( QList<SoilLayer*> & soilLayers, RockLayer * bedrock ) const
{
    // The first layer has no correlation
    double prevRandVar = gsl_ran_gaussian(m_rng, m_stdev);

    soilLayers[0]->setShearVel( soilLayers.first()->avg() * exp(prevRandVar));

    for (int i = 1; i < soilLayers.size(); ++i) {
        // Depth at the middle of the layer
        double depthToMid = soilLayers.at(i)->depth() + soilLayers.at(i)->thickness()/2.;

        // If the English units are used convert the depthToMid to meters
        if ( m_units->system() == Units::English )
            depthToMid *= 0.3048;

        // Depth dependent correlation
        double depthCorrel = 0;

        if ( depthToMid < 200)
            depthCorrel = m_correlFinal * pow( (depthToMid + m_correlInitial) /
                    (200 + m_correlInitial), m_correlExponent);
        else
            depthCorrel = m_correlFinal;

        // Thickness dependent correlation
        double thicknessCorrel = m_correlInitial * exp(-soilLayers.at(i)->thickness() / m_correlDelta );

        // Combine the correlations
        double correl = (1 - depthCorrel) * thicknessCorrel + depthCorrel;

        /* 
         * Compute the random variable taking into account the correlation from
         * the previous layer.  
         */
        double randVar = correl * prevRandVar + gsl_ran_gaussian(m_rng, m_stdev) * sqrt( 1 - correl * correl);
        
        // Vary the layer using the random variable
        soilLayers[i]->setShearVel( soilLayers.at(i)->avg() * exp(randVar));
       
        // Save the previous random variable
        prevRandVar = randVar;
    }

    // Randomize the bedrock layer
    /* The thickness dependent correlation is assumed to be zero because the
     * layer is infinitely thick.  Therefore the correl with the previous layer
     * is based on depth alone.  The depth dependent correlation is assumed to
     * be the final correlation (or correlation at 200m).
     */
    double randVar = m_correlFinal * prevRandVar + gsl_ran_gaussian(m_rng, m_stdev) * sqrt( 1 - m_correlFinal * m_correlFinal );
        
    bedrock->setShearVel( bedrock->avg() * exp(randVar));
}

QMap<QString, QVariant> ProfileVariation::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("enabled", QVariant(m_enabled));
    
    map.insert("isVelocityVaried", m_isVelocityVaried);
    map.insert("isLayeringVaried", m_isLayeringVaried);
    map.insert("isBedrockDepthVaried", m_isBedrockDepthVaried);

    map.insert("stdevModel", m_stdevModel);
    map.insert("stdevIsLayerSpecifc", m_stdevIsLayerSpecific);
    map.insert("stdev", m_stdev);

    map.insert("correlModel", m_correlModel);
    map.insert("correlInitial", m_correlInitial);
    map.insert("correlFinal", m_correlFinal);
    map.insert("correlDelta", m_correlDelta);
    map.insert("correlIntercept", m_correlIntercept);
    map.insert("correlExponent", m_correlExponent);

    map.insert("layeringModel", m_layeringModel);
    map.insert("layeringCoeff", m_layeringCoeff);
    map.insert("layeringInitial", m_layeringInitial);
    map.insert("layeringExponent", m_layeringExponent);
    
    map.insert("bedrockDepth", m_bedrockDepth.toMap());

    return map;
}

void ProfileVariation::fromMap( const QMap<QString, QVariant> & map )
{
	m_enabled = map.value("enabled").toBool();
    
	m_isVelocityVaried = map.value("isVelocityVaried").toBool();
	m_isLayeringVaried = map.value("isLayeringVaried").toBool();
	m_isBedrockDepthVaried = map.value("isBedrockDepthVaried").toBool();

	m_stdevModel = (VelocityModel)map.value("stdevModel").toInt();
	m_stdevIsLayerSpecific = map.value("stdevIsLayerSpecifc").toBool();
	m_stdev = map.value("stdev").toDouble();

	m_correlModel = (VelocityModel)map.value("correlModel").toInt();
	m_correlInitial = map.value("correlInitial").toDouble();
	m_correlFinal = map.value("correlFinal").toDouble();
	m_correlDelta = map.value("correlDelta").toDouble();
	m_correlIntercept = map.value("correlIntercept").toDouble();
	m_correlExponent = map.value("correlExponent").toDouble();

	m_layeringModel = (LayeringModel)map.value("layeringModel").toInt();
	m_layeringCoeff = map.value("layeringCoeff").toDouble();
	m_layeringInitial = map.value("layeringInitial").toDouble();
	m_layeringExponent = map.value("layeringExponent").toDouble();

    m_bedrockDepth.fromMap( map.value("bedrockDepth").toMap() );
}
