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

#include "SoilProfile.h"

#include "Algorithms.h"
#include "BedrockDepthVariation.h"
#include "Location.h"
#include "LayerThicknessVariation.h"
#include "NonlinearPropertyRandomizer.h"
#include "ProfileRandomizer.h"
#include "RockLayer.h"
#include "SoilLayer.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"
#include "VelocityVariation.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariant>

#include <cmath>

SoilProfile::SoilProfile(QObject * parent)
    : MyAbstractTableModel(parent)
{
    // Allocate the random number generator with the "Mersenne Twister" algorithm
    m_rng = gsl_rng_alloc(gsl_rng_mt19937);

    // Initialize the seed of the generator using the time
    gsl_rng_set(m_rng, QDateTime().currentDateTime().toTime_t());

    m_bedrock = new RockLayer;
    connect( m_bedrock, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    m_profileRandomizer = new ProfileRandomizer(m_rng, this);
    connect( m_profileRandomizer, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    m_nonlinearPropertyRandomizer = new NonlinearPropertyRandomizer(m_rng, this);
    connect( m_nonlinearPropertyRandomizer, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    m_soilTypeCatalog = new SoilTypeCatalog;
    connect(m_soilTypeCatalog, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(wasModified()));

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(updateUnits()));


    m_isVaried = false;
    m_profileCount = 100;
    m_inputDepth = -1;
    m_maxFreq = 20;
    m_waveFraction = 0.20;
    m_disableAutoDiscretization = false;
    m_waterTableDepth = 0.;
}

SoilProfile::~SoilProfile()
{
    delete m_profileRandomizer;
    delete m_nonlinearPropertyRandomizer;
    delete m_bedrock;
    delete m_soilTypeCatalog;
    gsl_rng_free(m_rng);
}

int SoilProfile::rowCount(const QModelIndex &parent) const
{
   Q_UNUSED(parent);

   return m_soilLayers.size() + 1;
}

int SoilProfile::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 8;
}

QVariant SoilProfile::data(const QModelIndex &index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::BackgroundRole && !(flags(index) & Qt::ItemIsEditable))
        // Color the background light gray for cells that are not editable
        return QVariant(QBrush(QColor(200,200,200)));

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case DepthColumn:
            return QString::number(velocityLayer(index.row())->depth(), 'f', 2);
        case ThicknessColumn:
            if (index.row() < (rowCount() - 1)) {
                // Soil layers
                return QString::number(m_soilLayers.at(index.row())->thickness(), 'f', 2);
            } else {
                // Rock layer
                return tr("Half-Space");
            }
        case SoilTypeColumn:
            if (index.row() < (rowCount() - 1)) {
                // Soil layers
                SoilType* const st = m_soilLayers.at(index.row())->soilType();
                return st ? st->name() : "";
            } else {
                // Rock layer
                return tr("Bedrock");
            }
        case VelocityColumn:
            return QString::number(velocityLayer(index.row())->avg(), 'f', 2);
        case StdevColumn:
            return QString::number(velocityLayer(index.row())->stdev(), 'f', 2);
        case MinColumn:
            return QString::number(velocityLayer(index.row())->min(), 'f', 2);
        case MaxColumn:
            return QString::number(velocityLayer(index.row())->max(), 'f', 2);
        case VariedColumn:
        default:
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole) {
        switch (index.column()) {
        case MinColumn:
            return velocityLayer(index.row())->hasMin() ? Qt::Checked : Qt::Unchecked;
        case MaxColumn:
            return velocityLayer(index.row())->hasMax() ? Qt::Checked : Qt::Unchecked;
        case VariedColumn:
            return velocityLayer(index.row())->isVaried() ? Qt::Checked : Qt::Unchecked;
        }
    }

    return MyAbstractTableModel::data(index, role);
}

bool SoilProfile::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.parent()!=QModelIndex() || m_readOnly)
        return false;

    if (role == Qt::EditRole) {
        switch (index.column()) {
        case ThicknessColumn:
            if (index.row() < (rowCount() - 1)) {
                // Soil layers
                bool success;
                const double d = value.toDouble(&success);

                if (!success)
                    return false;

                m_soilLayers.at(index.row())->setThickness(d);
                updateDepths();
                emit dataChanged(this->index(index.row(), DepthColumn),
                                 this->index(rowCount(), DepthColumn));
                break;
            } else {
                // Rock layer
                return false;
            }
        case SoilTypeColumn:
            // Check if it is the final layer and if the SoilType can be identified
            if (index.row() < (rowCount() - 1)) {
                 if (SoilType* st = m_soilTypeCatalog->soilTypeOf(value)) {
                    m_soilLayers.at(index.row())->setSoilType(st);
                    break;
                }
            } else {
                return false;
            }
        case VelocityColumn:
        case StdevColumn:
                {
                    bool success;
                    const double d = value.toDouble(&success);

                    if (!success)
                        return false;

                    if (index.column() == VelocityColumn)
                        velocityLayer(index.row())->setAvg(d);
                    else if (index.column() == StdevColumn)
                        velocityLayer(index.row())->setStdev(d);
                    break;
                }
        case MinColumn:
        case MaxColumn:
                {
                    bool success;
                    const double d = value.toDouble(&success);

                    if (!success)
                        return false;

                    if (index.column() == MinColumn)
                        velocityLayer(index.row())->setMin(d);
                    else if(index.column() == MaxColumn)
                        velocityLayer(index.row())->setMax(d);
                    break;
                }
        case DepthColumn:
        case VariedColumn:
        default:
            return false;
        }
    } else if(role == Qt::CheckStateRole) {
        switch (index.column()) {
        case MinColumn:
            velocityLayer(index.row())->setHasMin(value.toBool());
            break;
        case MaxColumn:
            velocityLayer(index.row())->setHasMax(value.toBool());
            break;
        case VariedColumn:
            velocityLayer(index.row())->setIsVaried(value.toBool());
            break;
        }
    }

    emit dataChanged(index, index);
    return true;
}

QVariant SoilProfile::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case DepthColumn:
            return QString(tr("Depth (%1)")).arg(Units::instance()->length());
        case ThicknessColumn:
            return QString(tr("Thickness (%1)")).arg(Units::instance()->length());
        case SoilTypeColumn:
            return tr("Soil Type");
        case VelocityColumn:
            return QString(tr("Vs (%1)")).arg(Units::instance()->vel());
        case StdevColumn:
            return tr("Std. Dev.");
        case MinColumn:
            return QString(tr("Minimum (%1)")).arg(Units::instance()->vel());
        case MaxColumn:
            return QString(tr("Maximum (%1)")).arg(Units::instance()->vel());
        case VariedColumn:
            return tr("Varied");
        }
    case Qt::Vertical:
        return section+1;
                default:
        return QVariant();
    }
}

Qt::ItemFlags SoilProfile::flags(const QModelIndex &index ) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    switch (index.column()) {
    case ThicknessColumn:
    case SoilTypeColumn:
        if (index.row() == (rowCount() - 1))
            // Rock layer
            break;
    case VelocityColumn:
    case StdevColumn:   
        flags |= Qt::ItemIsEditable;
        break;
    case MinColumn:
    case MaxColumn:
    case VariedColumn:
        flags |= Qt::ItemIsUserCheckable;

        if ((index.column() == MinColumn && velocityLayer(index.row())->hasMin())
            || (index.column() == MaxColumn && velocityLayer(index.row())->hasMax()))
            flags |= Qt::ItemIsEditable;

        break;
    case DepthColumn:
    default:
        break;
    }

    return flags;
}

bool SoilProfile::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginInsertRows(parent, row, row+count-1);

    for (int i=0; i < count; ++i)
        m_soilLayers.insert(row, new SoilLayer(this));

    emit endInsertRows();
    return true;
}

bool SoilProfile::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginRemoveRows(parent, row, qMin(row + count - 1, m_soilLayers.size() - 1));

    for (int i=0; i < count; ++i) {
        if (m_soilLayers.isEmpty())
            break;

        m_soilLayers.takeAt(row)->deleteLater();       
    }
    emit endRemoveRows();

    // Update the depths of the layers.
    updateDepths();

    return true;
}

int SoilProfile::profileCount() const
{
    return m_profileCount;
}

void SoilProfile::setProfileCount(int count)
{
    if ( m_profileCount != count ) {
        m_profileCount = count;

        emit profileCountChanged(m_profileCount);
        emit wasModified();
    }
}

bool SoilProfile::isVaried() const
{
    return m_isVaried;
}

void SoilProfile::setIsVaried(bool isVaried)
{
    if ( m_isVaried != isVaried ) {
        m_isVaried = isVaried;

        emit wasModified();
        emit isVariedChanged(m_isVaried);
    }
}

QList<SoilLayer*> & SoilProfile::soilLayers()
{
    return m_soilLayers;
}

QList<SubLayer> & SoilProfile::subLayers()
{
    return m_subLayers;
}

RockLayer* SoilProfile::bedrock()
{
    return m_bedrock;
}

double SoilProfile::inputDepth() const
{
    return m_inputDepth;
}

void SoilProfile::setInputDepth(double depth)
{
    if ( m_inputDepth != depth ) {
        m_inputDepth = depth;

        emit wasModified();
        emit inputDepthChanged(m_inputDepth);
    }
}

const Location & SoilProfile::inputLocation() const
{
    return m_inputLocation;
}

const Location SoilProfile::depthToLocation(const double depth) const
{
    int index = 0;
    double interDepth = 0;

    if (depth < 0 || m_subLayers.last().depthToBase() <= depth) {
        // Use the surface of the bedrock
        index = m_subLayers.size();
        interDepth = 0;   
    } else {
        // Use the layer whose bottom depth is deeper
        index = 0;

        while (index <= m_subLayers.size()
                && m_subLayers.at(index).depthToBase() < depth)
            ++index;

        interDepth = depth - m_subLayers.at(index).depth();
    }

    return Location(index, interDepth);
}

ProfileRandomizer* SoilProfile::profileRandomizer()
{
    return m_profileRandomizer;
}

NonlinearPropertyRandomizer* SoilProfile::nonlinearPropertyRandomizer()
{
    return m_nonlinearPropertyRandomizer;
}

SoilTypeCatalog* SoilProfile::soilTypeCatalog()
{
    return m_soilTypeCatalog;
}

double SoilProfile::waterTableDepth() const
{
    return m_waterTableDepth;
}

void SoilProfile::setWaterTableDepth(double waterTableDepth)
{
    if (m_waterTableDepth != waterTableDepth) {
        m_waterTableDepth = waterTableDepth;

        emit wasModified();
        emit waterTableDepthChanged(waterTableDepth);
    }
}

double SoilProfile::maxFreq() const
{
    return m_maxFreq;
}

void SoilProfile::setMaxFreq(double maxFreq)
{
    if ( m_maxFreq != maxFreq ) {
        m_maxFreq = maxFreq;

        emit wasModified();
        emit maxFreqChanged(m_maxFreq);
    }
}

double SoilProfile::waveFraction() const
{
    return m_waveFraction;
}

void SoilProfile::setWaveFraction(double waveFraction)
{
    if ( m_waveFraction != waveFraction ) {
        m_waveFraction = waveFraction;

        emit wasModified();
        emit waveFractionChanged(m_waveFraction);
    }
}

bool SoilProfile::disableAutoDiscretization() const
{
    return m_disableAutoDiscretization;
}

void SoilProfile::setDisableAutoDiscretization(bool disableAutoDiscretization)
{
    if (m_disableAutoDiscretization != disableAutoDiscretization) {
        m_disableAutoDiscretization = disableAutoDiscretization;

        emit wasModified();
        emit disableAutoDiscretizationChanged(m_disableAutoDiscretization);
    }
}

QStringList SoilProfile::soilLayerNameList() const
{
    QStringList list;

    for (int i = 0; i < m_soilLayers.size(); ++i)
        list << QString("%1 %2").arg(i+1).arg(m_soilLayers.at(i)->toString());
    
    return list;
}

void SoilProfile::createSubLayers(TextLog * textLog)
{
    // Clear the previously generated sublayers and delete the soillayers if
    // they are not the used defined values.  This could be improved as it runs
    // regardless of it is really needed. FIXME
    if (!m_subLayers.isEmpty()) {
        QList<SoilLayer*> soilLayers;

        // Create a list of unique soilLayers
        foreach (SubLayer sl, m_subLayers) {
            if (!soilLayers.contains(sl.soilLayer()))
                soilLayers << sl.soilLayer();
        }

        // Delete the SoilLayers created in the process of randomizing the site
        foreach (SoilLayer* sl, soilLayers) {
            if (!m_soilLayers.contains(sl))
                delete sl;
        }

        // Clear the sublayers
        m_subLayers.clear();
    }

    // Vary the nonlinear properties of the SoilTypes
    if (m_nonlinearPropertyRandomizer->enabled()) {
        if (textLog->level() > TextLog::Low)
            textLog->append(QObject::tr("Varying dynamic properties of soil types"));

        // Vary the nonlinear properties of the soil types
        for (int i = 0; i < m_soilTypeCatalog->rowCount(); ++i) {
            SoilType * st = m_soilTypeCatalog->soilType(i);

            if (st->isVaried()) {
                // Vary the properties
                if (textLog->level() > TextLog::Low)
                    textLog->append(QString("\t%1").arg(st->name()));
                m_nonlinearPropertyRandomizer->vary(st);
            }
        }

        // Vary the damping of the bedrock
        if (m_profileRandomizer->bedrockDepthVariation()->enabled()) {
            if ( textLog->level() > TextLog::Low )
                textLog->append(QObject::tr("Varying damping of bedrock"));

            m_nonlinearPropertyRandomizer->vary(m_bedrock);
        }
    }
    
    // Vary the depth to the bedrock
    double depthToBedrock;

    if (m_profileRandomizer->bedrockDepthVariation()->enabled()) {
        if ( textLog->level() > TextLog::Low )
            textLog->append(QObject::tr("Varying depth to bedrock"));       

        m_profileRandomizer->bedrockDepthVariation()->setAvg(m_bedrock->depth());
        depthToBedrock = m_profileRandomizer->bedrockDepthVariation()->rand();
    } else {
        depthToBedrock = m_bedrock->depth();
    }

    // Vary the layering 
    QList<SoilLayer*> soilLayers;
    if (m_profileRandomizer->layerThicknessVariation()->enabled()) {        
        if ( textLog->level() > TextLog::Low ) {
            textLog->append(QObject::tr("Varying the layering"));
        }
        // Randomize the layer thicknesses
        QList<double> thickness =
                m_profileRandomizer->layerThicknessVariation()->vary(depthToBedrock);

        // For each thickness, determine the representative soil layer
        double depth = 0;
        for (int i = 0; i < thickness.size(); ++i ) {
            soilLayers << createRepresentativeSoilLayer(depth, depth+thickness.at(i));
            // Set the new depth and thickness
            soilLayers.last()->setDepth(depth);
            soilLayers.last()->setThickness(thickness.at(i));

            // Increment the depth
            depth += thickness.at(i);
        }
    } else {
        if (m_profileRandomizer->bedrockDepthVariation()->enabled()) {
            foreach (SoilLayer * layer, m_soilLayers) {
                soilLayers << layer;

                if (soilLayers.last()->depthToBase() > depthToBedrock) {
                    // Truncate the last SoilLayer once the depth to the bedrock is exceeded
                    double thickness = depthToBedrock - soilLayers.last()->depth();
                    soilLayers.last()->setThickness(thickness);
                    break;
                }
            }
        } else {
            // Copy over previous soil layers
            soilLayers = m_soilLayers;
        }
    }

    // Vary the shear-wave velocity
    if (m_profileRandomizer->velocityVariation()->enabled()) {
        if (textLog->level() > TextLog::Low )
            textLog->append(QObject::tr("Varying the shear-wave velocity"));

        m_profileRandomizer->velocityVariation()->vary(soilLayers, m_bedrock);
    } else {
        // Reset the values
        foreach (SoilLayer* sl, soilLayers)
            sl->reset();

        m_bedrock->reset();
    }

    /* Create the SubLayers by dividing the thicknesses.  The height of the
     * sublayer is determined by computing the shortest wavelength of interest
     * (Vs/freqMax) and then reducing this by the wavelength fraction.
     */
    double depth = 0;
    double vTotalStress = 0;

    if (m_disableAutoDiscretization) {
        foreach (SoilLayer* sl, soilLayers) {
            m_subLayers << SubLayer(sl->thickness(), depth, vTotalStress, m_waterTableDepth, sl);

            depth += sl->thickness();
            vTotalStress += sl->thickness() * sl->untWt();
        }
    } else {
        foreach (SoilLayer* sl, soilLayers) {
            // Compute the optimal thickness of the sublayers
            double optSubThickness = sl->shearVel() / m_maxFreq * m_waveFraction;
            // Compute the required number of sub-layers for this thickness
            int numSubLayers = int(ceil(sl->thickness() / optSubThickness));
            // The subThickness for an even number of layers
            double subThickness = sl->thickness() / numSubLayers;

            for (int j = 0; j < numSubLayers; ++j) {
                m_subLayers << SubLayer(subThickness, depth, vTotalStress, m_waterTableDepth, sl);
                // Increment the depth by the subThicknees
                depth += subThickness;
                // Compute the stress at the base of the layer and this to the total stress
                vTotalStress += subThickness * sl->untWt();
            }
        }
    }
    
    // Compute the SubLayer index associated with the input depth
    m_inputLocation = depthToLocation(m_inputDepth);
}

void SoilProfile::resetSubLayers()
{
    for ( int i = 0; i < m_subLayers.size(); ++i ) {
        m_subLayers[i].reset();
    }
}

int SoilProfile::subLayerCount() const
{
    return m_subLayers.size();
}

double SoilProfile::untWt( int layer ) const
{
    if ( layer < m_subLayers.size() ) {
        return m_subLayers.at(layer).untWt();
    } else {
        return m_bedrock->untWt();
    }
}

double SoilProfile::density( int layer ) const
{
    if ( layer < m_subLayers.size() ) {
        return m_subLayers.at(layer).density();
    } else {
        return m_bedrock->density();
    }
}

double SoilProfile::shearVel( int layer ) const
{
    if ( layer < m_subLayers.size() ) {
        return m_subLayers.at(layer).shearVel();
    } else {
        return m_bedrock->shearVel();
    }
}

double SoilProfile::shearMod(int layer) const
{
    if ( layer < m_subLayers.size() ) {
        return m_subLayers.at(layer).shearMod();
    } else {
        return m_bedrock->shearMod();
    }
}

double SoilProfile::damping( int layer ) const
{
    if ( layer < m_subLayers.size() ) {
        return m_subLayers.at(layer).damping();
    } else {
        return m_bedrock->damping();
    }
}

QVector<double> SoilProfile::depthProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).depth();

    profile << m_bedrock->depth();

    return profile;
}

QVector<double> SoilProfile::depthToMidProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).depthToMid();

    return profile;
}

QVector<double> SoilProfile::initialVelocityProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i) {
        profile << m_subLayers.at(i).initialShearVel();
    }

    profile << m_bedrock->shearVel();

    return profile;
}

QVector<double> SoilProfile::finalVelocityProfile() const
{
    QVector<double> profile;

    for (int i = 0; i < m_subLayers.size(); ++i) {
        profile << m_subLayers.at(i).shearVel();
    }

    profile << m_bedrock->shearVel();

    return profile;
}

QVector<double> SoilProfile::modulusProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).shearMod();

    profile << m_bedrock->shearMod();

    return profile;
}

QVector<double> SoilProfile::dampingProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).damping();

    profile << m_bedrock->damping();

    return profile;
}

QVector<double> SoilProfile::vTotalStressProfile() const
{
    QVector<double> profile;

    // Add value at surface
    profile << 0.;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).vTotalStress();

    return profile;
}

QVector<double> SoilProfile::vEffectiveStressProfile() const
{
    QVector<double> profile;

    // Add value at surface
    profile << 0.;

    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).vEffectiveStress();

    return profile;
}

QVector<double> SoilProfile::maxErrorProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).error();

    return profile;
}

QVector<double> SoilProfile::stressReducCoeffProfile(const double pga) const
{
    // The stress reduction cofficient is used to compare the maximum shear
    // stress of the soil to the maximum shear stress of a rigid block for a
    // given acceleration at the ground surface.  PGA needs to be in units of
    // g.
    QVector<double> profile;

    // The total weight of all of the soil layers above the current
    double totalWeight = 0;

    // Defined to be 1 at surface
    profile << 1.0;

    for (int i = 0; i < m_subLayers.size(); ++i) {
        // Add the half layer to the total weight 
        totalWeight += m_subLayers.at(i).untWt() *
                       m_subLayers.at(i).thickness() / 2.;

        const double rigidStress = totalWeight * pga;

        profile << m_subLayers.at(i).shearStress() / rigidStress;

        // Add the half layer to the total weight 
        totalWeight += m_subLayers.at(i).untWt() *
                       m_subLayers.at(i).thickness() / 2.;

    }

    return profile;
}

QVector<double> SoilProfile::maxShearStrainProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).maxStrain();

    return profile;
}

QVector<double> SoilProfile::shearStressProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).shearStress();

    return profile;
}

QVector<double> SoilProfile::stressRatioProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < m_subLayers.size(); ++i)
        profile << m_subLayers.at(i).stressRatio();

    return profile;
}

QString SoilProfile::subLayerTable() const
{
    QString html;
    
    html +=  QObject::tr(
            "<table >"
            "<tr>"
            "<th colspan=\"6\"</th>"
            "<th colspan=\"3\">Damping (%)</th>"
            "<th colspan=\"4\">Shear Modulus</th>"
            "</tr>"
            "<tr>"
            "<th>No.</th>"
            "<th>Soil Type</th>"
            "<th>Depth</th>"
            "<th>Thickness</th>"
            "<th>Max Strain (%)</th>"
            "<th>Eff. Strain (%)</th>"
            "<th>New</th>"
            "<th>Old</th>"
            "<th>Error (%)</th>"
            "<th>New</th>"
            "<th>Old</th>"
            "<th>Error (%)</th>"
            "<th>Norm.</th>"
            "</tr>");

    // Create each of the rows
    for (int i = 0; i < m_subLayers.size(); ++i)
        html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13</tr>")
        .arg(i+1)
        .arg(m_subLayers.at(i).soilTypeName())
        .arg(m_subLayers.at(i).depth(), 0, 'f', 2)
        .arg(m_subLayers.at(i).thickness(), 0, 'f', 2)
        .arg(m_subLayers.at(i).maxStrain(), 0, 'e', 2)
        .arg(m_subLayers.at(i).effStrain(), 0, 'e', 2)
        .arg(m_subLayers.at(i).damping(), 0, 'f', 2)
        .arg(m_subLayers.at(i).oldDamping(), 0, 'f', 2)
        .arg(m_subLayers.at(i).dampingError(), 0, 'f', 2)
        .arg(m_subLayers.at(i).shearMod(), 0, 'f', 0)
        .arg(m_subLayers.at(i).oldShearMod(), 0, 'f', 0)
        .arg(m_subLayers.at(i).shearModError(), 0, 'f', 2)
        .arg(m_subLayers.at(i).normShearMod(), 0, 'f', 3);

    // Bedrock layer
    html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13<td>%14</tr>")
            .arg(m_subLayers.size()+1)
            .arg(QObject::tr("Bedrock"))
            .arg(m_bedrock->depth(), 0, 'f', 2)
            .arg("--")
            .arg("--")
            .arg("--")
            .arg("--")
            .arg(m_bedrock->damping(), 0, 'f', 2)
            .arg("--")
            .arg("--")
            .arg(m_bedrock->shearMod(), 0, 'f', 0)
            .arg("--")
            .arg("--")
            .arg("--");

    // Complete the table
    html += "</table></p>";

    return html;
}

QString SoilProfile::toHtml() const
{
    // Requires that the HTML header is already established.

    // 
    // Soil Types
    //
    QString html = m_soilTypeCatalog->toHtml();

    html +=  QString(tr(
            "<li><a name=\"Bedrock\">Bedrock<a>"
            "<table border=\"0\">"
            "<tr><th>Unit weight:</th><td>%1 %2</td></tr>"
            "<tr><th>Damping:</th><td>%3</td></tr>"))
            .arg(m_bedrock->untWt())
            .arg(Units::instance()->untWt())
            .arg(m_bedrock->damping());


    if (m_isVaried)
        html += QString("<tr><th>Varied:</th><td>%1</td></tr>").arg(
                boolToString(m_bedrock->isVaried()));


    html += "</table></li></ol>";

    //
    // Soil Layers
    //
    html += tr("<li><a name=\"soil-layers\">Soil Layers</a>");

    // Table header
    html += QString(tr("<table border = \"1\">"
                       "<tr>"
                       "<th>Depth (%1)</th>"
                       "<th>Thickness (%1)</th>"
                       "<th>Soil Type</th>"
                       "<th>Average Vs (%2)</th>"
                       ))
            .arg(Units::instance()->length())
            .arg(Units::instance()->vel());

    if (m_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
        html += tr("<th>Stdev.</th>");

    if (m_profileRandomizer->velocityVariation()->enabled())
        html += QString(tr(
                "<th>Minimum Vs. (%1)</th>"
                "<th>Maximum Vs. (%1)</th>"
                "<th>Varied</th>"
                )).arg(Units::instance()->vel());

    html += "</tr>";

    // Table information
    for (int i = 0; i < m_soilLayers.size(); ++i ) {
        html += QString("<tr><td>%1</td><td>%2</td><td><a href=\"#%3\">%3</a></td><td>%4</td>")
                .arg(m_soilLayers.at(i)->depth())
                .arg(m_soilLayers.at(i)->thickness())
                .arg(m_soilLayers.at(i)->soilType()->name())
                .arg(m_soilLayers.at(i)->avg());

        if (m_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
            html += QString("<td>%1</td>").arg(m_soilLayers.at(i)->stdev());

        if (m_profileRandomizer->velocityVariation()->enabled())
            html += QString("<td>%1</td><td>%2</td><td>%3</td>")
            .arg(m_soilLayers.at(i)->min())
            .arg(m_soilLayers.at(i)->max())
            .arg(boolToString(m_soilLayers.at(i)->isVaried()));

        html += "</tr>";
    }

    // Bedrock layer
    html += QString("<tr><td>%1</td><td>---</td><td><a href=\"#Bedrock\">Bedrock</a></td><td>%2</td>")
            .arg(m_bedrock->depth())
            .arg(m_bedrock->avg());

    if (m_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
        html += QString("<td>%1</td>").arg(m_bedrock->stdev());

    if (m_profileRandomizer->velocityVariation()->enabled())
        html += QString("<td>%1</td><td>%2</td><td>%3</td>")
        .arg(m_bedrock->min())
        .arg(m_bedrock->max())
        .arg(boolToString(m_bedrock->isVaried()));

    html += "</tr>";


    html += "</table>";
    return html;
}

void SoilProfile::updateDepths()
{
    if (m_soilLayers.size()) {
        m_soilLayers[0]->setDepth(0);

        // Set the depth for the lowers below the one that changed
        for (int i = 0; i < m_soilLayers.size()-1; ++i)
            m_soilLayers[i+1]->setDepth(m_soilLayers.at(i)->depth() + m_soilLayers.at(i)->thickness());

        // Update the depth of the rock layer
        m_bedrock->setDepth(m_soilLayers.last()->depth() + m_soilLayers.last()->thickness());

        // Signal that the depths are updated
        emit depthsChanged();
    }
}

SoilLayer* SoilProfile::createRepresentativeSoilLayer(double top, double base)
{
    SoilLayer* selectedLayer = 0;
    double longestTime = 0;

    // If the layer is deeper than the site profile, use the deepest layer
    if (top > m_soilLayers.last()->depthToBase())
        return new SoilLayer(m_soilLayers.last());

    foreach (SoilLayer* sl, m_soilLayers) {
        // Skip the layer if it isn't in the depth range of interest
        if ( sl->depthToBase() < top || sl->depth() > base )
            continue;
        
        // If the layer is completely within a given layer, return that layer
        if ( sl->depth() <= top &&  base <= sl->depthToBase()) {
            selectedLayer = sl;
            break;
        }

        // The representative layer is the layer with the most travel time
        // through it

        // Path length within the depth interest range
        double length = 0;

        if (sl->depth() > top && base < sl->depthToBase()) {
            // New layer extends above the SoilLayer
            length =  base - sl->depth();
        } else if (sl->depth() < top && base > sl->depthToBase()) {
            // New layer extends below the SoilLayer
            length = sl->depthToBase() - top;
        } else {
            // New layer exists totally with the SoilLayer
            length = base - top;
        }

        // Compute the travel time
        double time = length / sl->shearVel();

        if (!selectedLayer || time > longestTime) {
            selectedLayer = sl;
            longestTime = time;
        }
    }

    Q_ASSERT(selectedLayer);
    
    return new SoilLayer(selectedLayer);
}

void SoilProfile::updateUnits()
{
    emit headerDataChanged(Qt::Horizontal, DepthColumn, MaxColumn);
}

VelocityLayer * SoilProfile::velocityLayer(int index) const
{
    if (index < m_soilLayers.size()) {
        return m_soilLayers.at(index);
    } else {
        return m_bedrock;
    }
}

QDataStream & operator<< (QDataStream & out, const SoilProfile* sp)
{
    out << (quint8)3;

    // Save soil types
    out << sp->m_soilTypeCatalog;

    // Save soil layers
    out << sp->m_soilLayers.size();

    foreach(const SoilLayer* sl, sp->m_soilLayers) {
        out << sl;

        // Save which soil type the soil layer is connected to.
        if (sl->soilType()) {
            out << sp->m_soilTypeCatalog->rowOf(sl->soilType());
        } else {
            out << -1;
        }
    }

    // Save remaining information
    out << sp->m_bedrock
            << sp->m_profileRandomizer
            << sp->m_nonlinearPropertyRandomizer
            << sp->m_inputDepth
            << sp->m_isVaried
            << sp->m_profileCount
            << sp->m_maxFreq
            << sp->m_waveFraction
            << sp->m_disableAutoDiscretization
            << sp->m_waterTableDepth;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilProfile* sp)
{
    quint8 ver;
    in >> ver;

    // Load soil types
    in >> sp->m_soilTypeCatalog;

    // Load soil layers
    sp->beginResetModel();

    int count;
    in >> count;
    while (sp->m_soilLayers.size() < count) {
        int row;
        SoilLayer* sl = new SoilLayer(sp);

        in >> sl >> row;


        // If no soil type is defined for the soil layer set it the pointer to be zero
        sl->setSoilType(
                row < 0 ? 0 : sp->m_soilTypeCatalog->soilType(row));

        sp->m_soilLayers << sl;
    }
    sp->updateDepths();
    sp->endResetModel();

    // Load remaining information
    in >> sp->m_bedrock
            >> sp->m_profileRandomizer
            >> sp->m_nonlinearPropertyRandomizer
            >> sp->m_inputDepth
            >> sp->m_isVaried
            >> sp->m_profileCount
            >> sp->m_maxFreq
            >> sp->m_waveFraction;

    if (ver > 1) {
        // Added disable auto-discretization in version 2
        in >> sp->m_disableAutoDiscretization;
    }

    if (ver > 2) {
        // Added water table depth in version 3
        in >> sp->m_waterTableDepth;
    }

    return in;
}
