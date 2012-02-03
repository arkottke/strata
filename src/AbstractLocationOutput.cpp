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

#include "AbstractLocationOutput.h"

#include "Algorithms.h"
#include "OutputStatistics.h"
#include "Units.h"

AbstractLocationOutput::AbstractLocationOutput(OutputCatalog* catalog)
    : AbstractOutput(catalog)
{
    m_type = AbstractMotion::Outcrop;
    m_depth = 0;
}

bool AbstractLocationOutput::needsOutputConditions() const
{
    return true;
}

QString AbstractLocationOutput::fullName() const
{
    return tr("Location -- %1 -- %2")
            .arg(prefix())
            .arg(name());
}

double AbstractLocationOutput::depth() const
{
    return m_depth;
}

void AbstractLocationOutput::setDepth(double depth)
{
    if (m_depth != depth) {
        m_depth = depth;

        emit depthChanged(m_depth);
        emit wasModified();
    }
}

AbstractMotion::Type AbstractLocationOutput::type() const
{
    return m_type;
}

void AbstractLocationOutput::setType(AbstractMotion::Type type)
{
    if (m_type != type) {
        m_type = type;

        emit typeChanged(m_type);
        emit wasModified();
    }
}

void AbstractLocationOutput::setType(int type)
{
    setType((AbstractMotion::Type)type);
}

QString AbstractLocationOutput::fileName(int motion) const
{
    Q_UNUSED(motion)

    return prefix() + "-" + shortName();
}

const QString AbstractLocationOutput::prefix() const
{
    return QString("%1 (%3)")
            .arg(locationToString(m_depth))
            .arg(AbstractMotion::typeList().at(m_type));
}

QDataStream & operator<< (QDataStream & out, const AbstractLocationOutput* alo)
{
    out << (quint8)1;

    out << static_cast<const AbstractOutput*>(alo)
            << (int)alo->m_type
            << alo->m_depth;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractLocationOutput* alo)
{
    quint8 ver;
    in >> ver;

    int type;
    in >> static_cast<AbstractOutput*>(alo)
            >> type
            >> alo->m_depth;

    alo->m_type = (AbstractMotion::Type)type;

    return in;
}
