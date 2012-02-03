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

#ifndef ABSTRACT_RATIO_OUTPUT_H
#define ABSTRACT_RATIO_OUTPUT_H

#include "AbstractOutput.h"
#include "AbstractMotion.h"

class OutputStatistics;

class AbstractRatioOutput : public AbstractOutput
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractRatioOutput* aro);
    friend QDataStream & operator>> (QDataStream & in, AbstractRatioOutput* aro);

public:
    explicit AbstractRatioOutput(OutputCatalog* catalog);

    virtual QString fullName() const;

    double inDepth() const;
    AbstractMotion::Type inType() const;
    void setInType(AbstractMotion::Type inType);

    double outDepth() const;
    AbstractMotion::Type outType() const;
    void setOutType(AbstractMotion::Type outType);

public slots:
    void setInDepth(double inDepth);
    void setInType(int inType);

    void setOutDepth(double outDepth);
    void setOutType(int outType);

signals:
    void inDepthChanged(double inDepth);
    void inTypeChanged(int inType);

    void outDepthChanged(double outDepth);
    void outTypeChanged(int outType);

protected:
    virtual QString fileName(int motion = 0) const;
    virtual const QString prefix() const;

    //! Input depth
    double  m_inDepth;

    //! Input type
    AbstractMotion::Type m_inType;

    //! Output depth
    double  m_outDepth;

    //! Output type
    AbstractMotion::Type m_outType;
};

#endif // ABSTRACT_RATIO_OUTPUT_H
