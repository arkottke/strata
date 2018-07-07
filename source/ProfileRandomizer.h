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

#ifndef PROFILE_RANDOMIZER_H_
#define PROFILE_RANDOMIZER_H_

#include <QObject>

#include <QDataStream>
#include <QJsonObject>
#include <QMap>
#include <QStringList>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include <gsl/gsl_rng.h>

class BedrockDepthVariation;
class LayerThicknessVariation;
class RockLayer;
class SoilProfile;
class SoilLayer;
class VelocityVariation;

/*! Variation of the layer thickness and shear-wave velocity.
 * The variation of the layer thickness and the shear-wave velocity of these
 * layers is varied using the Toro (1995) model.
 */

class ProfileRandomizer : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const ProfileRandomizer* pr);
    friend QDataStream & operator>> (QDataStream & in, ProfileRandomizer* pr);

public:
    ProfileRandomizer(gsl_rng * rng, SoilProfile* siteProfile);
    ~ProfileRandomizer();

    bool enabled() const;

    BedrockDepthVariation* bedrockDepthVariation();
    LayerThicknessVariation* layerThicknessVariation();
    VelocityVariation* velocityVariation();

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void enabledChanged(bool enabled);
    void wasModified();

public slots:
    void setEnabled(bool enabled);

protected slots:
    void updateEnabled();

private:
    //! Enable variation of the profile
    bool _enabled;

    //! Model for the shear-wave velocity variation
    VelocityVariation* _velocityVariation;

    //! Model for layering thickness
    LayerThicknessVariation* _layerThicknessVariation;

    //! Variation of the bedrock layer
    BedrockDepthVariation* _bedrockDepthVariation;

    //! Site profile
    SoilProfile* _siteProfile;
};
#endif
