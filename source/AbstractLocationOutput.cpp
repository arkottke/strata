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

#include "AbstractLocationOutput.h"

#include "Algorithms.h"
#include "OutputStatistics.h"
#include "Units.h"

AbstractLocationOutput::AbstractLocationOutput(OutputCatalog* catalog)
    : AbstractOutput(catalog)
{
    _type = AbstractMotion::Outcrop;
    _depth = 0;
}

auto AbstractLocationOutput::needsOutputConditions() const -> bool
{
    return true;
}

auto AbstractLocationOutput::fullName() const -> QString
{
    return tr("Location -- %1 -- %2")
            .arg(prefix())
            .arg(name());
}

auto AbstractLocationOutput::depth() const -> double
{
    return _depth;
}

void AbstractLocationOutput::setDepth(double depth)
{
    if (_depth != depth) {
        _depth = depth;

        emit depthChanged(_depth);
        emit wasModified();
    }
}

auto AbstractLocationOutput::type() const -> AbstractMotion::Type
{
    return _type;
}

void AbstractLocationOutput::setType(AbstractMotion::Type type)
{
    if (_type != type) {
        _type = type;

        emit typeChanged(_type);
        emit wasModified();
    }
}

void AbstractLocationOutput::setType(int type)
{
    setType((AbstractMotion::Type)type);
}

auto AbstractLocationOutput::fileName(int motion) const -> QString
{
    Q_UNUSED(motion)

    return prefix() + "-" + shortName();
}

auto AbstractLocationOutput::prefix() const -> const QString
{
    return QString("%1 (%3)")
            .arg(locationToString(_depth))
            .arg(AbstractMotion::typeList().at(_type));
}

void AbstractLocationOutput::fromJson(const QJsonObject &json)
{
    AbstractOutput::fromJson(json);
    _type = (AbstractMotion::Type) json["type"].toInt();
    _depth = json["depth"].toDouble();
}

auto AbstractLocationOutput::toJson() const -> QJsonObject
{
    QJsonObject json = AbstractOutput::toJson();
    json["type"] = (int) _type;
    json["depth"] = _depth;

    return json;
}


auto operator<< (QDataStream & out, const AbstractLocationOutput* alo) -> QDataStream &
{
    out << (quint8)1;

    out << static_cast<const AbstractOutput*>(alo)
            << (int)alo->_type
            << alo->_depth;

    return out;
}

auto operator>> (QDataStream & in, AbstractLocationOutput* alo) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    int type;
    in >> static_cast<AbstractOutput*>(alo)
            >> type
            >> alo->_depth;

    alo->_type = (AbstractMotion::Type)type;

    return in;
}
