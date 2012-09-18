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

#ifndef SOIL_PROFILE_H_
#define SOIL_PROFILE_H_

#include "MyAbstractTableModel.h"

#include "Location.h"

#include <gsl/gsl_rng.h>

class NonlinearPropertyRandomizer;
class ProfileRandomizer;
class RockLayer;
class SoilType;
class SoilLayer;
class SoilTypeCatalog;
class SubLayer;
class TextLog;
class VelocityLayer;

class SoilProfile : public MyAbstractTableModel
{
    Q_OBJECT 

    friend QDataStream & operator<< (QDataStream & out, const SoilProfile* siteProfile);
    friend QDataStream & operator>> (QDataStream & in, SoilProfile* siteProfile);

public:
    SoilProfile(QObject * parent = 0);
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
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index ) const;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!@}

    QList<SoilLayer*> & soilLayers();
    QList<SubLayer> & subLayers();
    RockLayer * bedrock();

    bool isVaried() const;

    double inputDepth() const;

    const Location & inputLocation() const;

    /*! Compute the layer index associated with a depth.
         * \param depth depth in the site profile
         * \return Location corresponding to the depth.
         */
    const Location depthToLocation(const double depth) const;

    QStringList soilLayerNameList() const;

    int profileCount() const;

    ProfileRandomizer* profileRandomizer();
    NonlinearPropertyRandomizer* nonlinearPropertyRandomizer();

    SoilTypeCatalog* soilTypeCatalog();

    double waterTableDepth() const;

    double maxFreq() const;
    double waveFraction() const;
    bool disableAutoDiscretization() const;

    /*! Insert a new soil type and listen to its wasModified() signal.
         * @param row location of new SoilType
         */
    void insertSoilType( int index = 0 );

    //! Create the sublayers for a given realization
    void createSubLayers(TextLog * textLog);

    //! Reset the properties of the SubLayers to their initial properties
    void resetSubLayers();

    int subLayerCount() const;

    /*! @name Convience accessors
         * The following accessors allow for the SubLayers and Bedrock to be
         * accessed using the same functions.
         */
    //@{
    double untWt( int layer ) const;
    double density( int layer ) const;
    double shearVel( int layer ) const;
    double shearMod( int layer) const;
    double damping( int layer ) const;
    //@}

    //! Return the shear-wave velocity of the sublayers and bedrock
    QVector<double> depthProfile() const;
    QVector<double> depthToMidProfile() const;
    QVector<double> initialVelocityProfile() const;
    QVector<double> finalVelocityProfile() const;
    QVector<double> modulusProfile() const;
    QVector<double> dampingProfile() const;
    QVector<double> vTotalStressProfile() const;
    QVector<double> vEffectiveStressProfile() const;

    /*! Profile of the stress reduction coefficient (r_d).
         * The stress reduction coefficient relates the maximum shear stress of
         * a rigid block to the maximum shear stress in the deformable soil.
         * \param pga peak ground acceleration in g
         * \return a list of cofficients at the top of each layer.
         */
    QVector<double> stressReducCoeffProfile(const double pga ) const;
    QVector<double> maxShearStrainProfile() const;
    QVector<double> shearStressProfile() const;
    QVector<double> stressRatioProfile() const;
    QVector<double> maxErrorProfile() const;

    QMap<QString, QVariant> toMap() const;
    void fromMap( const QMap<QString, QVariant> & map );

    //! Create a table of the sublayers
    QString subLayerTable() const;

    //! Create a html document containing the information of the model
    QString toHtml() const;

public slots:
    void setMaxFreq(double maxFreq);
    void setProfileCount(int count);
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
    SoilLayer* createRepresentativeSoilLayer( double top, double base);

    /*! Return the velocity layer at a given index.
     * Combines both the soil layers and rock layer
     */
    VelocityLayer* velocityLayer(int index) const;

    /*
         * A site profile consists of a hierarchy of three different layers
         * SoilTypes -> VelocityLayers -> SubLayers
         * The SubLayers are what matters for the site response calculation
         * phase.  The ordering of soil layers and velocity layers is used to
         * facilitate the varying of the soil properties.
         */
    QList<SoilLayer*> m_soilLayers;
    QList<SubLayer> m_subLayers;

    //! Catalog of defined soil types
    SoilTypeCatalog* m_soilTypeCatalog;

    //! Bedrock layer in the model
    RockLayer* m_bedrock;

    //! Water table depth
    double m_waterTableDepth;

    /*! @name Input motion specification
     */
    //{@
    //! Location where the motion is applied
    /*! The motions are applied at a certain depth and propagated to the
         * surface.  If the depth is less than zero then the depth of the
         * bedrock is used.  The user defines the depth of the input location.
         */
    double m_inputDepth;

    //! Index associated with the location
    Location m_inputLocation;
    //@}

    /*! @name Variation parameter
        */
    //{@
    //! Variation of the velocity profile
    ProfileRandomizer* m_profileRandomizer;

    //! Variation of the soil properites
    NonlinearPropertyRandomizer* m_nonlinearPropertyRandomizer;

    //! If the site is varied
    bool m_isVaried;

    //! Number of artificial profiles to generate
    int m_profileCount;

    //! Method used to find representative soillayer
    LayerSelectionMethod m_layerSelectionMethod;

    //! Random number generator
    gsl_rng* m_rng;
    //@}

    /*! @name Layer discretization parameters
        */
    //@{
    //! Maximum frequency of interest
    double m_maxFreq;

    //! Wavelength fraction
    double m_waveFraction;

    //! Disable the layer discretization and use the layering provided
    bool m_disableAutoDiscretization;
    //@}
};
#endif
