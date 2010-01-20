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
      
#include "PointSourceModel.h"
#include "Serializer.h"
#include "Dimension.h"

#include <QDebug>

#include <gsl/gsl_interp.h>

#include <cmath>


PointSourceModel::PointSourceModel(QObject * parent)
    : QObject(parent)
{
    m_modified = true;
    m_distance = 20;
    m_depth = 8;
    m_siteSpecificCrustalAmp = false;
    m_crustAmpNeedsUpdate = true;
    setMomentMag(6.5);
    setLocation(WUS);
}

PointSourceModel::Location PointSourceModel::location() const
{
    return m_location;
}
        
QStringList PointSourceModel::locationList()
{
    QStringList list;

    list << tr("Custom") << tr("Western NA") << tr("Eastern NA");

    return list;
}

void PointSourceModel::setLocation(Location location)
{
    m_location = location;

    switch (m_location)
    {
        case Custom:
            // Do nothing
            break;
        case WUS:
            m_stressDrop = 100;
            m_pathAttenCoeff = 180;
            m_pathAttenPower = 0.45;
            m_shearVelocity = 3.5;
            m_density = 2.8;
            m_siteAtten = 0.04;

            // The final pair (100 Hz, 4.40) in this site amplication is estimated from extrapolation of the data.
            m_freq.clear();
            m_freq << 0.01 << 0.09 << 0.16 << 0.51 << 0.84 << 1.25 << 2.26 << 3.17 << 6.05 << 16.60 << 61.20 << 100.00;

            m_crustAmp.clear();
            m_crustAmp << 1.00 << 1.10 << 1.18 << 1.42 << 1.58 << 1.74 << 2.06 << 2.25 << 2.58 << 3.13 << 4.00 << 4.40;
            
            emit crustalAmpChanged();

            calcCornerFreq();
            calcGeoAtten();

            break;
        case CEUS:
            m_stressDrop = 150;
            m_pathAttenCoeff = 680;
            m_pathAttenPower = 0.36;
            m_shearVelocity = 3.6;
            m_density = 2.8;
            m_siteAtten = 0.006;

            m_freq.clear();
            m_freq << 0.01 << 0.10 << 0.20 << 0.30 << 0.50 << 0.90 << 1.25 << 1.80 << 3.00 << 5.30 << 8.00 << 14.00 << 30.00 << 60.00 << 100.00;

            m_crustAmp.clear();
            m_crustAmp << 1.00 << 1.02 << 1.03 << 1.05 << 1.07 << 1.09 << 1.11 << 1.12 << 1.13 << 1.14 << 1.15 << 1.15 << 1.15 << 1.15 << 1.15;

            emit crustalAmpChanged();

            calcCornerFreq();
            calcGeoAtten();
            break;
    }
    m_modified = true;
}

QString PointSourceModel::toHtml()
{
    QString html = QString(tr(
                "<tr><td><strong>Location:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Moment Magnitude:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Distance:</strong></td><td>%3 km</td></tr>"
                "<tr><td><strong>Depth:</strong></td><td>%4 km </td></tr>"
                "<tr><td><strong>Stress Drop:</strong></td><td>%5 bars</td></tr>"
                "<tr><td><strong>Geometric Attenuation:</strong></td><td>%6</td></tr>"
                "<tr><td><strong>Path Atten Coeff.:</strong></td><td>%7</td></tr>"
                "<tr><td><strong>Path Atten Power.:</strong></td><td>%8</td></tr>"
                "<tr><td><strong>Shear-wave velocity:</strong></td><td>%9 km/s</td></tr>"
                "<tr><td><strong>Density:</strong></td><td>%10 gm/cm%11</td></tr>"
                "<tr><td><strong>Site Attenuation (kappa):</strong></td><td>%12</td></tr>"
                "<tr><td><strong>Generic Crustal Amplication:</strong></td><td>%13</td></tr>"
                "</table>"
               ))
        .arg(locationList().at(m_location))
        .arg(m_momentMag)
        .arg(m_distance)
        .arg(m_depth)
        .arg(m_stressDrop)
        .arg(m_geoAtten)
        .arg(m_pathAttenCoeff)
        .arg(m_pathAttenPower)
        .arg(m_shearVelocity)
        .arg(m_density).arg(QChar(0x00B3))
        .arg(m_siteAtten)
        .arg(m_siteSpecificCrustalAmp ? tr("yes") : tr("no"));

    html += "<table><tr>";
    
    if (m_siteSpecificCrustalAmp) {
        // Add the velocity profile
        html += "<td><h4>Velocity Profile</h4><table border = \"1\">";
        html += QString("<tr><th>Thickness (km)</th><th>Shear Velocity (km/s)</th><th>Density (gm/cm%1)</th></tr>").arg(QChar(0x00B3));

        for (int i = 0; i < m_crustThickness.size(); ++i) {
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
                .arg(m_crustThickness.at(i))
                .arg(m_crustVelocity.at(i))
                .arg(m_crustDensity.at(i));
        }

        html += "</table></td>";

        // Add the crustal amplification 
        if (m_crustAmpNeedsUpdate) {
            calcCrustalAmp();
        }
    }
    
    html += "<td><h4>Crustal Amplification</h4><table border = \"1\">";
    html += "<tr><th>Frequency (Hz)</th><th>Amplification</th></tr>";

    for (int i = 0; i < m_freq.size(); ++i) {
        html += QString("<tr><td>%1</td><td>%2</td></tr>")
            .arg(m_freq.at(i))
            .arg(m_crustAmp.at(i));
    }
    html += "</table></td></tr></table>";

    return html;
}

double PointSourceModel::momentMag() const
{
	return m_momentMag;
}

void PointSourceModel::setMomentMag(double momentMag)
{
	m_momentMag = momentMag;

    // Compute seismic moment based on the moment magnitude
    m_seismicMoment = pow(10, 1.5 * (m_momentMag + 10.7));

    calcCornerFreq();
    m_modified = true;
}

double PointSourceModel::distance() const
{
	return m_distance;
}

void PointSourceModel::setDistance(double distance)
{
	m_distance = distance;
    
    calcHypoDistance();
}

double PointSourceModel::depth() const
{
	return m_depth;
}

void PointSourceModel::setDepth(double depth)
{
	m_depth = depth;

    calcHypoDistance();
}

double PointSourceModel::stressDrop() const
{
	return m_stressDrop;
}

void PointSourceModel::setStressDrop(double stressDrop)
{
	m_stressDrop = stressDrop;

    calcCornerFreq();
    m_modified = true;
}

double PointSourceModel::pathDurCoeff() const
{
    return m_pathDurCoeff;
}

void PointSourceModel::setPathDurCoeff(double pathDurCoeff)
{
	m_pathDurCoeff = pathDurCoeff;
    calcDuration();
    m_modified = true;
}

double PointSourceModel::geoAtten() const
{
	return m_geoAtten;
}

void PointSourceModel::setGeoAtten(double geoAtten)
{
	m_geoAtten = geoAtten;
    m_modified = true;
}

double PointSourceModel::pathAttenCoeff() const
{
	return m_pathAttenCoeff;
}

void PointSourceModel::setPathAttenCoeff(double pathAttenCoeff)
{
	m_pathAttenCoeff = pathAttenCoeff;
    m_modified = true;
}

double PointSourceModel::pathAttenPower() const
{
	return m_pathAttenPower;
}

void PointSourceModel::setPathAttenPower(double pathAttenPower)
{
	m_pathAttenPower = pathAttenPower;
    m_modified = true;
}

double PointSourceModel::shearVelocity() const
{
	return m_shearVelocity;
}

void PointSourceModel::setShearVelocity(double shearVelocity)
{
	m_shearVelocity = shearVelocity;
    calcCornerFreq();
    m_crustAmpNeedsUpdate = true;
    m_modified = true;
}
        
double PointSourceModel::density() const
{
	return m_density;
}

void PointSourceModel::setDensity(double density)
{
	m_density = density;
    m_crustAmpNeedsUpdate = true;
    m_modified = true;
}

double PointSourceModel::siteAtten() const
{
	return m_siteAtten;
}
        
double PointSourceModel::duration() const
{
    return m_duration;
}

bool PointSourceModel::siteSpecificCrustalAmp() const
{
    return m_siteSpecificCrustalAmp;
}

void PointSourceModel::setSiteSpecificCrustalAmp(bool siteSpecificCrustalAmp)
{
    m_siteSpecificCrustalAmp = siteSpecificCrustalAmp;
}

void PointSourceModel::setCrustAmpNeedsUpdate()
{
    m_crustAmpNeedsUpdate = true;
}

QVector<double> & PointSourceModel::freq()
{
    return m_freq;
}

QVector<double> & PointSourceModel::crustAmp()
{
    return m_crustAmp;
}

QVector<double> & PointSourceModel::crustThickness()
{
    return m_crustThickness;
}

QVector<double> & PointSourceModel::crustVelocity()
{
    return m_crustVelocity;
}

QVector<double> & PointSourceModel::crustDensity()
{
    return m_crustDensity;
}

bool PointSourceModel::modified() const
{
    return m_modified;
}

void PointSourceModel::setSiteAttenuation(double siteAttenuation)
{
	m_siteAtten = siteAttenuation;
    m_modified = true;
}

void PointSourceModel::calcHypoDistance()
{
    m_hypoDistance = sqrt(m_depth * m_depth + m_distance * m_distance);

    calcDuration();
    calcGeoAtten();
    m_modified = true;
}

void PointSourceModel::calcCornerFreq()
{
    if (m_shearVelocity > 0 && m_stressDrop > 0 && m_seismicMoment > 0) {
        m_cornerFreq = 4.9e6 * m_shearVelocity * pow(m_stressDrop/m_seismicMoment, 1./3.);
        calcDuration();
    }
    
    return;
}

void PointSourceModel::calcDuration()
{
    const double sourceDur = 1 / m_cornerFreq;

    // Offset distance
    double offsetDist = 0;
    // Duration at offset
    double offsetDur = 0;

    switch (m_location)
    {
        case Custom:
            // Do nothing
            break;
        // Values proposed by Campbell 2003
        case WUS:
            m_pathDurCoeff = 0.05;
            break;
        case CEUS:
            // The duration is assumed to be piece-wise linear with the equal
            // values at the intersections.
            if (m_hypoDistance <= 10.) {
                m_pathDurCoeff = 0;

                offsetDur = 0;
                offsetDist = 0;
            } else if (m_hypoDistance <= 70.) {
                // slope of 0.16 during this segment
                m_pathDurCoeff = 0.16;
                
                offsetDur = 0;
                offsetDist = 10.;
            } else if (m_hypoDistance <= 130.) {
                m_pathDurCoeff = -0.03;
                
                offsetDur = 0.16 * (70.-10.);
                offsetDist = 70.;
            } else {
                m_pathDurCoeff = 0.04;
                
                offsetDur = 0.16 * (70.-10.) - 0.03 * (130.-70.);
                offsetDist = 130.;
            }
            break;
    }

    double pathDur = offsetDur + m_pathDurCoeff * (m_hypoDistance - offsetDist);
   
    m_duration = sourceDur + pathDur;
    
    emit pathDurCoeffChanged(m_pathDurCoeff);
    emit durationChanged(m_duration);
    return;
}

void PointSourceModel::calcGeoAtten()
{
    if (m_hypoDistance) 
    {
        // Determine the geometric attenuation based on a piecewise linear
        // calculation
        switch (m_location)
        {
            case Custom:
                // Do nothing
                break;
            case WUS:
                if (m_hypoDistance < 40.)
                    m_geoAtten = 1. / m_hypoDistance;
                else
                    m_geoAtten = 1./40. * sqrt(40./m_hypoDistance);
                break;
            case CEUS:
                if (m_hypoDistance < 70.)
                    m_geoAtten = 1. / m_hypoDistance;
                else if (m_hypoDistance < 130.)
                    m_geoAtten = 1. / 70.;
                else
                    m_geoAtten = 1./70. * sqrt(130./m_hypoDistance);
                break;
        }
        emit geoAttenChanged(m_geoAtten);
    }

    return;
}

// Compute the average value of a property over a given depth range.
double PointSourceModel::averageValue(const QVector<double> & thickness, const QVector<double> & property, const double maxDepth)
{
    // Depth to the base of the current layer
    double depth = 0;
    double sum = 0;

    for (int i = 0; i < thickness.size(); ++i) {
        depth += thickness.at(i);

        // Partial layer
        if (maxDepth < depth) {
            sum += (thickness.at(i) - (depth - maxDepth)) * property.at(i);
            break;
        }
        // Final infinite layer
        if (i == thickness.size()-1) {
            sum += (maxDepth - depth) * property.last();
            break;
        }
                
        sum += thickness.at(i) * property.at(i);
    }
    return sum / maxDepth;
}

void PointSourceModel::calcCrustalAmp()
{
    if (!m_crustAmpNeedsUpdate) {
        return;
    }

    m_freq = Dimension::logSpace(0.01, 100., 20);
    m_crustAmp.resize(m_freq.size());

    // Slowness (inverse of the crustal velocity
    QVector<double> slowness(m_crustVelocity);

    for (int i = 0; i < slowness.size(); ++i) {
        slowness[i] = 1./m_crustVelocity.at(i);
    }

    // average slowness over a depth range (1/velocity)
    QVector<double> avgSlow(m_freq.size(), slowness.first());
    // Frequency dependent depth
    QVector<double> depth_f(m_freq.size(), 0.);
        
    for (int i = 0; i < m_freq.size(); ++i) {
        double error = 0;
        int count = 0;

        do {
            ++count;
            depth_f[i] = 1. / (4 * m_freq.at(i) * avgSlow.at(i));
            double oldValue = avgSlow.at(i);
            avgSlow[i] = averageValue(m_crustThickness, slowness, depth_f.at(i));
            error = fabs((oldValue - avgSlow.at(i)) / avgSlow.at(i));
        } while (error > 0.005 && count < 10);
    }

    for (int i = 0; i < m_freq.size(); ++i) {
        // Average density for the depth range
        double avgDensity = averageValue(m_crustThickness, m_crustDensity, depth_f.at(i));

        m_crustAmp[i] = sqrt((m_shearVelocity * m_density) / (avgDensity / avgSlow.at(i)));
    }

    m_crustAmpNeedsUpdate = false;
    emit crustalAmpChanged();
}

QVector<double> PointSourceModel::calcFourierSpectrum(const QVector<double> & freq)
{
    QVector<double> fas(freq.size());
    
    // Compute the crustal amplification
    if (m_siteSpecificCrustalAmp)
        calcCrustalAmp();

    // Setup the interpolator
    gsl_interp * interpolator = gsl_interp_alloc(gsl_interp_linear, m_freq.size());
    gsl_interp_init(interpolator, m_freq.data(), m_crustAmp.data(), m_freq.size());
    gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

    // Conversion factor to convert from dyne-cm into gravity-sec
    // const double conv = 1e-20 / 981; 
    const double conv = 1e-18 / 981.; 

    // Constant term for the source component
    const double C = (0.55 * 2) / (M_SQRT2 * 4 * M_PI * m_density * pow(m_shearVelocity, 3));

    for (int i = 0; i < fas.size(); ++i)
    {

        // Source component
        double S = 1 / (1 + pow(freq.at(i)/m_cornerFreq, 2));
        double sourceComp =  C * m_seismicMoment * S;

        // Path component
        double pathAtten = m_pathAttenCoeff * pow(freq.at(i), m_pathAttenPower);
        double pathComp = m_geoAtten * exp((-M_PI * freq.at(i) * m_hypoDistance) / (pathAtten * m_shearVelocity));

        // Site component
        double siteAmp = gsl_interp_eval(interpolator, m_freq.data(), m_crustAmp.data(), freq.at(i), accelerator);
        double siteDim = exp(-M_PI * m_siteAtten * freq.at(i));
        double siteComp = siteAmp * siteDim;
        
        // Combine the three components and convert from displacement to
        // acceleleration
        fas[i] = conv * pow(M_2_PI * freq.at(i), 2) * sourceComp * pathComp * siteComp;
    }

    m_modified = false;

    gsl_interp_free(interpolator);
    gsl_interp_accel_free(accelerator);

    return fas;
}

QMap<QString, QVariant> PointSourceModel::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("momentMag", m_momentMag);
    map.insert("distance", m_distance);
    map.insert("depth", m_depth);
    map.insert("location", m_location);

    switch (m_location)
    {
        case Custom:
            map.insert("stressDrop", m_stressDrop);
            map.insert("geoAtten", m_geoAtten);
            map.insert("pathDurCoeff", m_pathDurCoeff);
            map.insert("pathAttenCoef", m_pathAttenCoeff);
            map.insert("pathAttenPower", m_pathAttenPower);
            map.insert("shearVelocity", m_shearVelocity);
            map.insert("density", m_density);
            map.insert("siteAtten", m_siteAtten);
            map.insert("siteSpecificCrustAmp", m_siteSpecificCrustalAmp);

            if (m_siteSpecificCrustalAmp) {
                map.insert("crustThickness", Serializer::toVariantList(m_crustThickness));
                map.insert("crustVelocity", Serializer::toVariantList(m_crustVelocity));
                map.insert("crustDensity", Serializer::toVariantList(m_crustDensity));
            } else {
                map.insert("freq", Serializer::toVariantList(m_freq));
                map.insert("crustAmp", Serializer::toVariantList(m_crustAmp));
            }
            

            break;
        case WUS:
        case CEUS:
            // Do nothing
            break;
    }

    return map;
}

void PointSourceModel::fromMap(const QMap<QString, QVariant> & map)
{
    m_location = (Location)map.value("location").toInt();
    m_distance = map.value("distance").toDouble();
    // Updates the hypocentral distance as well
    setDepth(map.value("depth").toDouble());
    setMomentMag(map.value("momentMag").toDouble());

    switch (m_location)
    {
        case Custom:
            m_stressDrop = map.value("stressDrop").toDouble();
            m_geoAtten = map.value("geoAtten").toDouble();
            m_pathDurCoeff = map.value("pathDurCoeff").toDouble();
            m_pathAttenCoeff = map.value("pathAttenCoef").toDouble();
            m_pathAttenPower = map.value("pathAttenPower").toDouble();
            m_shearVelocity = map.value("shearVelocity").toDouble();
            m_density = map.value("density").toDouble();
            m_siteAtten = map.value("siteAtten").toDouble();
            m_siteSpecificCrustalAmp = map.value("siteSpecificCrustAmp").toBool();
           

            if (m_siteSpecificCrustalAmp) {
                m_crustThickness = Serializer::fromVariantList(map.value("crustThickness").toList()).toVector();
                m_crustVelocity = Serializer::fromVariantList(map.value("crustVelocity").toList()).toVector();
                m_crustDensity = Serializer::fromVariantList(map.value("crustDensity").toList()).toVector();
                emit crustalVelChanged();

                calcCrustalAmp();
            } else {
                m_freq = Serializer::fromVariantList(map.value("freq").toList()).toVector();
                m_crustAmp = Serializer::fromVariantList(map.value("crustAmp").toList()).toVector();
                emit crustalAmpChanged();
            }

            calcCornerFreq();

            break;
        case WUS:
        case CEUS:
            setLocation(m_location);
            break;
    }
}
