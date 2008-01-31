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

#include "ResponseSpectrum.h"
#include "Serializer.h"

ResponseSpectrum::ResponseSpectrum(QObject * parent)
    : QObject(parent)
{
}

void ResponseSpectrum::reset()
{
    m_modified = false; 
    m_damping = 5.0;

    m_period.clear();
    m_sa.clear();
}
        
bool ResponseSpectrum::modified() const
{
    return m_modified;
}

void ResponseSpectrum::setModified(bool modified)
{
    m_modified = modified;
}

double ResponseSpectrum::damping() const
{
    return m_damping;
}

void ResponseSpectrum::setDamping(double damping)
{
    m_damping = damping;
}

QVector<double> & ResponseSpectrum::period()
{
    return m_period;
}

QVector<double> & ResponseSpectrum::sa()
{
    return m_sa;
}

QMap<QString, QVariant> ResponseSpectrum::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("modified", m_modified);
    map.insert("damping", m_damping);
    map.insert("period", Serializer::toVariantList(m_period));
    map.insert("sa", Serializer::toVariantList(m_sa));

    return map;
}


void ResponseSpectrum::fromMap( const QMap<QString, QVariant> & map)
{
    m_modified = map.value("modified").toBool();
    m_damping = map.value("damping").toDouble();
    m_period = Serializer::fromVariantList(map.value("period").toList()).toVector();
    m_sa = Serializer::fromVariantList(map.value("sa").toList()).toVector();

    emit dataChanged();
}
