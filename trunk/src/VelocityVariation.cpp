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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "VelocityVariation.h"

#include "ProfileRandomizer.h"
#include "RockLayer.h"
#include "SoilLayer.h"
#include "Units.h"

#include <gsl/gsl_randist.h>

#include <cmath>
#include <cfloat>

VelocityVariation::VelocityVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    m_rng(rng), m_profileRandomizer(profileRandomizer)
{
    connect(m_profileRandomizer, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateEnabled()));

    m_enabled = false;
    m_stdevIsLayerSpecific = false;

    m_stdevModel = Custom;
    setStdevModel(USGS_C);

    m_correlModel = Custom;
    setCorrelModel(USGS_C);
}

QStringList VelocityVariation::modelList()
{
    QStringList list;

    list << tr("Custom")
        << tr("GeoMatrix A&B, Rock & Shallow Soil")
        << tr("GeoMatrix C&D, Deep Narrow/Broad Soil");

    if (Units::instance()->system() == Units::Metric ) {
        list << tr("USGS A&B, >360 m/s")
            << tr("USGS C&D, <360 m/s")
            << tr("USGS A, >750 m/s")
            << tr("USGS B, 360-750 m/s")
            << tr("USGS C, 180 to 360 m/s")
            << tr("USGS D, <180 m/s");
    } else {
        list << tr("USGS A&B, >1200 ft/s")
            << tr("USGS C&D, <1200 ft/s")
            << tr("USGS A, >2500 ft/s")
            << tr("USGS B, 1200-2500 ft/s")
            << tr("USGS C, 600 to 1200 ft/s")
            << tr("USGS D, <600 ft/s");
    }

    return list;
}

bool VelocityVariation::enabled() const
{
    return m_profileRandomizer->enabled() && m_enabled;
}

void VelocityVariation::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        emit enabledChanged(this->enabled());
        emit stdevIsLayerSpecificChanged(stdevIsLayerSpecific());
        emit wasModified();
    }
}

void VelocityVariation::updateEnabled()
{
    emit enabledChanged(enabled());
    emit stdevIsLayerSpecificChanged(stdevIsLayerSpecific());
}

VelocityVariation::Model VelocityVariation::stdevModel() const
{
    return m_stdevModel;
}

bool VelocityVariation::stdevCustomEnabled() const
{
    return m_stdevModel == Custom;
}

void VelocityVariation::setStdevModel(Model model)
{
    if (m_stdevModel != model) {
        m_stdevModel = model;

        switch (m_stdevModel) {
        case Custom:
            break;
        case GeoMatrix_AB:
            setStdev(0.46);
            break;
        case GeoMatrix_CD:
            setStdev(0.38);
            break;
        case USGS_AB:
            setStdev(0.35);
            break;
        case USGS_CD:
            setStdev(0.36);
            break;
        case USGS_A:
            setStdev(0.36);
            break;
        case USGS_B:
            setStdev(0.27);
            break;
        case USGS_C:
            setStdev(0.31);
            break;
        case USGS_D:
            setStdev(0.37);
            break;
        }

        emit stdevModelChanged(m_stdevModel);
        emit stdevCustomEnabledChanged(stdevCustomEnabled());
        emit wasModified();
    }
}

void VelocityVariation::setStdevModel(int model)
{
    setStdevModel((Model)model);
}

bool VelocityVariation::stdevIsLayerSpecific() const
{
    return m_stdevIsLayerSpecific;
}

void VelocityVariation::setStdevIsLayerSpecific(bool b)
{
    if (m_stdevIsLayerSpecific != b) {
        m_stdevIsLayerSpecific = b;

        emit stdevIsLayerSpecificChanged(b);
        emit wasModified();
    }
}

double VelocityVariation::stdev() const
{
    return m_stdev;
}

void VelocityVariation::setStdev(double stdev)
{
    if (fabs(m_stdev - stdev) > DBL_EPSILON) {
        m_stdev = stdev;

        emit stdevChanged(m_stdev);
        emit wasModified();
    }
}

VelocityVariation::Model VelocityVariation::correlModel() const
{
    return m_correlModel;
}

bool VelocityVariation::correlCustomEnabled() const
{
    return m_correlModel == Custom;
}

void VelocityVariation::setCorrelModel(Model model)
{
    if (m_correlModel != model) {
        m_correlModel = model;

        switch (m_correlModel) {
        case Custom:
            break;
        case GeoMatrix_AB:
            setCorrelInitial(0.96);
            setCorrelFinal(0.96);
            setCorrelDelta(13.1);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.095);
            break;
        case GeoMatrix_CD:
            setCorrelInitial(0.99);
            setCorrelFinal(1.00);
            setCorrelDelta(8.0);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.160);
            break;
        case USGS_AB:
            setCorrelInitial(0.95);
            setCorrelFinal(1.00);
            setCorrelDelta(4.2);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.138);
            break;
        case USGS_CD:
            setCorrelInitial(0.99);
            setCorrelFinal(1.00);
            setCorrelDelta(3.9);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.293);
            break;
        case USGS_A:
            setCorrelInitial(0.95);
            setCorrelFinal(0.42);
            setCorrelDelta(3.4);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.063);
            break;
        case USGS_B:
            setCorrelInitial(0.97);
            setCorrelFinal(1.00);
            setCorrelDelta(3.8);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.293);
            break;
        case USGS_C:
            setCorrelInitial(0.99);
            setCorrelFinal(0.98);
            setCorrelDelta(3.9);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.344);
            break;
        case USGS_D:
            setCorrelInitial(0.00);
            setCorrelFinal(0.50);
            setCorrelDelta(5.0);
            setCorrelIntercept(0.0);
            setCorrelExponent(0.744);
            break;
        }

        emit correlModelChanged(m_correlModel);
        emit correlCustomEnabledChanged(correlCustomEnabled());
        emit wasModified();
    }
}

void VelocityVariation::setCorrelModel(int model)
{
    setCorrelModel((Model)model);
}

double VelocityVariation::correlInitial() const
{
    return m_correlInitial;
}

void VelocityVariation::setCorrelInitial(double correlInitial)
{
    if (fabs(m_correlInitial - correlInitial) > DBL_EPSILON) {
        m_correlInitial = correlInitial;

        emit correlInitialChanged(m_correlInitial);
        emit wasModified();
    }
}

double VelocityVariation::correlFinal() const
{
    return m_correlFinal;
}

void VelocityVariation::setCorrelFinal(double correlFinal)
{
    if (fabs(m_correlFinal - correlFinal) > DBL_EPSILON) {
        m_correlFinal = correlFinal;

        emit correlFinalChanged(m_correlFinal);
        emit wasModified();
    }
}

double VelocityVariation::correlDelta() const
{
    return m_correlDelta;
}

void VelocityVariation::setCorrelDelta(double correlDelta)
{
    if (fabs(m_correlDelta - correlDelta) > DBL_EPSILON) {
        m_correlDelta = correlDelta;

        emit correlDeltaChanged(m_correlDelta);
        emit wasModified();
    }
}

double VelocityVariation::correlIntercept() const
{
    return m_correlIntercept;
}

void VelocityVariation::setCorrelIntercept(double correlIntercept)
{
    if (fabs(m_correlIntercept - correlIntercept) > DBL_EPSILON) {
        m_correlIntercept = correlIntercept;

        emit correlInterceptChanged(m_correlIntercept);
        emit wasModified();
    }
}

double VelocityVariation::correlExponent() const
{
    return m_correlExponent;
}

void VelocityVariation::setCorrelExponent(double correlExponent)
{
    if (fabs(m_correlExponent - correlExponent) > DBL_EPSILON) {
        m_correlExponent = correlExponent;

        emit correlExponentChanged(m_correlExponent);
        emit wasModified();
    }
}

void VelocityVariation::vary(QList<SoilLayer*> & soilLayers, RockLayer* bedrock) const
{
    // Random variable for the previous layer
    double prevRandVar = 0;
    // Current random variable and standard deviation
    double randVar = 0;
    double stdev = 0;

    for (int i = 0; i < soilLayers.size(); ++i) {
        stdev = m_stdevIsLayerSpecific ? soilLayers.at(i)->stdev() : m_stdev;

        if (i == 0) {
            // First layer is not correlated
            randVar = gsl_ran_gaussian(m_rng, stdev);
        } else {
            // Depth at the middle of the layer
            double depthToMid = soilLayers.at(i)->depth() + soilLayers.at(i)->thickness()/2.;

            // If the English units are used convert the depthToMid to meters
            depthToMid *= Units::instance()->toMeters();

            // Depth dependent correlation
            const double dCorrel =  (depthToMid <= 200) ?
                                    m_correlFinal * pow((depthToMid + m_correlInitial)
                                                        / (200 + m_correlInitial), m_correlExponent)
                                                            : m_correlFinal;
            // Thickness dependent correlation
            const double tCorrel = m_correlInitial * exp(-soilLayers.at(i)->thickness() / m_correlDelta );

            // Combine the correlations
            double correl = (1 - dCorrel) * tCorrel + dCorrel;


            // Compute the random variable taking into account the correlation from
            // the previous layer.
            randVar = correl * prevRandVar
                             + gsl_ran_gaussian(m_rng, stdev) * sqrt( 1 - correl * correl);
        }

        if (soilLayers.at(i)->isVaried()) {
            // Vary the layer using the random variable
            soilLayers.at(i)->setVaried(soilLayers.at(i)->avg() * exp(randVar));
        } else {
            // Use average velocity for the layer
            soilLayers.at(i)->reset();
        }

        // Save the previous random variable
        prevRandVar = randVar;
    }

    // Randomize the bedrock layer
    //
    // The bedrock layer is assumed to be perfectly correlated to the previous
    // soil layer.  In addition the velocity is set to be at least as fast as
    // the velocity of the last soil layer.
    //
    if (bedrock->isVaried()) {
        double randVar = prevRandVar;

        // Adjust the random variable by the ratio of the standard deviation
        if (m_stdevIsLayerSpecific) {
            randVar *= bedrock->stdev() / soilLayers.last()->stdev();
        }

        bedrock->setVaried(qMax(
                    bedrock->avg() * exp(randVar),
                    soilLayers.last()->shearVel()));

    } else {
        // Use average velocity for the layer
        bedrock->reset();
    }
}

QDataStream & operator<< (QDataStream & out, const VelocityVariation* vv)
{
    out << (quint8)1;

    out << vv->m_enabled
            << (int)vv->m_stdevModel;

    if (vv->m_stdevModel == VelocityVariation::Custom) {
        out << vv->m_stdevIsLayerSpecific
                << vv->m_stdev;
    }

    out << (int)vv->m_correlModel;

    if (vv->m_correlModel == VelocityVariation::Custom) {
        out << vv->m_correlInitial
                << vv->m_correlFinal
                << vv->m_correlDelta
                << vv->m_correlIntercept
                << vv->m_correlExponent;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, VelocityVariation* vv)
{
    quint8 ver;
    in >> ver;

    int stdevModel;
    int correlModel;

    in >> vv->m_enabled
            >> stdevModel;

    vv->m_stdevModel = (VelocityVariation::Model)stdevModel;

    if (vv->m_stdevModel == VelocityVariation::Custom) {
        in >> vv->m_stdevIsLayerSpecific
                >> vv->m_stdev;
    }

    in >> correlModel;
    vv->m_correlModel = (VelocityVariation::Model)correlModel;

    if (vv->m_correlModel == VelocityVariation::Custom) {
        in >> vv->m_correlInitial
                >> vv->m_correlFinal
                >> vv->m_correlDelta
                >> vv->m_correlIntercept
                >> vv->m_correlExponent;
    }

    return in;
}
