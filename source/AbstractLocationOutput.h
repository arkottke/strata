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

#ifndef ABSTRACT_LOCATION_OUTPUT_H
#define ABSTRACT_LOCATION_OUTPUT_H

#include "AbstractMotion.h"
#include "AbstractOutput.h"

#include <QDataStream>
#include <QJsonObject>

class AbstractLocationOutput : public AbstractOutput
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractLocationOutput* alo) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractLocationOutput* alo) -> QDataStream &;

public:
    explicit AbstractLocationOutput(OutputCatalog* catalog);

    auto needsOutputConditions() const -> bool;
    virtual auto fullName() const -> QString;

    auto depth() const -> double;
    auto type() const -> AbstractMotion::Type;
    void setType(AbstractMotion::Type type);

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setDepth(double depth);
    void setType(int type);

signals:
    void depthChanged(double depth);
    void typeChanged(int type);

protected:
    virtual auto fileName(int motion = 0) const -> QString;
    virtual auto prefix() const -> const QString;

    //! Depth of the response
    double  _depth;

    //! Type of the repsonse
    AbstractMotion::Type _type;
};
#endif // ABSTRACTLOCATIONOUTPUT_H
