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

#include "SoilTypeOutput.h"

#include "NonlinearPropertyOutput.h"
#include "SoilType.h"

#include <QDebug>

SoilTypeOutput::SoilTypeOutput(SoilType* soilType, OutputCatalog* catalog) :
    QObject(soilType), m_soilType(soilType), m_enabled(false)
{
    m_modulus = new NonlinearPropertyOutput(soilType->modulusModel(), catalog);

    m_modulus->setSoilName(soilType->name());
    connect(m_soilType, SIGNAL(nameChanged(QString)),
            m_modulus, SLOT(setSoilName(QString)));
    connect(m_soilType, SIGNAL(modulusModelChanged(NonlinearProperty*)),
            m_modulus, SLOT(setNonlinearProperty(NonlinearProperty*)));

    m_damping = new NonlinearPropertyOutput(soilType->dampingModel(), catalog);
    m_damping->setSoilName(soilType->name());
    connect(m_soilType, SIGNAL(nameChanged(QString)),
            m_damping, SLOT(setSoilName(QString)));
    connect(m_soilType, SIGNAL(dampingModelChanged(NonlinearProperty*)),
            m_damping, SLOT(setNonlinearProperty(NonlinearProperty*)));
}

SoilType* SoilTypeOutput::soilType() const
{
    return m_soilType;
}

QString SoilTypeOutput::name() const
{
    return m_soilType->name();
}

NonlinearPropertyOutput* SoilTypeOutput::modulus()
{
    return m_modulus;
}

NonlinearPropertyOutput* SoilTypeOutput::damping()
{
    return m_damping;
}

bool SoilTypeOutput::enabled() const
{
    return m_enabled;
}

void SoilTypeOutput::setEnabled(bool enabled)
{
    m_enabled = enabled;
    emit wasModified();
}

void SoilTypeOutput::ptRead(const ptree &pt)
{
    m_enabled = pt.get<bool>("enabled");

    ptree modulus = pt.get_child("modulus");
    m_modulus->ptRead(modulus);

    ptree damping = pt.get_child("damping");
    m_damping->ptRead(damping);
}

void SoilTypeOutput::ptWrite(ptree &pt) const
{
    pt.put("enabled", m_enabled);

    ptree modulus;
    m_modulus->ptWrite(modulus);
    pt.add_child("modulus", modulus);

    ptree damping;
    m_damping->ptWrite(damping);
    pt.add_child("damping", damping);
}

QDataStream & operator<< (QDataStream & out, const SoilTypeOutput* sto)
{
    out << (quint8)1;

    out << sto->m_enabled
            << sto->m_modulus
            << sto->m_damping;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilTypeOutput* sto)
{
    quint8 ver;
    in >> ver;

    in >> sto->m_enabled
            >> sto->m_modulus
            >> sto->m_damping;

    return in;
}
