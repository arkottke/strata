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

#ifndef SOIL_LAYER_H_
#define SOIL_LAYER_H_

#include "VelocityLayer.h"

#include <QPointer>

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

class SoilTypeCatalog;
class SoilType;

//! Describes the velocity variation and nonlinear response of soil

class SoilLayer : public VelocityLayer
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const SoilLayer* sl);
    friend QDataStream & operator>> (QDataStream & in, SoilLayer* sl);
    friend class SoilProfile;

public:
    SoilLayer(QObject* parent = 0);
    SoilLayer(const SoilLayer* soilLayer);

    SoilType* soilType() const;
    void setSoilType(SoilType* soilType);

    double thickness() const;

    double depthToBase() const;

    QString toString() const;

    double untWt() const;
    double density() const;

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

protected:
    void setThickness(double thickness);

private:
    //! Soil type of the layer
    QPointer<SoilType> m_soilType;

    //! Total thickness of the layer
    double m_thickness;
};
#endif
