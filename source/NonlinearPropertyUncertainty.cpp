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

#include "NonlinearPropertyUncertainty.h"

#include <QDebug>

#include <QtAlgorithms>

#include <cfloat>
#include <cmath>

NonlinearPropertyUncertainty::NonlinearPropertyUncertainty(double lnStdev,
                                                           double min,
                                                           double max,
                                                           QObject *parent) :
    QObject(parent), _min(min), _max(max), _lnStdev(lnStdev)
{
}

void NonlinearPropertyUncertainty::vary(NonlinearPropertyRandomizer::Model model, NonlinearProperty* nlProperty, const double rand) const
{
    QVector<double> varied(nlProperty->strain().size());
    double avg;
    if (nlProperty->type() == NonlinearProperty::ModulusReduction) {
        // G/Gmax
        if (model == NonlinearPropertyRandomizer::SPID) {
            // SPID

            // While this is referred to as the SPID approach, it is best described
            // in PNNL (2014) by Coppersmith et al.

            // Translate the standard deviation to the transformed space. Instead of contraining the standard deviation
            // at a specific strain, the standard deviation is constrained at G/Gmax of 0.5.
            // This is modified from Equation 9.44 of PNNL (2014).
            const double f_std = _lnStdev * (1 / (1 - 0.5));
            double f_avg;
            double f_varied;
            for (int i = 0; i < varied.size(); ++i) {
                avg = nlProperty->average().at(i);
                // Vary the G/Gmax in transformed space.
                // Equation 9.43 of PNNL (2014)
                f_avg = avg / (1 - avg);
                f_varied = exp(rand * f_std) * f_avg;
                // Convert back to normal space
                // Equation 9.45 of PNNL (2014)
                varied[i] = f_varied / (1 + f_varied);
            }
        } else {
            // Darendeli
            double stdev;
            for (int i = 0; i < varied.size(); ++i) {
                avg = nlProperty->average().at(i);
                stdev = exp(-4.23) + sqrt(0.25 / exp(3.62) - pow(avg - 0.5, 2) / exp(3.62));
                varied[i] = limit(stdev * rand + avg);
            }
        }
    } else {
        // Damping
        for (int i = 0; i < varied.size(); ++i) {
            varied[i] = variedDamping(model, nlProperty->average().at(i), rand);
        }
    }
    nlProperty->setVaried(varied);
}

double NonlinearPropertyUncertainty::variedDamping(NonlinearPropertyRandomizer::Model model, const double average, const double rand) const
{
    double varied = 0;
    if (model == NonlinearPropertyRandomizer::SPID) {
        // SPID
        varied = exp(rand * _lnStdev) * average;
    } else {
        // Darendeli
        const double stdev = exp(-5) + exp(-0.25) * sqrt(average);
        varied = limit(stdev * rand + average);
    }
    return varied;
}

double NonlinearPropertyUncertainty::limit(double value) const
{
    return qBound(_min, value, _max);
}

double NonlinearPropertyUncertainty::min() const
{
    return _min;
}

double NonlinearPropertyUncertainty::max() const
{
    return _max;
}

double NonlinearPropertyUncertainty::lnStdev() const
{
    return _lnStdev;
}

void NonlinearPropertyUncertainty::setMin(double min)
{
    if (fabs(_min - min) > DBL_EPSILON) {
        _min = min;
        emit minChanged(_min);
        emit wasModified();
    }
}

void NonlinearPropertyUncertainty::setMax(double max)
{
    if (fabs(_max - max) > DBL_EPSILON) {
        _max = max;
        emit maxChanged(_max);
        emit wasModified();
    }
}

void NonlinearPropertyUncertainty::setRange(double min, double max)
{
    setMin(min);
    setMax(max);
}

void NonlinearPropertyUncertainty::setLnStdev(double lnStdev)
{
    if (fabs(_lnStdev - lnStdev) > DBL_EPSILON) {
        _lnStdev = lnStdev;
        emit maxChanged(lnStdev);
        emit wasModified();
    }
}

void NonlinearPropertyUncertainty::fromJson(const QJsonObject &json)
{
    _min = json["min"].toDouble();
    _max = json["max"].toDouble();
    _lnStdev = json["lnStdev"].toDouble();
}

QJsonObject NonlinearPropertyUncertainty::toJson() const
{
    QJsonObject json;
    json["min"] = _min;
    json["max"] = _max;
    json["lnStdev"] = _lnStdev;
    return json;
}


QDataStream& operator<< (QDataStream & out, const NonlinearPropertyUncertainty* npu)
{
    out << (quint8)1;
    out << npu->_min << npu->_max <<  npu->_lnStdev;
    return out;
}

QDataStream& operator>> (QDataStream & in, NonlinearPropertyUncertainty* npu)
{
    quint8 ver;
    in >> ver;
    in >> npu->_min >> npu->_max >> npu->_lnStdev;
    return in;
}
