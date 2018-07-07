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

#include "SoilProfile.h"

#include "Algorithms.h"
#include "BedrockDepthVariation.h"
#include "Location.h"
#include "LayerThicknessVariation.h"
#include "MyRandomNumGenerator.h"
#include "NonlinearPropertyRandomizer.h"
#include "ProfileRandomizer.h"
#include "RockLayer.h"
#include "SiteResponseModel.h"
#include "SoilLayer.h"
#include "SoilType.h"
#include "SoilTypeCatalog.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"
#include "VelocityVariation.h"

#include <QBrush>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <cmath>

SoilProfile::SoilProfile(SiteResponseModel * parent)
    : MyAbstractTableModel(parent), _siteResponseModel(parent)
{
    MyRandomNumGenerator* randNumGen = _siteResponseModel->randNumGen();

    _bedrock = new RockLayer;
    connect( _bedrock, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    _profileRandomizer = new ProfileRandomizer(
            randNumGen->gsl_pointer(), this);
    connect( _profileRandomizer, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    _nonlinearPropertyRandomizer = new NonlinearPropertyRandomizer(
            randNumGen->gsl_pointer(), this);
    connect( _nonlinearPropertyRandomizer, SIGNAL(wasModified()),
             this, SIGNAL(wasModified()));

    _soilTypeCatalog = new SoilTypeCatalog;
    connect(_soilTypeCatalog, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SIGNAL(wasModified()));

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(updateUnits()));

    _isVaried = false;
    _profileCount = 100;
    _inputDepth = -1;
    _maxFreq = 20;
    _waveFraction = 0.20;
    _disableAutoDiscretization = false;
    _waterTableDepth = 0.;
    _layerSelectionMethod = MidDepth;
}

SoilProfile::~SoilProfile()
{
    delete _profileRandomizer;
    delete _nonlinearPropertyRandomizer;
    delete _bedrock;
    delete _soilTypeCatalog;
}

int SoilProfile::rowCount(const QModelIndex &parent) const
{
   Q_UNUSED(parent);

   return _soilLayers.size() + 1;
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
                return QString::number(_soilLayers.at(index.row())->thickness(), 'f', 2);
            } else {
                // Rock layer
                return tr("Half-Space");
            }
        case SoilTypeColumn:
            if (index.row() < (rowCount() - 1)) {
                // Soil layers
                SoilType* const st = _soilLayers.at(index.row())->soilType();
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
    if(index.parent()!=QModelIndex() || _readOnly)
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

                _soilLayers.at(index.row())->setThickness(d);
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
                 if (SoilType* st = _soilTypeCatalog->soilTypeOf(value)) {
                    _soilLayers.at(index.row())->setSoilType(st);
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
        _soilLayers.insert(row, new SoilLayer(this));

    emit endInsertRows();
    return true;
}

bool SoilProfile::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!count)
        return false;

    emit beginRemoveRows(parent, row, qMin(row + count - 1, _soilLayers.size() - 1));

    for (int i=0; i < count; ++i) {
        if (_soilLayers.isEmpty())
            break;

        _soilLayers.takeAt(row)->deleteLater();       
    }
    emit endRemoveRows();

    // Update the depths of the layers.
    updateDepths();

    return true;
}

int SoilProfile::profileCount() const
{
    return _profileCount;
}

void SoilProfile::setProfileCount(int count)
{
    if ( _profileCount != count ) {
        _profileCount = count;

        emit profileCountChanged(_profileCount);
        emit wasModified();
    }
}

bool SoilProfile::isVaried() const
{
    return _isVaried;
}

void SoilProfile::setIsVaried(bool isVaried)
{
    if ( _isVaried != isVaried ) {
        _isVaried = isVaried;

        emit wasModified();
        emit isVariedChanged(_isVaried);
    }
}

QList<SoilLayer*> & SoilProfile::soilLayers()
{
    return _soilLayers;
}

QList<SubLayer> & SoilProfile::subLayers()
{
    return _subLayers;
}

RockLayer* SoilProfile::bedrock()
{
    return _bedrock;
}

double SoilProfile::inputDepth() const
{
    return _inputDepth;
}

void SoilProfile::setInputDepth(double depth)
{
    if ( _inputDepth != depth ) {
        _inputDepth = depth;

        emit wasModified();
        emit inputDepthChanged(_inputDepth);
    }
}

const Location & SoilProfile::inputLocation() const
{
    return _inputLocation;
}

const Location SoilProfile::depthToLocation(const double depth) const
{
    int index = 0;
    double interDepth = 0;

    if (depth < 0 || _subLayers.last().depthToBase() <= depth) {
        // Use the surface of the bedrock
        index = _subLayers.size();
        interDepth = 0;   
    } else {
        // Use the layer whose bottom depth is deeper
        index = 0;

        while (index <= _subLayers.size()
                && _subLayers.at(index).depthToBase() <= depth)
            ++index;

        interDepth = depth - _subLayers.at(index).depth();
    }

    return Location(index, interDepth);
}

ProfileRandomizer* SoilProfile::profileRandomizer()
{
    return _profileRandomizer;
}

NonlinearPropertyRandomizer* SoilProfile::nonlinearPropertyRandomizer()
{
    return _nonlinearPropertyRandomizer;
}

SoilTypeCatalog* SoilProfile::soilTypeCatalog()
{
    return _soilTypeCatalog;
}

double SoilProfile::waterTableDepth() const
{
    return _waterTableDepth;
}

void SoilProfile::setWaterTableDepth(double waterTableDepth)
{
    if (_waterTableDepth != waterTableDepth) {
        _waterTableDepth = waterTableDepth;

        emit wasModified();
        emit waterTableDepthChanged(waterTableDepth);
    }
}

double SoilProfile::maxFreq() const
{
    return _maxFreq;
}

void SoilProfile::setMaxFreq(double maxFreq)
{
    if ( _maxFreq != maxFreq ) {
        _maxFreq = maxFreq;

        emit wasModified();
        emit maxFreqChanged(_maxFreq);
    }
}

double SoilProfile::waveFraction() const
{
    return _waveFraction;
}

void SoilProfile::setWaveFraction(double waveFraction)
{
    if ( _waveFraction != waveFraction ) {
        _waveFraction = waveFraction;

        emit wasModified();
        emit waveFractionChanged(_waveFraction);
    }
}

bool SoilProfile::disableAutoDiscretization() const
{
    return _disableAutoDiscretization;
}

void SoilProfile::setDisableAutoDiscretization(bool disableAutoDiscretization)
{
    if (_disableAutoDiscretization != disableAutoDiscretization) {
        _disableAutoDiscretization = disableAutoDiscretization;

        emit wasModified();
        emit disableAutoDiscretizationChanged(_disableAutoDiscretization);
    }
}

QStringList SoilProfile::soilLayerNameList() const
{
    QStringList list;

    for (int i = 0; i < _soilLayers.size(); ++i)
        list << QString("%1 %2").arg(i+1).arg(_soilLayers.at(i)->toString());
    
    return list;
}

void SoilProfile::createSubLayers(TextLog * textLog)
{
    // Clear the previously generated sublayers and delete the soillayers if
    // they are not the used defined values.  This could be improved as it runs
    // regardless of it is really needed. FIXME
    if (!_subLayers.isEmpty()) {
        QList<SoilLayer*> soilLayers;

        // Create a list of unique soilLayers
        foreach (SubLayer sl, _subLayers) {
            if (!soilLayers.contains(sl.soilLayer()))
                soilLayers << sl.soilLayer();
        }

        // Delete the SoilLayers created in the process of randomizing the site
        foreach (SoilLayer* sl, soilLayers) {
            if (!_soilLayers.contains(sl))
                delete sl;
        }

        // Clear the sublayers
        _subLayers.clear();
    }

    // Vary the nonlinear properties of the SoilTypes
    if (_nonlinearPropertyRandomizer->enabled()) {
        if (textLog->level() > TextLog::Low) {
            textLog->append(QObject::tr("Varying dynamic properties of soil types"));
        }

        // Vary the nonlinear properties of the soil types
        for (int i = 0; i < _soilTypeCatalog->rowCount(); ++i) {
            SoilType * st = _soilTypeCatalog->soilType(i);

            if (st->isVaried()) {
                // Vary the properties
                if (textLog->level() > TextLog::Low) {
                    textLog->append(QString("\t%1").arg(st->name()));
                }
                _nonlinearPropertyRandomizer->vary(st);
            }
        }

        // Vary the damping of the bedrock
        if (_nonlinearPropertyRandomizer->bedrockIsEnabled()) {
            if ( textLog->level() > TextLog::Low ) {
                textLog->append(QObject::tr("Varying damping of bedrock"));
            }
            _nonlinearPropertyRandomizer->vary(_bedrock);
        }
    }
    
    // Vary the depth to the bedrock
    double depthToBedrock;
    const double minDepthToBedrock = 1.0;

    if (_profileRandomizer->bedrockDepthVariation()->enabled()) {
        if ( textLog->level() > TextLog::Low ) {
            textLog->append(QObject::tr("Varying depth to bedrock"));       
}
        _profileRandomizer->bedrockDepthVariation()->setAvg(_bedrock->depth());


        depthToBedrock = _profileRandomizer->bedrockDepthVariation()->rand();

        if (depthToBedrock < minDepthToBedrock) {
            depthToBedrock = minDepthToBedrock;

            textLog->append(QString("\tDepth to bedrock limited to be greater than 1 %1").arg(
                                Units::instance()->length()));
        }
    } else {
        depthToBedrock = _bedrock->depth();
    }

    // Vary the layering 
    QList<SoilLayer*> soilLayers;
    if (_profileRandomizer->layerThicknessVariation()->enabled()) {        
        if ( textLog->level() > TextLog::Low ) {
            textLog->append(QObject::tr("Varying the layering"));
        }
        // Randomize the layer thicknesses
        QList<double> thicknesses =
                _profileRandomizer->layerThicknessVariation()->vary(depthToBedrock);

        // For each thickness, determine the representative soil layer
        double depth = 0;
        foreach (double thickness, thicknesses) {
            soilLayers << createRepresentativeSoilLayer(depth, depth+thickness);
            // Set the new depth and thickness
            soilLayers.last()->setDepth(depth);
            soilLayers.last()->setThickness(thickness);

            // Increment the depth
            depth += thickness;
        }
    } else {
        if (_profileRandomizer->bedrockDepthVariation()->enabled()) {
            foreach (SoilLayer * layer, _soilLayers) {               
                if (layer->depthToBase() > depthToBedrock) {
                    // Create a new SoilLayer since the thickness will be modified
                    soilLayers << new SoilLayer(layer);

                    // Truncate the last SoilLayer once the depth to the bedrock is exceeded
                    double thickness = depthToBedrock - layer->depth();
                    soilLayers.last()->setThickness(thickness);
                    break;
                } else {
                    // Add the layer
                    soilLayers << layer;
                }
            }

            // Add thickness to the last layer if needed            
            if (soilLayers.last()->depthToBase() < depthToBedrock) {
                // Replace the last SoilLayer with a copy
                soilLayers << new SoilLayer(soilLayers.takeLast());
                soilLayers.last()->setThickness(depthToBedrock - soilLayers.last()->depth());
            }
        } else {
            // Copy over previous soil layers
            soilLayers = _soilLayers;
        }
    }

    // Vary the shear-wave velocity
    if (_profileRandomizer->velocityVariation()->enabled()) {
        if (textLog->level() > TextLog::Low )
            textLog->append(QObject::tr("Varying the shear-wave velocity"));

        _profileRandomizer->velocityVariation()->vary(soilLayers, _bedrock);
    } else {
        // Reset the values
        foreach (SoilLayer* sl, soilLayers)
            sl->reset();

        _bedrock->reset();
    }

    /* Create the SubLayers by dividing the thicknesses.  The height of the
     * sublayer is determined by computing the shortest wavelength of interest
     * (Vs/freqMax) and then reducing this by the wavelength fraction.
     */
    double depth = 0;
    double vTotalStress = 0;

    if (_disableAutoDiscretization) {
        foreach (SoilLayer* sl, soilLayers) {
            _subLayers << SubLayer(sl->thickness(), depth, vTotalStress, _waterTableDepth, sl);

            depth += sl->thickness();
            vTotalStress += sl->thickness() * sl->untWt();
        }
    } else {
        foreach (SoilLayer* sl, soilLayers) {
            // Compute the optimal thickness of the sublayers
            double optSubThickness = sl->shearVel() / _maxFreq * _waveFraction;
            // Compute the required number of sub-layers for this thickness
            int numSubLayers = int(ceil(sl->thickness() / optSubThickness));
            // The subThickness for an even number of layers
            double subThickness = sl->thickness() / numSubLayers;

            for (int j = 0; j < numSubLayers; ++j) {
                _subLayers << SubLayer(subThickness, depth, vTotalStress, _waterTableDepth, sl);
                // Increment the depth by the subThicknees
                depth += subThickness;
                // Compute the stress at the base of the layer and this to the total stress
                vTotalStress += subThickness * sl->untWt();
            }
        }
    }
    
    // Compute the SubLayer index associated with the input depth
    _inputLocation = depthToLocation(_inputDepth);
}

void SoilProfile::resetSubLayers()
{
    for (SubLayer &sl : _subLayers)
        sl.reset();
}

int SoilProfile::subLayerCount() const
{
    return _subLayers.size();
}

double SoilProfile::untWt( int layer ) const
{
    if ( layer < _subLayers.size() ) {
        return _subLayers.at(layer).untWt();
    } else {
        return _bedrock->untWt();
    }
}

double SoilProfile::density( int layer ) const
{
    if ( layer < _subLayers.size() ) {
        return _subLayers.at(layer).density();
    } else {
        return _bedrock->density();
    }
}

double SoilProfile::shearVel( int layer ) const
{
    if ( layer < _subLayers.size() ) {
        return _subLayers.at(layer).shearVel();
    } else {
        return _bedrock->shearVel();
    }
}

double SoilProfile::shearMod(int layer) const
{
    if ( layer < _subLayers.size() ) {
        return _subLayers.at(layer).shearMod();
    } else {
        return _bedrock->shearMod();
    }
}

double SoilProfile::damping( int layer ) const
{
    if ( layer < _subLayers.size() ) {
        return _subLayers.at(layer).damping();
    } else {
        return _bedrock->damping();
    }
}

QVector<double> SoilProfile::depthProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.depth();

    profile << _bedrock->depth();

    return profile;
}

QVector<double> SoilProfile::depthToMidProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.depthToMid();

    return profile;
}

QVector<double> SoilProfile::initialVelocityProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.initialShearVel();

    profile << _bedrock->shearVel();

    return profile;
}

QVector<double> SoilProfile::finalVelocityProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.shearVel();

    profile << _bedrock->shearVel();

    return profile;
}

QVector<double> SoilProfile::modulusProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.shearMod();

    profile << _bedrock->shearMod();

    return profile;
}

QVector<double> SoilProfile::dampingProfile() const
{
    QVector<double> profile;
    for (const SubLayer &sl : _subLayers)
        profile << sl.damping();

    profile << _bedrock->damping();

    return profile;
}

QVector<double> SoilProfile::vTotalStressProfile() const
{
    QVector<double> profile;

    // Add value at surface
    profile << 0.;

    for (const SubLayer &sl : _subLayers)
        profile << sl.vTotalStress();

    return profile;
}

QVector<double> SoilProfile::vEffectiveStressProfile() const
{
    QVector<double> profile;

    // Add value at surface
    profile << 0.;

    for (const SubLayer &sl : _subLayers)
        profile << sl.vEffectiveStress();

    return profile;
}

QVector<double> SoilProfile::maxErrorProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.error();

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

    for (const SubLayer &sl : _subLayers) {
        // Add the half layer to the total weight 
        totalWeight += sl.untWt() * sl.thickness() / 2.;

        const double rigidStress = totalWeight * pga;

        profile << sl.shearStress() / rigidStress;

        // Add the half layer to the total weight 
        totalWeight += sl.untWt() * sl.thickness() / 2.;
    }

    // Add the value at the base of the profile
    profile << _subLayers.last().shearStress() / (totalWeight * pga);

    return profile;
}

QVector<double> SoilProfile::maxShearStrainProfile() const
{
    QVector<double> profile;
    
    for (int i = 0; i < _subLayers.size(); ++i)
        profile << _subLayers.at(i).maxStrain();

    return profile;
}

QVector<double> SoilProfile::shearStressProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.shearStress();

    return profile;
}

QVector<double> SoilProfile::stressRatioProfile() const
{
    QVector<double> profile;

    for (const SubLayer &sl : _subLayers)
        profile << sl.stressRatio();

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
    for (int i = 0; i < _subLayers.size(); ++i)
        html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13</tr>")
        .arg(i+1)
        .arg(_subLayers.at(i).soilTypeName())
        .arg(_subLayers.at(i).depth(), 0, 'f', 2)
        .arg(_subLayers.at(i).thickness(), 0, 'f', 2)
        .arg(_subLayers.at(i).maxStrain(), 0, 'e', 2)
        .arg(_subLayers.at(i).effStrain(), 0, 'e', 2)
        .arg(_subLayers.at(i).damping(), 0, 'f', 2)
        .arg(_subLayers.at(i).oldDamping(), 0, 'f', 2)
        .arg(_subLayers.at(i).dampingError(), 0, 'f', 2)
        .arg(_subLayers.at(i).shearMod(), 0, 'f', 0)
        .arg(_subLayers.at(i).oldShearMod(), 0, 'f', 0)
        .arg(_subLayers.at(i).shearModError(), 0, 'f', 2)
        .arg(_subLayers.at(i).normShearMod(), 0, 'f', 3);

    // Bedrock layer
    html += QString("<tr><td>%1<td>%2<td>%3<td>%4<td>%5<td>%6<td>%7<td>%8<td>%9<td>%10<td>%11<td>%12<td>%13<td>%14</tr>")
            .arg(_subLayers.size()+1)
            .arg(QObject::tr("Bedrock"))
            .arg(_subLayers.last().depthToBase(), 0, 'f', 2)
            .arg("--")
            .arg("--")
            .arg("--")
            .arg("--")
            .arg(_bedrock->damping(), 0, 'f', 2)
            .arg("--")
            .arg("--")
            .arg(_bedrock->shearMod(), 0, 'f', 0)
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
    QString html = _soilTypeCatalog->toHtml();

    html +=  QString(tr(
            "<li><a name=\"Bedrock\">Bedrock<a>"
            "<table border=\"0\">"
            "<tr><th>Unit weight:</th><td>%1 %2</td></tr>"
            "<tr><th>Damping:</th><td>%3</td></tr>"))
            .arg(_bedrock->untWt())
            .arg(Units::instance()->untWt())
            .arg(_bedrock->damping());


    if (_isVaried)
        html += QString("<tr><th>Varied:</th><td>%1</td></tr>").arg(
                boolToString(_bedrock->isVaried()));


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

    if (_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
        html += tr("<th>Stdev.</th>");

    if (_profileRandomizer->velocityVariation()->enabled())
        html += QString(tr(
                "<th>Minimum Vs. (%1)</th>"
                "<th>Maximum Vs. (%1)</th>"
                "<th>Varied</th>"
                )).arg(Units::instance()->vel());

    html += "</tr>";

    // Table information
    for (int i = 0; i < _soilLayers.size(); ++i ) {
        html += QString("<tr><td>%1</td><td>%2</td><td><a href=\"#%3\">%3</a></td><td>%4</td>")
                .arg(_soilLayers.at(i)->depth())
                .arg(_soilLayers.at(i)->thickness())
                .arg(_soilLayers.at(i)->soilType()->name())
                .arg(_soilLayers.at(i)->avg());

        if (_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
            html += QString("<td>%1</td>").arg(_soilLayers.at(i)->stdev());

        if (_profileRandomizer->velocityVariation()->enabled())
            html += QString("<td>%1</td><td>%2</td><td>%3</td>")
            .arg(_soilLayers.at(i)->min())
            .arg(_soilLayers.at(i)->max())
            .arg(boolToString(_soilLayers.at(i)->isVaried()));

        html += "</tr>";
    }

    // Bedrock layer
    html += QString("<tr><td>%1</td><td>---</td><td><a href=\"#Bedrock\">Bedrock</a></td><td>%2</td>")
            .arg(_bedrock->depth())
            .arg(_bedrock->avg());

    if (_profileRandomizer->velocityVariation()->stdevIsLayerSpecific())
        html += QString("<td>%1</td>").arg(_bedrock->stdev());

    if (_profileRandomizer->velocityVariation()->enabled())
        html += QString("<td>%1</td><td>%2</td><td>%3</td>")
        .arg(_bedrock->min())
        .arg(_bedrock->max())
        .arg(boolToString(_bedrock->isVaried()));

    html += "</tr>";


    html += "</table>";
    return html;
}

void SoilProfile::updateDepths()
{
    if (_soilLayers.size()) {
        _soilLayers[0]->setDepth(0);

        // Set the depth for the lowers below the one that changed
        for (int i = 0; i < _soilLayers.size()-1; ++i)
            _soilLayers[i+1]->setDepth(_soilLayers.at(i)->depth() + _soilLayers.at(i)->thickness());

        // Update the depth of the rock layer
        _bedrock->setDepth(_soilLayers.last()->depth() + _soilLayers.last()->thickness());

        // Signal that the depths are updated
        emit depthsChanged();
    }
}

SoilLayer* SoilProfile::createRepresentativeSoilLayer(double top, double base)
{
    if (_layerSelectionMethod == MaximumTravelTime) {
        // The representative layer is the layer with the most travel time
        // through it
        SoilLayer* selectedLayer = 0;

        double longestTime = 0;

        // If the layer is deeper than the site profile, use the deepest layer
        if (top > _soilLayers.last()->depthToBase())
            return new SoilLayer(_soilLayers.last());

        for (SoilLayer *sl : _soilLayers) {
            // Skip the layer if it isn't in the depth range of interest
            if ( sl->depthToBase() < top || sl->depth() > base )
                continue;

            // If the layer is completely within a given layer, return that layer
            if ( sl->depth() <= top &&  base <= sl->depthToBase()) {
                selectedLayer = sl;
                break;
            }
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
    } else if (_layerSelectionMethod == MidDepth) {
        const float midDepth = (top + base) / 2.0;
        SoilLayer * newLayer = 0;

        // Try each of the layers
        for (SoilLayer *sl : _soilLayers) {
            if (sl->depth() < midDepth && midDepth <= sl->depthToBase()) {
                newLayer = new SoilLayer(sl);
            }
        }
        // Use the last layer is none are found
        if (!newLayer)
            newLayer = new SoilLayer(_soilLayers.last());

        return newLayer;
    }

    return NULL;
}

void SoilProfile::updateUnits()
{
    emit headerDataChanged(Qt::Horizontal, DepthColumn, MaxColumn);
}

VelocityLayer * SoilProfile::velocityLayer(int index) const
{
    if (index < _soilLayers.size()) {
        return _soilLayers.at(index);
    } else {
        return _bedrock;
    }
}

void SoilProfile::fromJson(const QJsonObject &json)
{
    _inputDepth = json["inputDepth"].toDouble();
    _isVaried = json["isVaried"].toBool();
    _profileCount = json["profileCount"].toInt();
    _maxFreq = json["maxFreq"].toDouble();
    _waveFraction = json["waveFraction"].toDouble();
    _disableAutoDiscretization = json["disableAutoDiscretization"].toBool();
    _waterTableDepth = json["waterTableDepth"].toDouble();

    _bedrock->fromJson(json["bedrock"].toObject());
    _nonlinearPropertyRandomizer->fromJson(json["nonlinearPropertyRandomizer"].toObject());
    _soilTypeCatalog->fromJson(json["soilTypeCatalog"].toArray());
    _profileRandomizer->fromJson(json["profileRandomizer"].toObject());

    beginResetModel();
    // Delete the old layers
    while (_soilLayers.size())
        _soilLayers.takeLast()->deleteLater();

    for (const QJsonValue &jv : json["soilLayers"].toArray()) {
        QJsonObject sljo = jv.toObject();

        SoilLayer * sl = new SoilLayer(this);
        sl->fromJson(sljo);

        const int row = sljo["soilType"].toInt();
        sl->setSoilType(row < 0 ? 0 : _soilTypeCatalog->soilType(row));

        _soilLayers << sl;
    }

    updateDepths();
    endResetModel();
}

QJsonObject SoilProfile::toJson() const
{
    QJsonObject json;
    json["inputDepth"] = _inputDepth;
    json["isVaried"] = _isVaried;
    json["profileCount"] = _profileCount;
    json["maxFreq"] = _maxFreq;
    json["waveFraction"] = _waveFraction;
    json["disableAutoDiscretization"] = _disableAutoDiscretization;
    json["waterTableDepth"] = _waterTableDepth;

    json["bedrock"] = _bedrock->toJson();
    json["nonlinearPropertyRandomizer"] = _nonlinearPropertyRandomizer->toJson();
    json["soilTypeCatalog"] = _soilTypeCatalog->toJson();
    json["profileRandomizer"] = _profileRandomizer->toJson();

    QJsonArray soilLayers;
    for (const SoilLayer *sl : _soilLayers) {
        QJsonObject sljo = sl->toJson();
        // Save the soil type index
        sljo["soilType"] = sl->soilType() ? _soilTypeCatalog->rowOf(sl->soilType()) : -1;
        soilLayers << sljo;
    }
    json["soilLayers"] = soilLayers;

    return json;
}

QDataStream & operator<< (QDataStream & out, const SoilProfile* sp)
{
    out << (quint8)3;

    // Save soil types
    out << sp->_soilTypeCatalog;

    // Save soil layers
    out << sp->_soilLayers.size();
    for (const SoilLayer *sl : sp->_soilLayers) {
        out << sl;
        // Save which soil type the soil layer is connected to.
        if (sl->soilType()) {
            out << sp->_soilTypeCatalog->rowOf(sl->soilType());
        } else {
            out << -1;
        }
    }

    // Save remaining information
    out << sp->_bedrock
            << sp->_profileRandomizer
            << sp->_nonlinearPropertyRandomizer
            << sp->_inputDepth
            << sp->_isVaried
            << sp->_profileCount
            << sp->_maxFreq
            << sp->_waveFraction
            << sp->_disableAutoDiscretization
            << sp->_waterTableDepth;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilProfile* sp)
{
    quint8 ver;
    in >> ver;

    // Load soil types
    in >> sp->_soilTypeCatalog;

    // Load soil layers
    sp->beginResetModel();

    int count;
    in >> count;
    while (sp->_soilLayers.size() < count) {
        int row;
        SoilLayer* sl = new SoilLayer(sp);

        in >> sl >> row;
        // If no soil type is defined for the soil layer set it the pointer to be zero
        sl->setSoilType(
                row < 0 ? 0 : sp->_soilTypeCatalog->soilType(row));

        sp->_soilLayers << sl;
    }
    sp->updateDepths();
    sp->endResetModel();

    // Load remaining information
    in >> sp->_bedrock
            >> sp->_profileRandomizer
            >> sp->_nonlinearPropertyRandomizer
            >> sp->_inputDepth
            >> sp->_isVaried
            >> sp->_profileCount
            >> sp->_maxFreq
            >> sp->_waveFraction;

    if (ver > 1) {
        // Added disable auto-discretization in version 2
        in >> sp->_disableAutoDiscretization;
    }

    if (ver > 2) {
        // Added water table depth in version 3
        in >> sp->_waterTableDepth;
    }

    return in;
}
