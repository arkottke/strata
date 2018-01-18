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

#ifndef NONLINEAR_PROPERTY_UNCERTAINTY_H
#define NONLINEAR_PROPERTY_UNCERTAINTY_H

#include <QObject>

#include "NonlinearProperty.h"
#include "NonlinearPropertyRandomizer.h"

class NonlinearPropertyUncertainty : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const NonlinearPropertyUncertainty* npu);
    friend QDataStream & operator>> (QDataStream & in, NonlinearPropertyUncertainty* npu);

public:
    explicit NonlinearPropertyUncertainty(double lnStdev, double min, double max, QObject *parent = 0);

    double min() const;
    double max() const;
    double lnStdev() const;

    //! Vary the nonlinear parameter
    void vary(NonlinearPropertyRandomizer::Model model, NonlinearProperty* nlProperty, const double rand) const;

    //! Vary a damping value
    double variedDamping(NonlinearPropertyRandomizer::Model model, const double average, const double rand) const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void minChanged(double min);
    void maxChanged(double max);
    void lnStdevChanged(double lnStdev);
    void wasModified();

public slots:
    void setRange(double min, double max);
    void setMin(double min);
    void setMax(double max);
    void setLnStdev(double lnStdev);

protected:
    //! Limit the value to the minimum and maximum
    double limit(double value) const;

    //! Minimum value
    double _min;

    //! Maximum value
    double _max;

    //! Logarithmic standard deviation for the SPID model
    double _lnStdev;
};

#endif // NONLINEAR_PROPERTY_UNCERTAINTY_H
