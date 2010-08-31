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
// Copyright 2007 Albert Kottke
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

    const QString& soilName() const;
    virtual QString name() const;
    virtual QString fullName() const;

    virtual bool motionIndependent() const;

public slots:
    void setNonlinearProperty(NonlinearProperty* nonlinearProperty);
    void setSoilName(const QString &soilName);

signals:
    void soilNameChanged(const QString& soilName);

protected:
    virtual QString fileName(int motion = 0) const;
    virtual QString shortName() const;

    virtual QwtScaleEngine* xScaleEngine() const;
    virtual QwtScaleEngine* yScaleEngine() const;
    virtual const QString xLabel() const;
    virtual const QString yLabel() const;
    virtual const QVector<double>& ref(int motion = 0) const;
    virtual const QString prefix() const;
    virtual const QString suffix() const;

    void extract(AbstractCalculator* const calculator,
                             QVector<double> & ref, QVector<double> & data) const;

    //! Name of the soil type associated with the nonlinear property
    QString m_soilName;

    //! Nonlinear property to save
    NonlinearProperty* m_nonlinearProperty;
};
#endif // NONLINEAR_PROPERTY_OUTPUT_H_
