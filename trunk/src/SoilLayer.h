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
#include "SoilType.h"

//! Describes the velocity variation and nonlinear response of soil

class SoilLayer : public VelocityLayer
{
    friend class SiteProfile;

    public:
        SoilLayer();
        SoilLayer( const SoilLayer & soilLayer);

        SoilType * soilType() const;
        void setSoilType(SoilType * soilType);
        
        double thickness() const;

        double depthToBase() const;

        QString toString() const;

        double untWt() const;
        double density() const;
        
        QMap<QString, QVariant> toMap() const;
        void fromMap(const QMap<QString, QVariant>& map);

    protected:
        void setThickness(double thickness);
        
    private:
        //! Soil type of the layer
        SoilType * m_soilType;
        
        //! Total thickness of the layer
        double m_thickness;
};
#endif
