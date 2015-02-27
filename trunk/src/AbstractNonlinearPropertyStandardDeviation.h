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

#ifndef ABSTRACT_STANDARD_DEVIATION_H
#define ABSTRACT_STANDARD_DEVIATION_H

#include <QObject>

#include "NonlinearPropertyRandomizer.h"

#include <QScriptEngine>

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;


class AbstractNonlinearPropertyStandardDeviation : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractNonlinearPropertyStandardDeviation* anpsd);
    friend QDataStream & operator>> (QDataStream & in, AbstractNonlinearPropertyStandardDeviation* anpsd);

public:
    explicit AbstractNonlinearPropertyStandardDeviation(QObject *parent = 0);

    double min() const;
    double max() const;
    const QString& function() const;
    bool okSyntax() const;

    //! Compute the standard deviation
    virtual double calculate(NonlinearPropertyRandomizer::Model model, double strain, double property);

    //! Limit the value to the minimum and maximum
    double limit(double value) const;

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

signals:
    void minChanged(double min);
    void maxChanged(double max);
    void functionChanged(const QString& function);
    void canEvaluateChanged(bool canEvaulate);

    void wasModified();

public slots:
    void setMin(double min);
    void setMax(double max);
    void setFunction(const QString& function);

    virtual void setModel(NonlinearPropertyRandomizer::Model model) = 0;

protected:
    virtual void setPropertyValue(double value) = 0;

    //! Minimum value
    double m_min;

    //! Maximum value
    double m_max;

    //! Functional form
    QString m_function;

    //! Script engine used to evaluate the custom standard deviation definitions
    QScriptEngine m_engine;
};
#endif // ABSTRACT_STANDARD_DEVIATION_H
