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

#ifndef ABSTRACT_ITERATIVE_CALCULATOR_H
#define ABSTRACT_ITERATIVE_CALCULATOR_H

#include "AbstractCalculator.h"

#include <QDataStream>
#include <QJsonObject>

class AbstractIterativeCalculator : public AbstractCalculator
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractIterativeCalculator* aic) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractIterativeCalculator* aic) -> QDataStream &;

public:
    explicit AbstractIterativeCalculator(QObject *parent = nullptr);

    //! Perform the site response calculation
    virtual auto run(AbstractMotion* motion, SoilProfile* site) -> bool;

    auto maxIterations() const -> int;
    auto errorTolerance() const -> double;
    virtual auto converged() const -> bool;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

signals:
    void maxIterationsChanged(int maxIterations);
    void errorToleranceChanged(double errorTolerance);

public slots:
    void setMaxIterations(int maxIterations);
    void setErrorTolerance(double errorTolerance);

protected:
    //! Compute the nonlinear properties
    virtual auto updateSubLayer(int index, const QVector<std::complex<double> > &strainTf) -> bool = 0;

    //! Compute the relative error between two values
    static inline auto relError(double value, double reference) -> double;

    //! Compute the maximum error of the maximum shear strain
    auto maxError(const QVector<double> & maxStrain) -> double;

    //! Set initial strains of the layers
    virtual void estimateInitialStrains() = 0;

    //! Maximum number of iterations in the equivalent linear loop
    int _maxIterations;

    //! Error tolerance of the equivalent linear loop -- percent
    double _errorTolerance;

    //! Previous maximum strain
    QVector<double> _prevMaxStrain;

    //! If the error tolerance was achieved
    bool _converged;
    
    //! Name of calcuation stage
    QString _name;
};

#endif // ABSTRACT_ITERATIVE_CALCULATOR_H
