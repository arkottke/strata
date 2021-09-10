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

    friend auto operator<< (QDataStream & out, const VelocityLayer* vl) -> QDataStream &;
    friend auto operator>> (QDataStream & in, VelocityLayer* vl) -> QDataStream &;

public:
    explicit VelocityLayer(QObject *parent = nullptr);
    virtual ~VelocityLayer() = 0;
    
    auto depth() const -> double;

    //! Randomized shear-wave velocity
    auto shearVel() const -> double;

    //! Randomized shear modulus
    auto shearMod() const -> double;

    virtual auto untWt() const -> double = 0;
    virtual auto density() const -> double = 0;

    auto isVaried() const -> bool;

    //! A description of the layer for tables
    virtual auto toString() const -> QString = 0;

    //! Vary the shear-wave velocity
    void vary(double randVar);

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

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
