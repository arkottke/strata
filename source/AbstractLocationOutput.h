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

#ifndef ABSTRACT_LOCATION_OUTPUT_H
#define ABSTRACT_LOCATION_OUTPUT_H

#include "AbstractMotion.h"
#include "AbstractOutput.h"

#include <QDataStream>
#include <QJsonObject>

class AbstractLocationOutput : public AbstractOutput
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractLocationOutput* alo);
    friend QDataStream & operator>> (QDataStream & in, AbstractLocationOutput* alo);

public:
    explicit AbstractLocationOutput(OutputCatalog* catalog);

    bool needsOutputConditions() const;
    virtual QString fullName() const;

    double depth() const;
    AbstractMotion::Type type() const;
    void setType(AbstractMotion::Type type);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

public slots:
    void setDepth(double depth);
    void setType(int type);

signals:
    void depthChanged(double depth);
    void typeChanged(int type);

protected:
    virtual QString fileName(int motion = 0) const;
    virtual const QString prefix() const;

    //! Depth of the response
    double  m_depth;

    //! Type of the repsonse
    AbstractMotion::Type m_type;
};
#endif // ABSTRACTLOCATIONOUTPUT_H
