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

#ifndef EQUIVALENT_LINEAR_CALCULATOR_H
#define EQUIVALENT_LINEAR_CALCULATOR_H

#include "AbstractIterativeCalculator.h"

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

class EquivalentLinearCalculator : public AbstractIterativeCalculator
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out,
                                     const EquivalentLinearCalculator* elc);
    friend QDataStream & operator>> (QDataStream & in,
                                     EquivalentLinearCalculator* elc);

public:
    EquivalentLinearCalculator(QObject* parent = 0);

    virtual QString toHtml() const;
    double strainRatio() const;

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

signals:
    void strainRatioChanged(double strainRatio);

public slots:
    void setStrainRatio(double strainRatio);

protected:
    virtual bool updateSubLayer(int index, const QVector<std::complex<double> > strainTf);

    //! Ratio between the maximum strain and the strain of the layer
    double m_strainRatio;
};

#endif // EQUIVALENTLINEARCALCULATOR_H
