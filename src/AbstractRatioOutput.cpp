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

#include "AbstractRatioOutput.h"

#include "Algorithms.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"

#include <QDebug>

AbstractRatioOutput::AbstractRatioOutput(OutputCatalog* catalog)
    : AbstractOutput(catalog)
{
    m_inDepth = -1;
    m_inType = AbstractMotion::Outcrop;
    m_outDepth = 0;
    m_outType = AbstractMotion::Outcrop;

    m_statistics = new OutputStatistics(this);
    connect(m_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}

QString AbstractRatioOutput::fullName() const
{
    return tr("Ratio -- %1 -- %2")
            .arg(prefix())
            .arg(name());
}

double AbstractRatioOutput::inDepth() const
{
    return m_inDepth;
}

AbstractMotion::Type AbstractRatioOutput::inType() const
{
    return m_inType;
}

void AbstractRatioOutput::setInType(AbstractMotion::Type inType)
{
    if (m_inType != inType) {
        m_inType = inType;

        emit inTypeChanged(m_inType);
        emit wasModified();
    }
}

double AbstractRatioOutput::outDepth() const
{
    return m_outDepth;
}

AbstractMotion::Type AbstractRatioOutput::outType() const
{
    return m_outType;
}

void AbstractRatioOutput::setOutType(AbstractMotion::Type outType)
{
    if (m_outType != outType) {
        m_outType = outType;

        emit outTypeChanged(m_outType);
        emit wasModified();
    }
}

void AbstractRatioOutput::setInDepth(double inDepth)
{
    if (m_inDepth != inDepth) {
        m_inDepth = inDepth;

        emit inDepthChanged(m_inDepth);
        emit wasModified();
    }
}

void AbstractRatioOutput::setInType(int inType)
{
    setInType((AbstractMotion::Type)inType);
}

void AbstractRatioOutput::setOutDepth(double outDepth)
{
    if (m_outDepth != outDepth) {
        m_outDepth = outDepth;

        emit outDepthChanged(m_outDepth);
        emit wasModified();
    }
}

void AbstractRatioOutput::setOutType(int outType)
{
    setOutType((AbstractMotion::Type)outType);
}

QString AbstractRatioOutput::fileName(int motion) const
{
    Q_UNUSED(motion);

    return prefix() + '-' + shortName();
}

const QString AbstractRatioOutput::prefix() const
{
    return QString("%1 (%2) from %3 (%4)")
            .arg(locationToString(m_outDepth))
            .arg(AbstractMotion::typeList().at(m_outType))
            .arg(locationToString(m_inDepth))
            .arg(AbstractMotion::typeList().at(m_inType));
}

void AbstractRatioOutput::ptRead(const ptree &pt)
{
    AbstractOutput::ptRead(pt);
    m_outType = (AbstractMotion::Type) pt.get<int>("outType");
    m_inType = (AbstractMotion::Type) pt.get<int>("inType");
    m_outDepth = pt.get<double>("outDepth");
    m_inDepth = pt.get<double>("inDepth");
}

void AbstractRatioOutput::ptWrite(ptree &pt) const
{
    AbstractOutput::ptWrite(pt);
    pt.put("outType", (int) m_outType);
    pt.put("inType", (int) m_inType);
    pt.put("outDepth", m_outDepth);
    pt.put("inDepth", m_inDepth);
}

QDataStream & operator<< (QDataStream & out, const AbstractRatioOutput* aro)
{
    out << (quint8)1;

    out << static_cast<const AbstractOutput*>(aro)
            << (int)aro->m_outType
            << aro->m_outDepth
            << (int)aro->m_inType
            << aro->m_inDepth;

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractRatioOutput* aro)
{
    quint8 ver;
    in >> ver;

    int outType;
    int inType;

    in >> static_cast<AbstractOutput*>(aro)
            >> outType
            >> aro->m_outDepth
            >> inType
            >> aro->m_inDepth;

    aro->m_outType = (AbstractMotion::Type)outType;
    aro->m_inType = (AbstractMotion::Type)inType;

    return in;
}
