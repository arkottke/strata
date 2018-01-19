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

// FIXME this class if very similar to the distribution class maybe these two can be combined

#ifndef VELOCITY_LAYER_H_
#define VELOCITY_LAYER_H_

#include "AbstractDistribution.h"

#include <QDataStream>
#include <QJsonObject>
#include <QStringList>

//! A virtual class that describes the shear-wave velocity of a specific layer

class VelocityLayer : public AbstractDistribution
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const VelocityLayer* vl);
    friend QDataStream & operator>> (QDataStream & in, VelocityLayer* vl);

public:
    explicit VelocityLayer(QObject *parent = nullptr);
    virtual ~VelocityLayer() = 0;
    
    double depth() const;

    //! Randomized shear-wave velocity
    double shearVel() const;

    //! Randomized shear modulus
    double shearMod() const;

    virtual double untWt() const = 0;
    virtual double density() const = 0;

    bool isVaried() const;

    //! A description of the layer for tables
    virtual QString toString() const = 0;

    //! Vary the shear-wave velocity
    void vary(double randVar);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void depthChanged(double depth);
    void isVariedChanged(bool isVaried);

public slots:
    void setDepth(double depth);
    void setIsVaried(bool isVaried);

protected:
    //! If the shear-wave velocity is varied with randomization
    bool _isVaried;

    //! Depth to the top of the layer
    double _depth;
};
#endif 
