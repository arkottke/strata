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

#ifndef ABSTRACT_ITERATIVE_CALCULATOR_H
#define ABSTRACT_ITERATIVE_CALCULATOR_H

#include "AbstractCalculator.h"

#include <QDataStream>
#include <QJsonObject>

class AbstractIterativeCalculator : public AbstractCalculator
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractIterativeCalculator* aic);
    friend QDataStream & operator>> (QDataStream & in, AbstractIterativeCalculator* aic);

public:
    explicit AbstractIterativeCalculator(QObject* parent = 0);

    //! Perform the site response calculation
    virtual bool run(AbstractMotion* motion, SoilProfile* site);

    int maxIterations() const;
    double errorTolerance() const;
    bool converged() const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void maxIterationsChanged(int maxIterations);
    void errorToleranceChanged(double errorTolerance);

public slots:
    void setMaxIterations(int maxIterations);
    void setErrorTolerance(double errorTolerance);

protected:
    //! Compute the nonlinear properties
    virtual bool updateSubLayer(int index, const QVector<std::complex<double> > strainTf) = 0;

    //! Compute the relative error between two values
    static inline double relError(double value, double reference);

    //! Compute the maximum error of the maximum shear strain
    double maxError(const QVector<double> & maxStrain);

    //! Set initial strains of the layers
    void setInitialStrains(bool estimateStrain);

    //! Maximum number of iterations in the equivalent linear loop
    int m_maxIterations;

    //! Error tolerance of the equivalent linear loop -- percent
    double m_errorTolerance;

    //! Previous maximum strain
    QVector<double> m_prevMaxStrain;

    //! If the error tolerance was achieved
    bool m_converged;
};

#endif // ABSTRACT_ITERATIVE_CALCULATOR_H
