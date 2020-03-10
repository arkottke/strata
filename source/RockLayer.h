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

#ifndef ROCK_LAYER_H_
#define ROCK_LAYER_H_

#include "VelocityLayer.h"

#include <QDataStream>
#include <QJsonObject>
#include <QString>

//! Describes the velocity variation and linear response of bedrock

class RockLayer : public VelocityLayer
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const RockLayer* rl) -> QDataStream &;
    friend auto operator>> (QDataStream & in, RockLayer* rl) -> QDataStream &;

public:
    RockLayer(QObject *parent = nullptr);

    auto untWt() const -> double;
    auto density() const -> double;

    //! Randomized damping and strain dependent damping
    auto damping() const -> double;

    //! Set the varied damping and strain dependent damping
    void setDamping(double damping);

    //! Set the average damping only
    auto avgDamping() const -> double;

    //! A description of the layer for tables
    auto toString() const -> QString;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setUntWt(double untWt);
    //! Set both the average and varied damping
    void setAvgDamping(double damping);

signals:
    void avgDampingChanged(double damping);
    void untWtChanged(double untWt);

private:
    //! Unit weight of the rock
    double _untWt;

    //! Damping of the rock -- randomized
    double _damping;

    //! Damping of the rock -- average
    double _avgDamping;
};
#endif
