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

#ifndef SOIL_PROFILE_H_
#define SOIL_PROFILE_H_

#include "MyAbstractTableModel.h"

#include "Location.h"

#include <QDataStream>
#include <QJsonObject>

#include <gsl/gsl_rng.h>

class NonlinearPropertyRandomizer;
class ProfileRandomizer;
class RockLayer;
class SiteResponseModel;
class SoilLayer;
class SoilType;
class SoilTypeCatalog;
class SubLayer;
class TextLog;
class VelocityLayer;

class SoilProfile : public MyAbstractTableModel
{
    Q_OBJECT 

    friend auto operator<< (QDataStream & out, const SoilProfile* siteProfile) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SoilProfile* siteProfile) -> QDataStream &;

public:
    explicit SoilProfile(SiteResponseModel *parent = nullptr);
    ~SoilProfile();

    //! Columns of the table
    enum Column {
        DepthColumn,
        ThicknessColumn,
        SoilTypeColumn,
        VelocityColumn,
        StdevColumn,
        MinColumn,
        MaxColumn,
        VariedColumn
    };

    //! Methods to locating representative soil layer
    enum LayerSelectionMethod {
        MaximumTravelTime,
        MidDepth,
    };

    //!@{
    auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int;

    auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant;
    auto setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) -> bool;

    auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant;
    auto flags(const QModelIndex &index ) const -> Qt::ItemFlags;

    auto insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    //!@}

    auto soilLayers() -> QList<SoilLayer*> &;
    auto subLayers() -> QList<SubLayer> &;
    auto bedrock() -> RockLayer *;

    auto isVaried() const -> bool;

    auto inputDepth() const -> double;

    auto inputLocation() const -> const Location &;

    /*! Compute the layer index associated with a depth.
         * \param depth depth in the site profile
         * \return Location corresponding to the depth.
         */
    auto depthToLocation(double depth) const -> const Location;

    auto soilLayerNameList() const -> QStringList;

    auto profileCount() const -> int;
    auto onlyConverged() const -> bool;

    auto profileRandomizer() -> ProfileRandomizer*;
    auto nonlinearPropertyRandomizer() -> NonlinearPropertyRandomizer*;

    auto soilTypeCatalog() -> SoilTypeCatalog*;

    auto waterTableDepth() const -> double;

    auto maxFreq() const -> double;
    auto waveFraction() const -> double;
    auto disableAutoDiscretization() const -> bool;

    /*! Insert a new soil type and listen to its wasModified() signal.
         * @param row location of new SoilType
         */
    void insertSoilType( int index = 0 );

    //! Create the sublayers for a given realization
    void createSubLayers(TextLog * textLog);

    //! Reset the properties of the SubLayers to their initial properties
    void resetSubLayers();

    auto subLayerCount() const -> int;

    /*! @name Convience accessors
         * The following accessors allow for the SubLayers and Bedrock to be
         * accessed using the same functions.
         */
    //@{
    auto untWt( int layer ) const -> double;
    auto density( int layer ) const -> double;
    auto shearVel( int layer ) const -> double;
    auto shearMod( int layer) const -> double;
    auto damping( int layer ) const -> double;
    //@}

    //! Return the shear-wave velocity of the sublayers and bedrock
    auto depthProfile() const -> QVector<double>;
    auto depthToMidProfile() const -> QVector<double>;
    auto initialVelocityProfile() const -> QVector<double>;
    auto finalVelocityProfile() const -> QVector<double>;
    auto modulusProfile() const -> QVector<double>;
    auto dampingProfile() const -> QVector<double>;
    auto vTotalStressProfile() const -> QVector<double>;
    auto vEffectiveStressProfile() const -> QVector<double>;

    /*! Profile of the stress reduction coefficient (r_d).
         * The stress reduction coefficient relates the maximum shear stress of
         * a rigid block to the maximum shear stress in the deformable soil.
         * \param pga peak ground acceleration in g
         * \return a list of cofficients at the top of each layer.
         */
    auto stressReducCoeffProfile(double pga) const -> QVector<double>;
    auto maxShearStrainProfile() const -> QVector<double>;
    auto shearStressProfile() const -> QVector<double>;
    auto stressRatioProfile() const -> QVector<double>;
    auto maxErrorProfile() const -> QVector<double>;

    //! Create a table of the sublayers
    auto subLayerTable() const -> QString;

    //! Create a html document containing the information of the model
    auto toHtml() const -> QString;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setMaxFreq(double maxFreq);
    void setProfileCount(int count);
    void setOnlyConverged(bool onlyConverged);
    void setIsVaried(bool isVaried);
    void setInputDepth(double depth);
    void setWaveFraction(double waveFraction);
    void setDisableAutoDiscretization(bool disableAutoDiscretization);
    void setWaterTableDepth(double waterTableDepth);

    //! Refresh depths of the layers
    void updateDepths();

signals:
    void maxFreqChanged(double maxFreq);
    void profileCountChanged(int profileCount);
    void onlyConvergedChanged(double onlyConverged);
    void isVariedChanged(bool isVaried);
    void inputDepthChanged(double depth);
    void waveFractionChanged(double waveFraction);
    void disableAutoDiscretizationChanged(bool disableAutoDiscretization);

    void waterTableDepthChanged(double waterTableDepth);

    void soilTypesChanged();
    void soilLayersChanged();
    void depthsChanged();

    void wasModified();

protected slots:
    void updateUnits();

private:
    //! Return the layer with the longest travel time between the two depths
    auto createRepresentativeSoilLayer( double top, double base) -> SoilLayer*;

    /*! Return the velocity layer at a given index.
     * Combines both the soil layers and rock layer
     */
    auto velocityLayer(int index) const -> VelocityLayer*;

    /*
         * A site profile consists of a hierarchy of three different layers
         * SoilTypes -> VelocityLayers -> SubLayers
         * The SubLayers are what matters for the site response calculation
         * phase.  The ordering of soil layers and velocity layers is used to
         * facilitate the varying of the soil properties.
         */
    QList<SoilLayer*> _soilLayers;
    QList<SubLayer> _subLayers;

    //! Parent site response model
    SiteResponseModel* _siteResponseModel;

    //! Catalog of defined soil types
    SoilTypeCatalog* _soilTypeCatalog;

    //! Bedrock layer in the model
    RockLayer* _bedrock;

    //! Water table depth
    double _waterTableDepth;

    /*! @name Input motion specification
     */
    //{@
    //! Location where the motion is applied
    /*! The motions are applied at a certain depth and propagated to the
         * surface.  If the depth is less than zero then the depth of the
         * bedrock is used.  The user defines the depth of the input location.
         */
    double _inputDepth;

    //! Index associated with the location
    Location _inputLocation;
    //@}

    /*! @name Variation parameter
        */
    //{@
    //! Variation of the velocity profile
    ProfileRandomizer* _profileRandomizer;

    //! Variation of the soil properites
    NonlinearPropertyRandomizer* _nonlinearPropertyRandomizer;

    //! If the site is varied
    bool _isVaried;

    //! Number of artificial profiles to generate
    int _profileCount;

    //! Method used to find representative soillayer
    LayerSelectionMethod _layerSelectionMethod;

    //! Maximum tolerable error in randomization
    bool _onlyConverged;

    //! Random number generator
    gsl_rng* _rng;
    //@}

    /*! @name Layer discretization parameters
        */
    //@{
    //! Maximum frequency of interest
    double _maxFreq;

    //! Wavelength fraction
    double _waveFraction;

    //! Disable the layer discretization and use the layering provided
    bool _disableAutoDiscretization;
    //@}
};
#endif
