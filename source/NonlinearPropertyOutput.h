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

#ifndef NONLINEAR_PROPERTY_OUTPUT_H_
#define NONLINEAR_PROPERTY_OUTPUT_H_

#include "AbstractOutput.h"

class AbstractCalculator;
class NonlinearProperty;
class OutputCatalog;
class OutputStatistics;

class NonlinearPropertyOutput : public AbstractOutput
{
    Q_OBJECT

public:
    NonlinearPropertyOutput(NonlinearProperty* nonlinearProperty, OutputCatalog* catalog);

    auto soilName() const -> const QString&;
    virtual auto name() const -> QString;
    virtual auto fullName() const -> QString;

    virtual auto motionIndependent() const -> bool;

public slots:
    void setNonlinearProperty(NonlinearProperty* nonlinearProperty);
    void setSoilName(const QString &soilName);

signals:
    void soilNameChanged(const QString& soilName);

protected:
    virtual auto fileName(int motion = 0) const -> QString;
    virtual auto shortName() const -> QString;

    virtual auto xScaleEngine() const -> QwtScaleEngine*;
    virtual auto yScaleEngine() const -> QwtScaleEngine*;
    virtual auto xLabel() const -> const QString;
    virtual auto yLabel() const -> const QString;
    virtual auto ref(int motion = 0) const -> const QVector<double>&;
    virtual auto prefix() const -> const QString;
    virtual auto suffix() const -> const QString;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;

    //! Name of the soil type associated with the nonlinear property
    QString _soilName;

    //! Nonlinear property to save
    NonlinearProperty* _nonlinearProperty;
};
#endif // NONLINEAR_PROPERTY_OUTPUT_H_
