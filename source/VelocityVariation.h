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

#ifndef VELOCITYVARIATION_H
#define VELOCITYVARIATION_H

#include <QObject>

#include <QDataStream>
#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <QTextStream>

#include <gsl/gsl_rng.h>

class ProfileRandomizer;
class SoilLayer;
class RockLayer;

class VelocityVariation : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const VelocityVariation* vv);
    friend QDataStream & operator>> (QDataStream & in, VelocityVariation* vv);

public:
    explicit VelocityVariation(gsl_rng* rng, ProfileRandomizer* profileRandomizer);

    enum Model{
        Custom,
        GeoMatrix_AB,
        GeoMatrix_CD,
        USGS_AB,
        USGS_CD,
        USGS_A,
        USGS_B,
        USGS_C,
        USGS_D };

    static QStringList modelList();

    //! Vary the shear-wave velocity of the layers
    void vary(QList<SoilLayer*>& soilLayers, RockLayer* bedrock) const;

    bool enabled() const;

    bool isBedrockDepthVaried() const;
    Model stdevModel() const;
    bool stdevCustomEnabled() const;
    void setStdevModel(Model model);
    bool stdevIsLayerSpecific() const;
    double stdev() const;

    Model correlModel() const;
    void setCorrelModel(Model model);
    bool correlCustomEnabled() const;
    double correlInitial() const;
    double correlFinal() const;
    double correlDelta() const;
    double correlIntercept() const;
    double correlExponent() const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void enabledChanged(bool enabled);
    void stdevIsLayerSpecificChanged(bool stdevIsLayerSpecific);
    void stdevChanged(double stdev);
    void stdevModelChanged(int model);
    void stdevCustomEnabledChanged(bool customEnabled);
    void correlModelChanged(int model);
    void correlCustomEnabledChanged(bool customEnabled);
    void correlInitialChanged(double correlInitial);
    void correlFinalChanged(double correlFinal);
    void correlDeltaChanged(double correlDelta);
    void correlInterceptChanged(double correlIntercept);
    void correlExponentChanged(double correlExponent);

    void wasModified();

public slots:
    void setEnabled(bool enabled);
    void setStdevIsLayerSpecific(bool stdevIsLayerSpecific);
    void setStdev(double stdev);
    void setStdevModel(int model);
    void setCorrelModel(int model);
    void setCorrelInitial(double correlInitial);
    void setCorrelFinal(double correlFinal);
    void setCorrelDelta(double correlDelta);
    void setCorrelIntercept(double correlIntercept);
    void setCorrelExponent(double correlExponent);

protected slots:
    void updateEnabled();

protected:
    //! If the variation is enabled
    bool _enabled;

    //! Model for the standard deviation
    Model _stdevModel;

    //! Allows the standard deviation to be defined at the layer
    bool _stdevIsLayerSpecific;

    //! Standard deviation of the entire site
    double _stdev;

    //! Model for the correlation
    Model _correlModel;

    //! Initial correlation (depth of 0 m)
    double _correlInitial;

    //! Final correlation  (depth of 200 m)
    double _correlFinal;

    //! Change in correlation with depth
    double _correlDelta;

    //! Depth intercept
    double _correlIntercept;

    //! Exponent of the correlation model
    double _correlExponent;

    //! Random number generator
    gsl_rng* _rng;

    //! Profile randomizer
    ProfileRandomizer* _profileRandomizer;
};
#endif // VELOCITYVARIATION_H
