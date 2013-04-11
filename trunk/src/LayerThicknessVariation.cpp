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

#include "LayerThicknessVariation.h"

#include "ProfileRandomizer.h"

#include <cfloat>
#include <cmath>

#include <gsl/gsl_randist.h>

LayerThicknessVariation::LayerThicknessVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer) :
    m_rng(rng), m_profileRandomizer(profileRandomizer)
{
    connect(m_profileRandomizer, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateEnabled()));

    m_enabled = false;
    m_model = Custom;
    setModel(Default);
}


QStringList LayerThicknessVariation::modelList()
{
    QStringList list;

    list << tr("Custom")
        << tr("Default (Toro 1995)");

    return list;
}

bool LayerThicknessVariation::enabled() const
{
    return m_profileRandomizer->enabled() && m_enabled;
}

void LayerThicknessVariation::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        emit enabledChanged(this->enabled());
        emit wasModified();
    }
}

void LayerThicknessVariation::updateEnabled()
{
    emit enabledChanged(enabled());
}


LayerThicknessVariation::Model LayerThicknessVariation::model()
{
    return m_model;
}

void LayerThicknessVariation::setModel(LayerThicknessVariation::Model model)
{
    if (m_model != model) {
        m_model = model;

        switch (m_model) {
        case Default:
            setCoeff(1.98);
            setInitial(10.86);
            setExponent(-0.89);
            break;
        case Custom:
            break;
        }

        emit modelChanged(m_model);
        emit customEnabledChanged(customEnabled());
        emit wasModified();
    }
}

void LayerThicknessVariation::setModel(int model)
{
    setModel((Model)model);
}


bool LayerThicknessVariation::customEnabled() const
{
    return m_model == Custom;
}

double LayerThicknessVariation::coeff() const
{
    return m_coeff;
}

void LayerThicknessVariation::setCoeff(double coeff)
{
    if (fabs(m_coeff - coeff) > DBL_EPSILON) {
        m_coeff = coeff;

        emit coeffChanged(m_coeff);
        emit wasModified();
    }
}

double LayerThicknessVariation::initial() const
{
    return m_initial;
}

void LayerThicknessVariation::setInitial(double initial)
{
    if (fabs(m_initial - initial) > DBL_EPSILON) {
        m_initial = initial;

        emit initialChanged(m_initial);
        emit wasModified();
    }
}

double LayerThicknessVariation::exponent() const
{
    return m_exponent;
}

void LayerThicknessVariation::setExponent(double exponent)
{
    if (fabs(m_exponent - exponent) > DBL_EPSILON) {
        m_exponent = exponent;

        emit exponentChanged(m_exponent);
        emit wasModified();
    }
}


void LayerThicknessVariation::reset()
{
    setEnabled(false);
    setModel(Default);
}

QList<double> LayerThicknessVariation::vary(double depthToBedrock) const
{
    // The thickness of the layers
    QList<double> thicknesses;

    // The layering is generated using a non-homogenous Poisson process.  The
    // following routine is used to generate the layering.  The rate function,
    // \lambda(t), is integrated from 0 to t to generate cumulative rate
    // function, \Lambda(t).  This function is then inverted producing
    // \Lambda^-1(t).  Random variables are produced using the a exponential
    // random variation with mu = 1 and converted to the nonhomogenous
    // variables using the inverted function.

    // Random variable that is a sum of exponential random variables
    double sum  = 0;

    // Previously computed depth
    double prevDepth = 0;

    while ( prevDepth < depthToBedrock ) {
        // Add a random increment
        sum += gsl_ran_exponential(m_rng, 1.0);

        // Convert between x and depth using the inverse of \Lambda(t)
        double depth =
            pow((m_exponent * sum) / m_coeff
                    + sum / m_coeff
                    + pow(m_initial, m_exponent + 1)
                    , 1 / (m_exponent + 1)
               ) - m_initial;

        // Add the thickness
        thicknesses << depth - prevDepth;

        prevDepth = depth;
    }


    // Correct the last layer of thickness so that the total depth is equal to the maximum depth
    if (thicknesses.size())
        thicknesses.last() = thicknesses.last() - (prevDepth - depthToBedrock);

    return thicknesses;
}

QDataStream & operator<< (QDataStream & out, const LayerThicknessVariation* ltv)
{
    out << (quint8)1;

    out << ltv->m_enabled
        << (int)ltv->m_model;

    if (ltv->m_model == LayerThicknessVariation::Custom) {
        out << ltv->m_coeff
            << ltv->m_initial
            << ltv->m_exponent;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, LayerThicknessVariation* ltv)
{
    quint8 ver;
    in >> ver;

    int model;

    in >> ltv->m_enabled
            >> model;

    ltv->setModel(model);
    if (ltv->m_model == LayerThicknessVariation::Custom) {
        in >> ltv->m_coeff
           >> ltv->m_initial
           >> ltv->m_exponent;
    }

    return in;
}
