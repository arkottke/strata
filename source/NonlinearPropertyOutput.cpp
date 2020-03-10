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

#include "NonlinearPropertyOutput.h"

#include "AbstractCalculator.h"
#include "MyQwtCompatibility.h"
#include "NonlinearProperty.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilType.h"

NonlinearPropertyOutput::NonlinearPropertyOutput(
        NonlinearProperty* nonlinearProperty, OutputCatalog* catalog)
    : AbstractOutput(catalog), _nonlinearProperty(nonlinearProperty)
{
    _statistics = new OutputStatistics(this);
    connect(_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}


auto NonlinearPropertyOutput::name() const -> QString
{
    switch (_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return tr("Damping Ratio");
    case NonlinearProperty::ModulusReduction:
        return tr("Shear Modulus Reduction");
    default:
        return "";
    }
}

auto NonlinearPropertyOutput::fullName() const -> QString
{
    return tr("Nonlinear Curve -- %1 -- %2")
            .arg(prefix())
            .arg(name());
}

auto NonlinearPropertyOutput::shortName() const -> QString
{
    switch (_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return "damping";
    case NonlinearProperty::ModulusReduction:
        return "modulus";
    default:
        return "";
    }
}

auto NonlinearPropertyOutput::motionIndependent() const -> bool
{
    return true;
}

auto NonlinearPropertyOutput::fileName(int motion) const -> QString
{
    Q_UNUSED(motion);

    return "nlCurve-" + prefix() + "-" + shortName();
}

auto NonlinearPropertyOutput::soilName() const -> const QString&
{
    return _soilName;
}

void NonlinearPropertyOutput::setNonlinearProperty(NonlinearProperty* nonlinearProperty)
{
    _nonlinearProperty = nonlinearProperty;
}

void NonlinearPropertyOutput::setSoilName(const QString &soilName)
{
    if (_soilName != soilName) {
        _soilName = soilName;

        emit soilNameChanged(_soilName);
        emit wasModified();
    }
}

auto NonlinearPropertyOutput::xScaleEngine() const -> QwtScaleEngine*
{
    return logScaleEngine();
}

auto NonlinearPropertyOutput::yScaleEngine() const -> QwtScaleEngine*
{
    return new QwtLinearScaleEngine;
}

auto NonlinearPropertyOutput::xLabel() const -> const QString
{
    return tr("Strain (%)");
}

auto NonlinearPropertyOutput::yLabel() const -> const QString
{
    switch (_nonlinearProperty->type()) {
    case NonlinearProperty::Damping:
        return tr("Damping (%)");
    case NonlinearProperty::ModulusReduction:
        return tr("Normalized Shear Modulus (G/G_max)");
    default:
        return "";
    }
}

auto NonlinearPropertyOutput::prefix() const -> const QString
{
    return _soilName;
}

auto NonlinearPropertyOutput::suffix() const -> const QString
{
    return "";
}

auto NonlinearPropertyOutput::ref(int motion) const -> const QVector<double>&
{
    Q_UNUSED(motion);

    return _nonlinearProperty->strain();
}

void NonlinearPropertyOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(calculator);
    Q_UNUSED(ref);

    data = _nonlinearProperty->varied();
}
