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

#ifndef ABSTRACT_PROFILE_OUTPUT_H
#define ABSTRACT_PROFILE_OUTPUT_H

#include "AbstractOutput.h"

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

class OutputStatistics;
class OutputCatalog;

class AbstractProfileOutput : public AbstractOutput
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractProfileOutput* ao);
    friend QDataStream & operator>> (QDataStream & in, AbstractProfileOutput* ao);

public:
    enum CurveType{
        Yfx, //!< Y data is a function of x data
        Xfx  //!< X data is a function of y data
    };

    explicit AbstractProfileOutput(OutputCatalog* catalog, bool interpolated = true);

    virtual QString fullName() const;
    virtual bool needsDepth() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    virtual AbstractOutput::CurveType curveType() const;

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

protected:
    QString fileName(int motion = 0) const;
    virtual QwtScaleEngine* xScaleEngine() const;
    virtual QwtScaleEngine* yScaleEngine() const;
    virtual const QString yLabel() const;
    virtual const QVector<double>& ref(int motion = 0) const;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;

    //! Extrapolate the data -- used for the final point in a strain based profiles
    void extrap(const QVector<double> & ref, QVector<double> & data, double layerThickness) const;

    //! If the output is enabled
    bool m_enabled;
};

#endif // ABSTRACT_PROFILE_OUTPUT_H
