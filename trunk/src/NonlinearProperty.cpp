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

#include "NonlinearProperty.h"
#include "Serializer.h"
#include <QObject>
#include <QDebug>
#include <cmath>

NonlinearProperty::NonlinearProperty(QString name, Type type, Source source, QObject * parent)
    : QObject(parent), m_name(name), m_type(type), m_source(source)
{
}

NonlinearProperty::NonlinearProperty( const QMap<QString, QVariant> & map )
{
    fromMap(map);
}

NonlinearProperty::~NonlinearProperty()
{
}
        
NonlinearProperty::Type NonlinearProperty::type() const
{
    return m_type;
}

void NonlinearProperty::setType(NonlinearProperty::Type type)
{
    m_type = type;
}

QString NonlinearProperty::typeLabel() const
{
    if (m_type == ModulusReduction)
        return QObject::tr("G/G_max");
    else if ( m_type == Damping )
        return QObject::tr("Damping (%)");

    return QObject::tr("Undefined");
}

const QString & NonlinearProperty::name() const
{
    return m_name;
}

void NonlinearProperty::setName( const QString & name)
{
    m_name = name;
}

NonlinearProperty::Source NonlinearProperty::source() const
{
    return m_source;
}

void NonlinearProperty::setSource(NonlinearProperty::Source source)
{
    m_source = source;
}

QList<double> & NonlinearProperty::strain()
{
    return m_strain;
}

QList<double> & NonlinearProperty::avg()
{
    return m_avg;
}

QList<double> & NonlinearProperty::prop()
{
    return m_prop;
}

void NonlinearProperty::reset()
{
    m_prop = m_avg;
}

void NonlinearProperty::copyValues( NonlinearProperty * other )
{
    m_name = other->name();
    m_source = other->source();

    if ( other->source() != NonlinearProperty::Temporary ) {
        m_strain = other->strain();
        m_avg = other->avg();
    }
}

double NonlinearProperty::interp(const double strain) const
{
    // If the strain is smaller than the provided strain, return the lowest value
    if ( strain < m_strain.first() )
        return m_prop.first();
    else if ( strain > m_strain.last() )
        return m_prop.last();
    
	for ( int i = 0; i < m_strain.size() - 1; ++i)
	{
		if( m_strain.at(i) <= strain && strain <= m_strain.at(i+1) )
		{
			double slope = ( m_prop.at(i+1) - m_prop.at(i) ) / log10( m_strain.at(i+1) / m_strain.at(i) );
			return m_prop.at(i) + slope * log10( strain / m_strain.at(i) );
		}
	}

	// Nonlinear properties can not be negative, so this will cause an error
	return -1;
}

QMap<QString, QVariant> NonlinearProperty::toMap() const
{
    QMap<QString, QVariant> map;

    map.insert("type", m_type);
    map.insert("name", m_name);
    map.insert("source", m_source);
    map.insert("strain", Serializer::toVariantList(m_strain));
    map.insert("avg", Serializer::toVariantList(m_avg));

    return map;
}

void NonlinearProperty::fromMap(const QMap<QString, QVariant> & map)
{
    m_type = (Type)map.value("type").toInt();
    m_name = map.value("name").toString();
    m_source = (Source)map.value("source").toInt();
    m_strain = Serializer::fromVariantList( map.value("strain").toList());
    m_avg = Serializer::fromVariantList( map.value("avg").toList());
}

QString NonlinearProperty::toHtml() const
{
   QString html;

   QString source;

   switch (m_source)
   {
       case Temporary:
           source = QObject::tr("Temporary");
       case HardCoded:
           source = QObject::tr("Hard Coded");
       case UserDefined:
           source = QObject::tr("User Defined");
       case Computed:
           source = QObject::tr("Computed");
   }

   html += QString(QObject::tr(
               "<h4>%1</h4>"
               "<table border = \"0\">"
               "<tr><td><strong>Type:</strong></td><td>%1</td></tr>"
               "<tr><td><strong>Name:</strong></td><td>%2</td></tr>"
               "<tr><td><strong>Source:</strong></td><td>%3</td></tr>"
               "</table>"))
       .arg(typeLabel())
       .arg(m_name)
       .arg(source);

   // Create a table of the strain and properties

   html += QString(QObject::tr(
            "<table border = \"1\">"
            "<tr><th>Strain (%)</th><th>%1</th></tr>"
            )).arg(typeLabel());

   for (int i = 0; i < m_strain.size(); ++i)
       html += QString("<tr><td>%1</td><td>%2</td></tr>")
           .arg(m_strain.at(i))
           .arg(m_avg.at(i));

   html += "</table>";

   return html;
}
