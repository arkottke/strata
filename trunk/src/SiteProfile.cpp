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

#include "SiteProfile.h"
#include "Algorithms.h"
#include <cmath>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

SiteProfile::SiteProfile( QObject * parent)
    : QObject(parent)
{
    m_bedrock = new RockLayer;

    // Allocate the random number generator with the "Mersenne Twister" algorithm
    m_rng = gsl_rng_alloc( gsl_rng_mt19937 );

    // Initialize the seed of the generator using the time
    gsl_rng_set(m_rng, time(0));

    // Set the generator of the dynamicPropertyVariation and the profileVariation
    m_nonLinearPropertyVariation.setRandomNumberGenerator(m_rng);
    m_profileVariation.setRandomNumberGenerator(m_rng);

    // reset the values
    reset();
}

SiteProfile::~SiteProfile()
{
    delete m_bedrock;
    gsl_rng_free(m_rng);
}

void SiteProfile::reset()
{
    m_isSiteVaried = false;
    m_profileCount = 100;

    m_inputDepth = -1;
    
    m_maxFreq = 20;
    m_waveFraction = 0.20;

    m_soilTypes.clear();
    emit soilTypeChanged();
   
    m_soilLayers.clear();
    emit soilLayersChanged();
    
    m_subLayers.clear();

    m_bedrock->reset();

    m_profileVariation.reset();
    m_nonLinearPropertyVariation.reset();
}

int SiteProfile::profileCount() const
{
	return m_profileCount;
}

void SiteProfile::setProfileCount(int count)
{
	m_profileCount = count;
}

bool SiteProfile::isSiteVaried() const
{
	return SiteProfile::m_isSiteVaried;
}

void SiteProfile::setIsSiteVaried(bool isSiteVaried)
{
	m_isSiteVaried = isSiteVaried;
}

QList<SoilType*> & SiteProfile::soilTypes()
{
	return m_soilTypes;
}
		
QList<SoilLayer*> & SiteProfile::soilLayers()
{
	return m_soilLayers;
}

QList<SubLayer> & SiteProfile::subLayers()
{
	return m_subLayers;
}

RockLayer * SiteProfile::bedrock()
{
    return m_bedrock;
}

double SiteProfile::inputDepth() const
{
    return m_inputDepth;
}

void SiteProfile::setInputDepth(double depth)
{
    m_inputDepth = depth;
}
        
const Location & SiteProfile::inputLocation() const
{
    return m_inputLocation;
}

const Location SiteProfile::depthToLocation(const double depth) const
{
    int index = 0;
    double interDepth = 0;

    if ( depth < 0 ) {
        // Use the surface of the bedrock
        index = m_subLayers.size();

        interDepth = 0;   
    } else {
        // Use the layer whose bottom depth is deeper
        index = 0;

        while ( index < m_subLayers.size() && m_subLayers.at(index).depthToBase() < depth )
            ++index;

        interDepth = depth - m_subLayers.at(index).depth();
    }

    return Location( index, interDepth);
}

ProfileVariation & SiteProfile::profileVariation()
{
    return m_profileVariation;
}

NonLinearPropertyVariation & SiteProfile::nonLinearPropertyVariation()
{
    return m_nonLinearPropertyVariation;
}

double SiteProfile::maxFreq() const
{
    return m_maxFreq;
}

void SiteProfile::setMaxFreq(double maxFreq)
{
    m_maxFreq = maxFreq;
}

double SiteProfile::waveFraction() const
{
    return m_waveFraction;
}

void SiteProfile::setWaveFraction(double waveFraction)
{
    m_waveFraction = waveFraction;
}

QStringList SiteProfile::soilTypeNameList() const
{
    QStringList list;

    for (int i = 0; i < m_soilTypes.size(); ++i)
        list << m_soilTypes.at(i)->toString();

    return list;
}

QStringList SiteProfile::soilLayerNameList() const
{
    QStringList list;

    for (int i = 0; i < m_soilLayers.size(); ++i)
        list << QString("%1 %2").arg(i+1).arg(m_soilLayers.at(i)->toString());
    
    return list;
}

void SiteProfile::createSubLayers( TextLog & textLog )
{
    // Clear the previously generated subLayers
    if ( !m_subLayers.isEmpty() && m_profileVariation.isLayeringVaried() ) {
        QList<SoilLayer*> soilLayers;

        // Create a list of unique soilLayers
        for ( int i = 0; i < m_subLayers.size(); ++i)
            if ( !soilLayers.contains( m_subLayers[i].soilLayer() ) )
                soilLayers << m_subLayers[i].soilLayer();

        // Delete the sublayers that were created
        for ( int i = 0; i < soilLayers.size(); ++i)
            delete soilLayers.takeFirst();
    }
    m_subLayers.clear();

    // Vary the non-linear properties of the SoilTypes
    if ( m_isSiteVaried && m_nonLinearPropertyVariation.enabled() && textLog.level() > TextLog::Low ) 
        textLog << QObject::tr("Varying dynamic properties of soil types");

    for( int i = 0; i < m_soilTypes.size(); ++i) {
        if ( m_isSiteVaried && m_nonLinearPropertyVariation.enabled() && m_soilTypes.at(i)->isVaried()) {

            if ( textLog.level() > TextLog::Low )
                textLog << QString(QObject::tr("\t%1")).arg(m_soilTypes.at(i)->name());

            m_nonLinearPropertyVariation.vary(*m_soilTypes[i]);
        } else if (m_soilTypes.at(i)->normShearMod().prop().isEmpty() ||
                m_soilTypes.at(i)->damping().prop().isEmpty()) {
            // Use average as property if it has not been copied over already
            m_soilTypes[i]->normShearMod().prop() = m_soilTypes.at(i)->normShearMod().avg();
            m_soilTypes[i]->damping().prop() = m_soilTypes.at(i)->damping().avg();
        }
    }

    // Vary the damping of the bedrock
    if (  m_isSiteVaried && m_nonLinearPropertyVariation.enabled() && m_nonLinearPropertyVariation.bedrockIsEnabled() ) {
        if ( textLog.level() > TextLog::Low )
            textLog << QString(QObject::tr("Varying damping of bedrock"));

        m_nonLinearPropertyVariation.vary(*m_bedrock);
    }
    
    // Vary the depth to the bedrock
    double depthToBedrock;

    if ( m_isSiteVaried && m_profileVariation.isBedrockDepthVaried() ) {
        if ( textLog.level() > TextLog::Low )
            textLog << QString(QObject::tr("Varying depth to bedrock"));

        m_profileVariation.bedrockDepth().setAvg(m_bedrock->depth());

        depthToBedrock = m_profileVariation.bedrockDepth().rand();
    } else
        depthToBedrock = m_bedrock->depth();

    // Vary the layering 
    QList<SoilLayer*> soilLayers;

    if ( m_isSiteVaried && m_profileVariation.isLayeringVaried()) {
        if ( textLog.level() > TextLog::Low )
            textLog << QString(QObject::tr("Varying the layering"));

        // Compute the depths for each of the layers
        updateDepths();

        // Randomize the layer thicknesses
        QList<double> thickness = m_profileVariation.varyLayering(depthToBedrock);

        // For each thickness, determine the representative soil layer
        double depth = 0;
        for (int i = 0; i < thickness.size(); ++i ) {
            soilLayers << new SoilLayer( *representativeSoilLayer(depth,depth+thickness.at(i)) );
            // Set the new depth and thickness
            soilLayers.last()->setDepth(depth);
            soilLayers.last()->setThickness(thickness.at(i));

            // Increment the depth
            depth += thickness.at(i);
        }
    } else
        // Use thickness of the SoilLayers -- so just copy the list over
        soilLayers = m_soilLayers;

    // Vary the shear-wave velocity
    if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() )
        m_profileVariation.varyVelocity(soilLayers, m_bedrock);

    /* Create the SubLayers by dividing the thicknesses.  The height of the
     * sublayer is determined by computing the shortest wavelength of interest
     * (Vs/freqMax) and then reducing this by the wavelength fraction.
     */
    double depth = 0;
    double vTotalStress = 0;
    for (int i = 0; i < soilLayers.size(); ++i ) {
        // Compute the optimal thickness of the sublayers
        double optSubThickness = soilLayers.at(i)->shearVel() / m_maxFreq * m_waveFraction;
        // Compute the required number of sub-layers for this thickness
        int numSubLayers = int(ceil( soilLayers.at(i)->thickness() / optSubThickness ));
        // The subThickness for an even number of layers
        double subThickness = soilLayers.at(i)->thickness() / numSubLayers; 
        
        for (int j = 0; j < numSubLayers; ++j) {
            m_subLayers << SubLayer( subThickness, depth, vTotalStress, soilLayers.at(i) );
            // Increment the depth by the subThicknees
            depth += subThickness;
            // Compute the stress at the base of the layer and this to the total stress
            vTotalStress += subThickness * soilLayers.at(i)->untWt();
        }
    }

    // Compute the SubLayer index associated with the input depth
    m_inputLocation = depthToLocation(m_inputDepth);
}

void SiteProfile::resetSubLayers()
{
    for ( int i = 0; i < m_subLayers.size(); ++i )
        m_subLayers[i].reset();
}

int SiteProfile::subLayerCount() const
{
	return m_subLayers.size();
}

double SiteProfile::untWt( int layer ) const
{
	if ( layer < m_subLayers.size() )
		return m_subLayers.at(layer).untWt();
	else
		return m_bedrock->untWt();
}

double SiteProfile::density( int layer ) const
{
	if ( layer < m_subLayers.size() )
		return m_subLayers.at(layer).density();
	else
		return m_bedrock->density();
}

double SiteProfile::shearVel( int layer ) const
{
	if ( layer < m_subLayers.size() )
		return m_subLayers.at(layer).shearVel();
	else
		return m_bedrock->shearVel();
}

double SiteProfile::shearMod( int layer) const
{
	if ( layer < m_subLayers.size() )
		return m_subLayers.at(layer).shearMod();
	else
		return m_bedrock->shearMod();
}

double SiteProfile::damping( int layer ) const
{
	if ( layer < m_subLayers.size() )
		return m_subLayers.at(layer).damping();
	else
		return m_bedrock->damping();
}

QVector<double> SiteProfile::depthProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).depth();

    profile << m_bedrock->depth();

    return profile;
}

QVector<double> SiteProfile::depthToMidProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).depthToMid();

    return profile;
}
        
QVector<double> SiteProfile::shearVelProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).shearVel();

    profile << m_bedrock->shearVel();

    return profile;
}

QVector<double> SiteProfile::shearModProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).shearMod();


    return profile;
}

QVector<double> SiteProfile::dampingProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).damping();


    return profile;
}

QVector<double> SiteProfile::vTotalStressProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).vTotalStress();


    return profile;
}

QVector<double> SiteProfile::maxErrorProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).error();

    return profile;
}

QVector<double> SiteProfile::maxShearStrainProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).maxStrain();

    return profile;
}

QVector<double> SiteProfile::shearStressProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).shearStress();


    return profile;
}

QVector<double> SiteProfile::stressRatioProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).stressRatio();


    return profile;
}

const Units * SiteProfile::units() const
{
    return m_units;
}
        
void SiteProfile::setUnits( const Units * units)
{
    m_units = units;
    m_bedrock->setUnits(m_units);
    m_profileVariation.setUnits(m_units);
}
        
void SiteProfile::setThicknessAt( int layer, double thickness )
{
    if ( layer < m_soilLayers.size() )
        m_soilLayers[layer]->setThickness(thickness);
    
    // Compute the new depths
    updateDepths();
}

QMap<QString, QVariant> SiteProfile::toMap() const
{
	QMap<QString, QVariant> map;
    // Used to store the layers before saving them to a map
    QList<QVariant> list;
    // Soil types
    list.clear();
	for ( int i = 0; i < m_soilTypes.size(); ++i)
		list << QVariant(m_soilTypes.at(i)->toMap());
    
    map.insert("soilTypes", list);
    
    // Soil Layers
    list.clear();
	for ( int i = 0; i < m_soilLayers.size(); ++i) {
        QMap<QString, QVariant> soilLayerMap = m_soilLayers.at(i)->toMap();

        // Add the SoilType
        if (m_soilLayers.at(i)->soilType())
            //Defined soil type
            soilLayerMap.insert("soilTypeIndex", QVariant( m_soilTypes.indexOf(
                            const_cast<SoilType*>(m_soilLayers.at(i)->soilType()))));
        else
            soilLayerMap.insert("soilTypeIndex", QVariant(-1));
    
        // Add the map to the list
		list << QVariant(soilLayerMap);
	}
    
    map.insert("soilLayers", list);

    // Bedrock layer
    QMap<QString, QVariant> bedrockMap = m_bedrock->toMap();
    // Add the velocity layer index
    map.insert("bedrock", bedrockMap);
    
    // Variation
    map.insert("profileVariation", m_profileVariation.toMap());
    map.insert("dynamicPropertyVariation", m_nonLinearPropertyVariation.toMap());
    
    // Individual variables
    map.insert("inputDepth",m_inputDepth);
    map.insert("isSiteVaried",m_isSiteVaried);
	map.insert("profileCount",m_profileCount);
    map.insert("maxFreq", m_maxFreq);
    map.insert("waveFraction", m_waveFraction);
    
	return map;
}

void SiteProfile::fromMap(const QMap<QString, QVariant> & map)
{
    // List to hold the different lists loaded from the map
    QList<QVariant> list;
	
    // Soil Types
    m_soilTypes.clear();
	list = map.value("soilTypes").toList();
	for (int i = 0; i < list.size(); ++i) {
        // Values are stored last first
		m_soilTypes << new SoilType(m_units);
		m_soilTypes.last()->fromMap(list.at(i).toMap());
	}

    emit soilTypeChanged();

    // Velocity Layers
    m_soilLayers.clear();
	list = map.value("soilLayers").toList();
	for (int i = 0; i < list.size(); ++i) {
        QMap<QString, QVariant> soilLayerMap = list.at(i).toMap();

        // Values are stored last first
		m_soilLayers << new SoilLayer;
		m_soilLayers.last()->fromMap(soilLayerMap);

        // Set the soil type
        int soilTypeIndex = soilLayerMap.value("soilTypeIndex").toInt();

        if (soilTypeIndex < 0)
            // Undefined
            m_soilLayers.last()->setSoilType(0);
        else
            m_soilLayers.last()->setSoilType(m_soilTypes.at(soilTypeIndex));
	}

    emit soilLayersChanged();

    // Bedrock Layer
    QMap<QString, QVariant> bedrockMap = map.value("bedrock").toMap();
    m_bedrock->fromMap(bedrockMap);
   
    // Variation
    m_nonLinearPropertyVariation.fromMap(map.value("dynamicPropertyVariation").toMap());
    m_profileVariation.fromMap( map.value("profileVariation").toMap() );

    // Inividual variables
    m_inputDepth = map.value("inputDepth").toDouble();
    m_isSiteVaried = map.value("isSiteVaried").toBool();
    m_profileCount = map.value("profileCount").toInt();
    m_maxFreq = map.value("maxFreq").toDouble();
    m_waveFraction = map.value("waveFraction").toDouble();
}

QString SiteProfile::subLayerTable() const
{
    QString html;
    
    html +=  QObject::tr(
                "<table >"
		            "<tr>"
		            	"<th colspan=\"6\"</th>"
		            	"<th colspan=\"3\">Damping (%)</th>"
		            	"<th colspan=\"4\">Shear Modulus</th>"
		            "</tr>"
		            "<tr>"
		            	"<th>No.</th>"
		            	"<th>Soil Type</th>"
		            	"<th>Depth</th>"
		            	"<th>Thickness</th>"
		            	"<th>Max Strain (%)</th>"
                        "<th>Eff. Strain (%)</th>"
		            	"<th>New</th>"
		            	"<th>Old</th>"
		            	"<th>Error (%)</th>"
		            	"<th>New</th>"
		            	"<th>Old</th>"
		            	"<th>Error (%)</th>"
		            	"<th>Norm.</th>"
		            "</tr>");
   
    // Create each of the rows
    for (int i = 0; i < m_subLayers.size(); ++i)
        html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13</tr>")
            .arg(i+1)
            .arg(m_subLayers.at(i).soilTypeName())
            .arg(m_subLayers.at(i).depth(), 0, 'f', 2)
            .arg(m_subLayers.at(i).thickness(), 0, 'f', 2)
            .arg(m_subLayers.at(i).maxStrain(), 0, 'e', 2)
            .arg(m_subLayers.at(i).effStrain(), 0, 'e', 2)
            .arg(m_subLayers.at(i).damping(), 0, 'f', 2)
            .arg(m_subLayers.at(i).oldDamping(), 0, 'f', 2)
            .arg(m_subLayers.at(i).dampingError(), 0, 'f', 2)
            .arg(m_subLayers.at(i).shearMod(), 0, 'f', 0)
            .arg(m_subLayers.at(i).oldShearMod(), 0, 'f', 0)
            .arg(m_subLayers.at(i).shearModError(), 0, 'f', 2)
            .arg(m_subLayers.at(i).normShearMod(), 0, 'f', 3);

    // Bedrock layer
    html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13<td>%14</tr>")
        .arg(m_subLayers.size())
        .arg(QObject::tr("Bedrock"))
        .arg(m_bedrock->depth(), 0, 'f', 2)
        .arg("--")
        .arg("--")
        .arg("--")
        .arg("--")
        .arg(m_bedrock->damping(), 0, 'f', 2)
        .arg("--")
        .arg("--")
        .arg(m_bedrock->shearMod(), 0, 'f', 0)
        .arg("--")
        .arg("--")
        .arg("--");

    // Complete the table
    html += "</table></p>";
   
    return html;
}

QString SiteProfile::toHtml() const
{
    // Requires that the HTML header is already established.
    QString html;

    // 
    // Soil Types
    //
    html += tr("<li><a name=\"soil-types\">Soil Types</a>");
  
    // Create hyper links for soil types
    /*
    html += "<ol>";
    for ( int i = 0; i < m_soilTypes.size(); ++i)
        html += QString("<li><a href=\"#%1\">%1</a></li>").arg(m_soilTypes.at(i)->name());

    html += "<li><a href=\"#Bedrock\">Bedrock<a></li></ol></li>";
    */

    // Generate the output for each soil type
    html += "<ol>";
    for ( int i = 0; i < m_soilTypes.size(); ++i)
        html += m_soilTypes.at(i)->toHtml();

    html +=  QString(tr(
                "<li><a name=\"Bedrock\">Bedrock<a>"
                "<table border=\"0\">"
                "<tr><td><strong>Unit weight:</strong></td><td>%1 %2</td></tr>"
                "<tr><td><strong>Damping:</strong></td><td>%3</td></tr>"))
        .arg(m_bedrock->untWt())
        .arg(m_units->untWt())
        .arg(m_bedrock->damping());


    if ( m_isSiteVaried )
        html += QString("<tr><td><strong>Varied:</strong></td><td>%1</td></tr>").arg(
            boolToString(m_bedrock->isVaried()));
         

    html += "</table></li></ol>";

    //
    // Soil Layers
    //
    html += tr("<li><a name=\"soil-layers\">Soil Layers</a>");

    // Table header
    html += QString(tr("<table border = \"1\">"
            "<tr>"
            "<th>Depth (%1)</th>"
            "<th>Thickness (%1)</th>"
            "<th>Soil Type</th>"
            "<th>Average Vs (%2)</th>"
            ))
        .arg(m_units->length())
        .arg(m_units->vel());

    if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() && m_profileVariation.stdevIsLayerSpecific() )
        html += tr("<th>Stdev.</th>");

    if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() )
        html += QString(tr(
                    "<th>Minimum Vs. (%1)</th>"
                    "<th>Maximum Vs. (%1)</th>"
                    "<th>Varied</th>"
                    )).arg(m_units->vel());

    html += "</tr>";

    // Table information
    for (int i = 0; i < m_soilLayers.size(); ++i ) {
        html += QString("<tr><td>%1</td><td>%2</td><td><a href=\"#%3\">%3</a></td><td>%4</td>")
            .arg(m_soilLayers.at(i)->depth())
            .arg(m_soilLayers.at(i)->thickness())
            .arg(m_soilLayers.at(i)->soilType()->name())
            .arg(m_soilLayers.at(i)->avg());
    
        if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() && m_profileVariation.stdevIsLayerSpecific() )
            html += QString("<td>%1</td>").arg(m_soilLayers.at(i)->stdev());

        if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() )
            html += QString("<td>%1</td><td>%2</td><td>%3</td>")
                .arg(m_soilLayers.at(i)->min())
                .arg(m_soilLayers.at(i)->max())
                .arg(boolToString(m_soilLayers.at(i)->isVaried()));

                html += "</tr>";
    }

    // Bedrock layer
    html += QString("<tr><td>%1</td><td>---</td><td><a href=\"#Bedrock\">Bedrock</a></td><td>%2</td>")
        .arg(m_bedrock->depth())
        .arg(m_bedrock->avg());

    if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() && m_profileVariation.stdevIsLayerSpecific() )
        html += QString("<td>%1</td>").arg(m_bedrock->stdev());

    if ( m_isSiteVaried && m_profileVariation.isVelocityVaried() )
        html += QString("<td>%1</td><td>%2</td><td>%3</td>")
            .arg(m_bedrock->min())
            .arg(m_bedrock->max())
            .arg(boolToString(m_bedrock->isVaried()));

    html += "</tr>";


    html += "</table>";
    return html;
}

void SiteProfile::updateDepths()
{
    m_soilLayers[0]->setDepth(0);

    // Set the depth for the lowers below the one that changed
    for (int i = 0; i < m_soilLayers.size()-1; ++i)
       m_soilLayers[i+1]->setDepth(m_soilLayers.at(i)->depth() + m_soilLayers.at(i)->thickness()); 

    // Update the depth of the rock layer
    m_bedrock->setDepth(m_soilLayers.last()->depth() + m_soilLayers.last()->thickness());    

    // Signal that the depths are updated
    emit depthsChanged();
}

SoilLayer * SiteProfile::representativeSoilLayer( double top, double base)
{
    SoilLayer * selectedLayer = 0;
    double longestTime = 0;

    // If the layer is deeper than the site profile, use the deepest layer
    if ( top > m_soilLayers.last()->depthToBase() )
        return m_soilLayers.last();

    for (int i = 0; i < m_soilLayers.size(); ++i )
    {
        // Skip the layer if it isn't in the depth range of interest
        if ( m_soilLayers.at(i)->depthToBase() < top 
                || m_soilLayers.at(i)->depth() > base )
            continue;
        
        // If the layer is completely within a given layer, return that layer
        if ( m_soilLayers.at(i)->depth() <= top 
                &&  base <= m_soilLayers.at(i)->depthToBase()) {
            selectedLayer = m_soilLayers[i];
            break;
        }

        // The representative layer is the layer with the most travel time
        // through it

        // Path length within the depth interest range
        double length = 0;

        if ( m_soilLayers.at(i)->depth() > top 
                && base < m_soilLayers.at(i)->depthToBase())
            length =  base - m_soilLayers.at(i)->depth();
        else if ( m_soilLayers.at(i)->depth() < top 
                && base > m_soilLayers.at(i)->depthToBase())
            length = m_soilLayers.at(i)->depthToBase() - top;
        else 
            length = base - top;

        // Compute the travel time
        double time = length / m_soilLayers.at(i)->shearVel();

        if ( !selectedLayer || time > longestTime ) {
            selectedLayer = m_soilLayers[i];
            longestTime = time;
        }
    }
    
    return selectedLayer;
}

