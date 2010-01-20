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
#include "Serializer.h"

NonlinearPropertyOutput::NonlinearPropertyOutput()
{
}

void NonlinearPropertyOutput::clear()
{
    m_strain.clear();
    m_prop.clear();
}

void NonlinearPropertyOutput::setStrain( const QVector<double> & strain )
{
    m_strain = strain;
}
        
QMap<QString, QVariant> NonlinearPropertyOutput::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;
    
    map.insert("enabled", m_enabled);

    if (saveData) {
        QList<QVariant> list;
        // Save the data
        for ( int i = 0; i < m_prop.size(); ++i )
            list << QVariant(Serializer::toVariantList(m_prop.at(i)));

        map.insert("prop", list);
        
        // Save the strain
        map.insert("strain", Serializer::toVariantList(m_strain));
    }
    
    return map;
}

void NonlinearPropertyOutput::fromMap(const QMap<QString, QVariant> & map)
{
    m_enabled = map.value("enabled").toBool();
    
    if (map.contains("prop")) {
        // Data
        m_prop.clear();

        QList<QVariant> list = map.value("prop").toList();
        for ( int i = 0; i < list.size(); ++i)
            m_prop << Serializer::fromVariantList(list.at(i).toList()).toVector();
    }
    
    if (map.contains("strain")) {
        // Strain
        m_strain.clear();
        
        m_strain = Serializer::fromVariantList(map.value("strain").toList()).toVector();
    }
}
