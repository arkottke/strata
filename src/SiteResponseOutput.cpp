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

#include "IndexedDepth.h"
#include "SiteResponseOutput.h"
#include "Serializer.h"
#include "Units.h"

#include <QtAlgorithms>
#include <QDebug>
#include <QFile>
#include <QString>

SiteResponseOutput::SiteResponseOutput(QObject * parent)
    : QObject(parent)
{
    m_textLog = new TextLog;
    connect(m_textLog, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_period = new Dimension;
    connect(m_period, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_freq = new Dimension;
    connect(m_freq, SIGNAL(wasModified()), SIGNAL(wasModified()));

    // Initialize the outputs with names
    m_initialShearVel = new Output(Output::InitialVelProfile);
    connect(m_initialShearVel, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_finalShearVel = new Output(Output::FinalVelProfile);
    connect(m_finalShearVel, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_finalShearMod = new Output(Output::ModulusProfile);
    connect(m_finalShearMod, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_finalDamping = new Output(Output::DampingProfile);
    connect(m_finalDamping, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_vTotalStress = new Output(Output::VerticalStressProfile);
    connect(m_vTotalStress, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_maxError = new Output(Output::MaxErrorProfile);
    connect(m_maxError, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_maxShearStress = new Output(Output::MaxStressProfile);
    connect(m_maxShearStress, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_stressReducCoeff = new Output(Output::StressReducCoeffProfile);
    connect(m_stressReducCoeff, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_maxShearStrain = new Output(Output::MaxStrainProfile);
    connect(m_maxShearStrain, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_maxAccel = new Output(Output::MaxAccelProfile);
    connect(m_maxAccel, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_maxVel = new Output(Output::MaxVelProfile);
    connect(m_maxVel, SIGNAL(wasModified()), SIGNAL(wasModified()));

    m_stressRatio = new Output(Output::StressRatioProfile);
    connect(m_stressRatio, SIGNAL(wasModified()), SIGNAL(wasModified()));

    reset();
}

SiteResponseOutput::~SiteResponseOutput()
{
    while (!m_responseLocations.isEmpty())
        delete m_responseLocations.takeFirst();
    
    while (!m_ratioLocations.isEmpty())
        delete m_ratioLocations.takeFirst();

    while (!m_nlProperties.isEmpty())
        delete m_nlProperties.takeFirst();
}
        
void SiteResponseOutput::reset()
{
    m_siteCount = 0;
    m_motionCount = 0;
    m_totalCount = 0;
    m_hasResults = false;

    m_damping = 5.0;

    m_fileName = "";
    m_title = "";
    m_filePrefix = "";

    m_period->setSpacing(Dimension::Log);
    m_period->setMin(0.01);
    m_period->setMax(10);
    m_period->setSize(100);

    m_freq->setSpacing(Dimension::Log);
    m_freq->setMin(0.1);
    m_freq->setMax(100);
    m_freq->setSize(1024);

    // Reset the data
    while (!m_responseLocations.isEmpty())
        delete m_responseLocations.takeFirst();
    
    while (!m_ratioLocations.isEmpty())
        delete m_ratioLocations.takeFirst();

    while (!m_nlProperties.isEmpty())
        delete m_nlProperties.takeFirst();
    
    m_initialShearVel->reset();
    m_finalShearVel->reset();
    m_finalShearMod->reset();
    m_finalDamping->reset();
    m_vTotalStress->reset();
    m_maxError->reset();
    m_maxShearStress->reset();
    m_stressReducCoeff->reset();
    m_maxShearStrain->reset();
    m_maxAccel->reset();
    m_maxVel->reset();
    m_stressRatio->reset();
}

QString SiteResponseOutput::fileName() const
{
    return m_fileName;
}

void SiteResponseOutput::setFileName(const QString & fileName)
{
    if (m_fileName != fileName) {
        emit wasModified();
    }

    m_fileName = fileName;
}

bool SiteResponseOutput::periodIsNeeded()
{
    for (int i = 0; i < m_responseLocations.size(); ++i)
        if (m_responseLocations.at(i)->needsPeriod())
            return true;

    for (int i = 0; i < m_ratioLocations.size(); ++i)
        if (m_ratioLocations.at(i)->respRatio()->enabled())
            return true;

    return false;
}

bool SiteResponseOutput::freqIsNeeded()
{
    for (int i = 0; i < m_ratioLocations.size(); ++i) {
        if (m_ratioLocations.at(i)->transFunc()->enabled()
            || m_ratioLocations.at(i)->strainTransFunc()->enabled()) {
            return true;
        }
    }
    
    for (int i = 0; i < m_responseLocations.size(); ++i) {
        if (m_responseLocations.at(i)->fourierSpec()->enabled()) {
            return true;
        }
    }

    return false;
}

QStringList & SiteResponseOutput::motionNames()
{
    return m_motionNames;
}

const QString & SiteResponseOutput::filePrefix() const
{
	return m_filePrefix;
}

void SiteResponseOutput::setFilePrefix(const QString & prefix)
{
    if (m_filePrefix != prefix) {
        emit wasModified();
    }

	m_filePrefix = prefix;
}

const QString & SiteResponseOutput::title() const
{
	return m_title;
}

void SiteResponseOutput::setTitle(const QString & title)
{
    if (m_title != title) {
        emit wasModified();
    }

	m_title = title;
}

bool SiteResponseOutput::seriesEnabledAt(int i) const
{
    return m_seriesEnabled.at(i);
}

QList<bool> & SiteResponseOutput::seriesEnabled()
{
    return m_seriesEnabled;
}
        
bool SiteResponseOutput::motionEnabledAt(int index) const
{
    for (int i = index % m_motionCount; i < m_seriesEnabled.size(); i += m_motionCount) {
        if (!m_seriesEnabled.at(i)) {
            return false;
        }
    }

    return true;
}

void SiteResponseOutput::setMotionEnabledAt(int index, bool enabled)
{
    for (int i = index % m_motionCount; i < m_seriesEnabled.size(); i += m_motionCount) {
        m_seriesEnabled[i] = enabled;
    }
    
    emit enabledChanged();
    emit wasModified();
}

bool SiteResponseOutput::siteEnabledAt(int index) const
{
    int startIdx = m_motionCount * (index / m_motionCount);

    for (int i = startIdx; i < startIdx + m_motionCount; ++i) {
        if (!m_seriesEnabled.at(i)) {
            return false;
        }
    }

    return true;
}

void SiteResponseOutput::setSiteEnabledAt(int index, bool enabled)
{
    // The smallest index of this possible motion
    int startIdx = m_motionCount * (index / m_motionCount);

    for (int i = startIdx; i < startIdx + m_motionCount; ++i) {
        m_seriesEnabled[i] = enabled;
    }

    emit enabledChanged();
    emit wasModified();
}

QString SiteResponseOutput::motionNameAt(int index) const
{
    return m_motionNames.at(index % m_motionCount);
}

QString SiteResponseOutput::siteNameAt(int index) const
{
    return QString::number(1 + index / m_motionCount);
}

Dimension * SiteResponseOutput::period()
{
    return m_period;
}

Dimension * SiteResponseOutput::freq()
{
    return m_freq;
}

const QVector<double> & SiteResponseOutput::depths() const
{
    return m_depths;
}

const QVector<QVector<double> > & SiteResponseOutput::times() const
{
    return m_times;
}

const QVector<QVector<double> > & SiteResponseOutput::strains() const
{
    return m_strains;
}

QList<ResponseLocation*> & SiteResponseOutput::responseLocations()
{
    return m_responseLocations;
}

QList<RatioLocation*> & SiteResponseOutput::ratioLocations()
{
    return m_ratioLocations;
}

Output * SiteResponseOutput::initialShearVel()
{
    return m_initialShearVel;
}

Output * SiteResponseOutput::finalShearVel()
{
    return m_finalShearVel;
}

Output * SiteResponseOutput::finalShearMod()
{
    return m_finalShearMod;
}

Output * SiteResponseOutput::finalDamping()
{
    return m_finalDamping;
}

Output * SiteResponseOutput::vTotalStress()
{
    return m_vTotalStress;
}

Output * SiteResponseOutput::maxError()
{
    return m_maxError;
}

Output * SiteResponseOutput::maxShearStress()
{
    return m_maxShearStress;
}

Output * SiteResponseOutput::stressReducCoeff()
{
    return m_stressReducCoeff;
}

Output * SiteResponseOutput::maxShearStrain()
{
    return m_maxShearStrain;
}

Output * SiteResponseOutput::maxAccel()
{
    return m_maxAccel;
}

Output * SiteResponseOutput::maxVel()
{
    return m_maxVel;
}

Output * SiteResponseOutput::stressRatio()
{
    return m_stressRatio;
}

double SiteResponseOutput::damping() const
{
    return m_damping;
}

void SiteResponseOutput::setDamping(double damping)
{
    m_damping = damping;
}
        
TextLog * SiteResponseOutput::textLog()
{
    return m_textLog;
}

int SiteResponseOutput::siteCount() const
{
    return m_siteCount;
}

int SiteResponseOutput::motionCount() const
{
    return m_motionCount;
}

int SiteResponseOutput::totalCount() const
{
    return m_totalCount;
}

int SiteResponseOutput::generalIndex(const int motionIndex, const int siteIndex) const
{
    // Each motion uses all of the siteCounts, and then the site would be added on
    return m_siteCount * motionIndex + siteIndex;
}

void SiteResponseOutput::createOutputList() 
{
    m_outputList.clear();
    //
    // Profiles
    //
    if (m_initialShearVel->enabled()) {
        m_outputList << m_initialShearVel;
    }

    if (m_finalShearVel->enabled()) {
        m_outputList << m_finalShearVel;
    }

    if (m_finalShearMod->enabled()) {
        m_outputList << m_finalShearMod;
    }

    if (m_finalDamping->enabled()) {
        m_outputList << m_finalDamping;
    }

    if (m_vTotalStress->enabled()) {
        m_outputList << m_vTotalStress;
    }
    
    if (m_maxError->enabled()) {
        m_outputList << m_maxError;
    }

    if (m_maxShearStress->enabled()) {
        m_outputList << m_maxShearStress;
    }
    
    if (m_stressReducCoeff->enabled()) {
        m_outputList << m_stressReducCoeff;
    }

    if (m_maxShearStrain->enabled()) {
        m_outputList << m_maxShearStrain;
    }

    if (m_stressRatio->enabled()) {
        m_outputList << m_stressRatio;
    }

    if (m_maxAccel->enabled()) {
        m_outputList << m_maxAccel;
    }
    
    if (m_maxVel->enabled()) {
        m_outputList << m_maxVel;
    }

    // Nonlinear properties
    for (int i = 0; i < m_nlProperties.size(); ++i) {
        if (m_nlProperties.at(i)->enabled()) {
            m_outputList << m_nlProperties[i];
        }
    }
       
    //
    // Response Locations
    //
    for (int i = 0; i < m_responseLocations.size(); ++i) {
        if (m_responseLocations[i]->respSpec()->enabled()) {
			m_outputList << m_responseLocations[i]->respSpec();
        }

        if (m_responseLocations[i]->fourierSpec()->enabled()) {
			m_outputList << m_responseLocations[i]->fourierSpec();
        }
        
        if (m_hasTime) {
            if (m_responseLocations[i]->accelTs()->enabled()) {
                m_outputList << m_responseLocations[i]->accelTs();
            }

            if (m_responseLocations[i]->velTs()->enabled()) {
                m_outputList << m_responseLocations[i]->velTs();
            }

            if (m_responseLocations[i]->dispTs()->enabled()) {
                m_outputList << m_responseLocations[i]->dispTs();
            }

            if (m_responseLocations[i]->strainTs()->enabled()) {
                m_outputList << m_responseLocations[i]->strainTs();
            }

            if (m_responseLocations[i]->stressTs()->enabled()) {
                m_outputList << m_responseLocations[i]->stressTs();
            }
        }
    }

    //
    // Ratio Locations
    //
    for (int i = 0; i < m_ratioLocations.size(); ++i) {
        if (m_ratioLocations[i]->transFunc()->enabled())
            m_outputList << m_ratioLocations[i]->transFunc();
        
        if (m_ratioLocations[i]->strainTransFunc()->enabled())
            m_outputList << m_ratioLocations[i]->strainTransFunc();

        if (m_ratioLocations[i]->respRatio()->enabled())
            m_outputList << m_ratioLocations[i]->respRatio();
    }

    // Define this to be the parent of all of the enabled outputs
    for (int i = 0; i < m_outputList.size(); ++i)
        m_outputList[i]->setParent(this);
    
    m_outputNameList.clear();
    // Create a list of all of the names
    for (int i = 0; i < m_outputList.size(); ++i)
        m_outputNameList << m_outputList.at(i)->name();

    emit outputListChanged();
}

QList<Output *> & SiteResponseOutput::output()
{
    return m_outputList;
}

const QStringList & SiteResponseOutput::outputNames()
{
    return m_outputNameList;
}

void SiteResponseOutput::computeStats()
{
    for (int i = 0; i < m_outputList.size(); ++i)
        m_outputList[i]->computeStats();
}

void SiteResponseOutput::clear()
{
    m_textLog->clear();

    m_motionNames.clear();
    m_seriesEnabled.clear();

    m_times.clear();
    m_depths.clear();
    m_strains.clear();

    m_outputList.clear();
    m_outputNameList.clear();
        
    m_initialShearVel->clear();
    m_finalShearVel->clear();
    m_finalShearMod->clear();
    m_finalDamping->clear();
    m_vTotalStress->clear();
    m_maxError->clear();
    m_maxShearStress->clear();
    m_stressReducCoeff->clear();
    m_maxShearStrain->clear();
    m_maxAccel->clear();
    m_maxVel->clear();
    m_stressRatio->clear();

    for (int i = 0; i < m_responseLocations.size(); ++i) {
        m_responseLocations[i]->clear();
    }

    for (int i = 0; i < m_ratioLocations.size(); ++i) {
        m_ratioLocations[i]->clear();
    }

    while (!m_nlProperties.isEmpty()) {
        delete m_nlProperties.takeFirst();
    }

    m_hasResults = false;
    emit wasModified();
}

void SiteResponseOutput::saveProfile(SiteProfile * siteProfile)
{
    // Compute the depth vector
    computeDepthVector(siteProfile->subLayers().last().depthToBase());

    if (m_initialShearVel->enabled()) {
        m_initialShearVel->addInterpData(siteProfile->shearVelProfile(), siteProfile->subLayers(), m_depths);
    }
    
    if (m_vTotalStress->enabled()) {
        m_vTotalStress->addInterpData(siteProfile->vTotalStressProfile(), siteProfile->subLayers(), m_depths);
    }

    int idx = 0;
    for (int i = 0; i < siteProfile->soilTypes().size(); ++i) {
        if (siteProfile->soilTypes().at(i)->saveData()) {
            m_nlProperties[idx]->addData(siteProfile->soilTypes().at(i)->normShearMod()->prop().toVector());
            ++idx;
            
            m_nlProperties[idx]->addData(siteProfile->soilTypes().at(i)->damping()->prop().toVector());
            ++idx;
        }
    }
}
        
void SiteResponseOutput::saveResults(EquivLinearCalc * calc)
{
    // Save that the results are enabled

    m_seriesEnabled << true;

    if (m_finalShearVel->enabled()) {
        m_finalShearVel->addInterpData(calc->site()->shearVelProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_finalShearMod->enabled()) {
        m_finalShearMod->addInterpData(calc->site()->shearModProfile(), calc->site()->subLayers(), m_depths);
    }
    
    if (m_finalDamping->enabled()) {
        m_finalDamping->addInterpData(calc->site()->dampingProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_maxError->enabled()) {
        m_maxError->addInterpData(calc->site()->maxErrorProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_maxShearStress->enabled()) {
        m_maxShearStress->addInterpData(calc->site()->shearStressProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_stressReducCoeff->enabled()) {
        m_stressReducCoeff->addInterpData(calc->site()->stressReducCoeffProfile(
                    calc->surfacePGA()), calc->site()->subLayers(), m_depths);
    }

    if (m_maxShearStrain->enabled()) {
        m_maxShearStrain->addInterpData(calc->site()->maxShearStrainProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_maxAccel->enabled()) {
        m_maxAccel->addInterpData(calc->maxAccelProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_maxVel->enabled()) {
        m_maxVel->addInterpData(calc->maxVelProfile(), calc->site()->subLayers(), m_depths);
    }

    if (m_stressRatio->enabled()) {
        m_stressRatio->addInterpData(calc->site()->stressRatioProfile(), calc->site()->subLayers(), m_depths);
    }



    // Compute the response at the specified locations
    for (int i = 0; i < m_responseLocations.size(); ++i) {
        // FIXME causes memory increase!
        m_responseLocations[i]->saveResults(calc, m_freq->data(), m_period->data(), m_damping);
    }



    // Compute the ratio at the specified locations
    for (int i = 0; i < m_ratioLocations.size(); ++i) {
        // FIXME causes memory increase!
        m_ratioLocations[i]->saveResults(calc, m_freq->data(), m_period->data(), m_damping);
    }
} 

void SiteResponseOutput::removeLastSite()
{
    //
    // Remove the site initial properties
    //
    if (m_initialShearVel->enabled()) {
        m_initialShearVel->removeLast();
    }
    
    if (m_vTotalStress->enabled()) {
        m_vTotalStress->removeLast();
    }

    foreach ( Output * output, m_nlProperties ) {
        output->removeLast();
    }

    // Remove all enabled outputs
    QList<Output*> outputs;
    outputs << m_finalShearVel << m_finalShearMod << m_finalDamping 
        << m_maxError << m_maxShearStress << m_stressReducCoeff 
        << m_maxShearStrain << m_maxAccel << m_maxVel << m_stressRatio;

    // Number of motions 
    const int nExcess = m_seriesEnabled.size() % m_motionCount;

    qDebug() << "Removing" << nExcess << "motions!";

    for (int i = 0; i < nExcess; ++i ) {
        m_seriesEnabled.removeLast();

        foreach (Output * output, outputs ) {
            if (output->enabled()) {
                output->removeLast();
            }
        }

        foreach (ResponseLocation * rl, m_responseLocations) {
            rl->removeLast();
        }
        
        foreach (RatioLocation * rl, m_ratioLocations) {
            rl->removeLast();
        }
    }
}

void SiteResponseOutput::exportData(const QString & path, const QString & separator , const QString & prefix)
{
    // Re-compute that statistics
    computeStats();

    for (int i = 0; i < m_outputList.size(); ++i) {
        if (!m_outputList.at(i)->exportEnabled()) {
            continue;
        }

        if (m_outputList.at(i)->hasMotionSpecificReference()) {
            // Export each time series separately
            for (int j = 0; j < m_motionCount; ++j) {
                m_outputList.at(i)->toTextFile(path, j, separator, prefix);
            }
        } else {
            m_outputList.at(i)->toTextFile(path, -1, separator, prefix);
        }
    }
}

QMap<QString, QVariant> SiteResponseOutput::toMap() const
{
    QMap<QString, QVariant> map;
    // List used in the saving process 
    QList<QVariant> list;

    // Motion names
    for (int i = 0; i < m_motionNames.size(); ++i) {
        list << QVariant(m_motionNames.at(i));
    }

    map.insert("motionNames", list);

    // Series enabled 
    for (int i = 0; i < m_seriesEnabled.size(); ++i) {
        list << QVariant(m_seriesEnabled.at(i));
    }

    map.insert("seriesEnabled", list);

    // Time
    list.clear();
    for (int i = 0; i < m_times.size(); ++i) {
        list << QVariant(Serializer::toVariantList(m_times.at(i).toList()));
    }

    map.insert("times", list);

    // Depths
    map.insert("depths", Serializer::toVariantList(m_depths.toList()));

    // Strain
    list.clear();
    for (int i = 0; i < m_strains.size(); ++i) {
        list << QVariant(Serializer::toVariantList(m_strains.at(i).toList()));
    }

    map.insert("strains", list);

    // Response locations
    list.clear();
    for (int i = 0; i < m_responseLocations.size(); ++i) {
        list << m_responseLocations.at(i)->toMap();
    }

    map.insert("responseLocations", list);
    
    // Ratio locations
    list.clear();
    for (int i = 0; i < m_ratioLocations.size(); ++i) {
        list << m_ratioLocations.at(i)->toMap();
    }

    map.insert("ratioLocations", list);

    // Nonlinear soil properties
    list.clear();
    for (int i = 0; i < m_nlProperties.size(); ++i) {
        list << m_nlProperties.at(i)->toMap();
    }

    map.insert("nlProperties", list);

    // Profiles
    map.insert("initialShearVel", m_initialShearVel->toMap());
    map.insert("finalShearVel", m_finalShearVel->toMap());
    map.insert("finalShearMod", m_finalShearMod->toMap());
    map.insert("finalDamping", m_finalDamping->toMap());
    map.insert("vTotalStress", m_vTotalStress->toMap());
    map.insert("maxError", m_maxError->toMap());
    map.insert("maxShearStress", m_maxShearStress->toMap());
    map.insert("stressReducCoeff", m_stressReducCoeff->toMap());
    map.insert("maxShearStrain", m_maxShearStrain->toMap());
    map.insert("maxAccel", m_maxAccel->toMap());
    map.insert("maxVel", m_maxVel->toMap());
    map.insert("stressRatio", m_stressRatio->toMap());

    // Misc 
	map.insert("title", m_title);
	map.insert("filePrefix", m_filePrefix);
    map.insert("siteCount", m_siteCount);
    map.insert("motionCount", m_motionCount);
    map.insert("totalCount", m_totalCount);
    map.insert("motionCount", m_motionCount);
    map.insert("damping", m_damping);
    map.insert("hasTime", m_hasTime);
    map.insert("hasResults", m_hasResults);
    map.insert("period", m_period->toMap());
    map.insert("freq", m_freq->toMap());
    map.insert("textLog", m_textLog->toMap());

    map.insert("units", Units::instance()->toMap());

    return map;
}

void SiteResponseOutput::fromMap(QMap<QString, QVariant> map)
{
    QList<QVariant> list;
    
    // Motion names
    if (map.contains("motionNames")) {
        m_motionNames.clear();
        list = map.value("motionNames").toList();
        
        for (int i = 0; i < list.size(); ++i)
            m_motionNames << list.at(i).toString();
    }

    if (map.contains("seriesEnabled")){
        m_seriesEnabled.clear();
        list = map.value("seriesEnabled").toList();
        
        for (int i = 0; i < list.size(); ++i)
            m_seriesEnabled << list.at(i).toBool();
    }

    // Time
    if (map.contains("times")) {
        m_times.clear();
        list = map.value("times").toList();
        
        for (int i = 0; i < list.size(); ++i)
            m_times << Serializer::fromVariantList(list.at(i).toList()).toVector();
    }
    
    // Strain
    if (map.contains("strains")) {
        m_strains.clear();
        list = map.value("strains").toList();
        
        for (int i = 0; i < list.size(); ++i)
            m_strains << Serializer::fromVariantList(list.at(i).toList()).toVector();
    }
    
    // Period
    if (map.contains("period")) {
        m_period->fromMap(map.value("period").toMap());
    }
    
    // Frequency
    if (map.contains("freq")) {
        m_freq->fromMap(map.value("freq").toMap());
    }
        
    // Top depths
    if (map.contains("depths"))
        m_depths = Serializer::fromVariantList(map.value("depths").toList()).toVector();
 
    // Response locations
    while (!m_responseLocations.isEmpty())
        delete m_responseLocations.takeFirst();
    
    list = map.value("responseLocations").toList();

    for (int i = 0; i < list.size(); ++i) {
        m_responseLocations << new ResponseLocation;
        m_responseLocations.last()->fromMap(list.at(i).toMap());
    }
    
    emit responseLocationsChanged(); 

    // Ratio locations
    while (!m_ratioLocations.isEmpty())
        delete m_ratioLocations.takeFirst();
    
    list = map.value("ratioLocations").toList();

    for (int i = 0; i < list.size(); ++i) {
        m_ratioLocations << new RatioLocation;
        m_ratioLocations.last()->fromMap(list.at(i).toMap());
    }
    
    emit ratioLocationsChanged(); 
    
    // Nonlinear curves 
    while (!m_nlProperties.isEmpty())
        delete m_nlProperties.takeFirst();
    
    list = map.value("nlProperties").toList();

    for (int i = 0; i < list.size(); ++i) {
        m_nlProperties << new Output;
        m_nlProperties.last()->fromMap(list.at(i).toMap());
    }
    
    emit nlPropertiesChanged(); 

    // Profiles
    m_initialShearVel->fromMap(map.value("initialShearVel").toMap());
    m_finalShearVel->fromMap(map.value("finalShearVel").toMap());
    m_finalShearMod->fromMap(map.value("finalShearMod").toMap());
    m_finalDamping->fromMap(map.value("finalDamping").toMap());
    m_vTotalStress->fromMap(map.value("vTotalStress").toMap());
    m_maxError->fromMap(map.value("maxError").toMap());
    m_maxShearStress->fromMap(map.value("maxShearStress").toMap());
    m_stressReducCoeff->fromMap(map.value("stressReducCoeff").toMap());
    m_maxShearStrain->fromMap(map.value("maxShearStrain").toMap());
    m_maxAccel->fromMap(map.value("maxAccel").toMap());
    m_maxVel->fromMap(map.value("maxVel").toMap());
    m_stressRatio->fromMap(map.value("stressRatio").toMap());
    
    // Misc
    m_title = map.value("title").toString();
    m_filePrefix = map.value("filePrefix").toString();
    m_siteCount = map.value("siteCount").toInt();
    m_motionCount = map.value("motionCount").toInt();
    m_totalCount = map.value("totalCount").toInt();
    m_motionCount = map.value("motionCount").toInt();
    m_damping = map.value("damping").toDouble();
    m_hasTime = map.value("hasTime").toBool();
    m_hasResults = map.value("hasResults").toBool();
    m_textLog->fromMap(map.value("textLog").toMap());

    if (map.contains("units")) {
        Units::instance()->fromMap(map.value("units").toMap());
    }

    // Re-create the output list
    createOutputList();

    // Re-compute statistics
    computeStats();
}

bool SiteResponseOutput::hasResults() const
{
    return m_hasResults;
}

void SiteResponseOutput::initialize(SiteResponseModel::Method method, int siteCount, const QList<Motion*> & motions, const QList<SoilType*> & soilTypes)
{
    // Clear the data vectors
    clear();

    m_hasTime = motions.first()->hasTime();
    // Save the number of sites and motions
    m_siteCount = siteCount;
    m_motionCount = motions.size();

    m_totalCount = m_siteCount * m_motionCount;

    // Save the motions names
    for (int i = 0; i < motions.size(); ++i) {
        m_motionNames << motions.at(i)->toString();
    }

    // Determine the maximum minimum and minimum maximum frequencies
    double freqMin = -1;
    double freqMax = -1;

    for (int i = 0; i < motions.size(); ++i) {
        if (i == 0 || freqMin < motions.at(i)->freqMin()) {
            freqMin = motions.at(i)->freqMin();
        }
        
        if (i == 0 || motions.at(i)->freqMax() < freqMax) {
            freqMax = motions.at(i)->freqMax();
        }
    }

    // Create the frequency and period dimensions
    m_freq->init(true, freqMin, false, freqMax);
    // Don't limit the minimum period
    m_period->init(false, 1 / freqMax, true, 1 / freqMin);

    // If the time is required save it
    if (method == SiteResponseModel::RecordedMotions) {
        bool needsTime = false;
        for (int i = 0; i < m_responseLocations.size(); ++i) {
            if (m_responseLocations.at(i)->needsTime()) {
                needsTime = true;
                break;
            }
        }

        if (needsTime) {
           // Save each of the times
           for (int i = 0; i < motions.size(); ++i)
               m_times << static_cast<const RecordedMotion*>(motions.at(i))->time();
        }

    }

    // Save the strain of each of the nonlinear properties
    int idx = 0;
    for (int i = 0; i < soilTypes.size(); ++i) {
        if (soilTypes.at(i)->saveData()) {
            m_strains << soilTypes.at(i)->normShearMod()->strain().toVector();
            m_nlProperties << new Output(Output::ModulusCurve, idx);
            m_nlProperties.last()->setPrefix(soilTypes.at(i)->name());
            m_nlProperties.last()->setEnabled(true);
            ++idx;
            
            m_strains << soilTypes.at(i)->damping()->strain().toVector();
            m_nlProperties << new Output(Output::DampingCurve, idx);
            m_nlProperties.last()->setPrefix(soilTypes.at(i)->name());
            m_nlProperties.last()->setEnabled(true);
            ++idx;
        }
    }

    // Create the output list
    createOutputList();

    // Mark that there are results
    m_hasResults = true;
}

void SiteResponseOutput::computeDepthVector(double maxDepth)
{
    // Add a point at the surface
    if (m_depths.isEmpty()) {
        m_depths << 0;
    }

    // Amount to increment the depth by
    double increment = 0;

    while (m_depths.last() < maxDepth) {
        if (m_depths.last() < 20) {
            increment = 1;
        } else if (m_depths.last() < 60) {
            increment = 2;
        } else if (m_depths.last() < 160) {
            increment = 5; 
        } else if (m_depths.last() < 360) {
            increment = 10;
        } else {
            increment = 20;
        }

        // Convert the increment to meters
        if (Units::instance()->system() == Units::Metric) {
            increment *=  0.3048;
        }

        // Compute the new depth
        double newDepth = m_depths.last() + increment;

        // Add the depth to the vector
        m_depths << newDepth;

        // If the depth exceeds the maxDepth stop.  The depth must exceed the
        // final depth so that values for the bedrock can be recorded.
        if (maxDepth < newDepth) {
            break;
        }
    }
}
