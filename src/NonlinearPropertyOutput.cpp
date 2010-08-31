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

#include "NonlinearPropertyOutput.h"

#include "AbstractCalculator.h"
#include "NonlinearProperty.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilType.h"

#include <qwt_scale_engine.h>

NonlinearPropertyOutput::NonlinearPropertyOutput(
        NonlinearProperty* nonlinearProperty, OutputCatalog* catalog)
    : AbstractOutput(catalog), m_nonlinearProperty(nonlinearProperty)
{
    m_statistics = new OutputStatistics(this);
    connect(m_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}


QString NonlinearPropertyOutput::name() const
{
    switch (m_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return tr("Damping Ratio");
    case NonlinearProperty::ModulusReduction:
        return tr("Shear Modulus Reduction");
    default:
        return "";
    }
}

QString NonlinearPropertyOutput::fullName() const
{
    return tr("Nonlinear Curve -- %1 -- %2")
            .arg(prefix())
            .arg(name());
}

QString NonlinearPropertyOutput::shortName() const
{
    switch (m_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return "damping";
    case NonlinearProperty::ModulusReduction:
        return "modulus";
    default:
        return "";
    }
}

bool NonlinearPropertyOutput::motionIndependent() const
{
    return true;
}

QString NonlinearPropertyOutput::fileName(int motion) const
{
    Q_UNUSED(motion);

    return "nlCurve-" + prefix() + "-" + shortName();
}

const QString& NonlinearPropertyOutput::soilName() const
{
    return m_soilName;
}

void NonlinearPropertyOutput::setNonlinearProperty(NonlinearProperty* nonlinearProperty)
{
    m_nonlinearProperty = nonlinearProperty;
}

void NonlinearPropertyOutput::setSoilName(const QString &soilName)
{
    if (m_soilName != soilName) {
        m_soilName = soilName;

        emit soilNameChanged(m_soilName);
        emit wasModified();
    }
}

QwtScaleEngine* NonlinearPropertyOutput::xScaleEngine() const
{
    return new QwtLog10ScaleEngine;
}

QwtScaleEngine* NonlinearPropertyOutput::yScaleEngine() const
{
    return new QwtLinearScaleEngine;
}

const QString NonlinearPropertyOutput::xLabel() const
{
    return tr("Strain (%)");
}

const QString NonlinearPropertyOutput::yLabel() const
{
    switch (m_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return tr("Damping (%)");
    case NonlinearProperty::ModulusReduction:
        return tr("Normalized Shear Modulus (G/G_max)");
    default:
        return "";
    }
}

const QString NonlinearPropertyOutput::prefix() const
{
    return m_soilName;
}

const QString NonlinearPropertyOutput::suffix() const
{
    return "";
}

const QVector<double>& NonlinearPropertyOutput::ref(int motion) const
{
    Q_UNUSED(motion);

    return m_nonlinearProperty->strain();
}

void NonlinearPropertyOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(calculator);
    Q_UNUSED(ref);

    data = m_nonlinearProperty->varied();
}
