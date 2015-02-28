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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "OutputCatalog.h"

#include "AbstractCalculator.h"
#include "AbstractOutput.h"
#include "Dimension.h"
#include "MotionLibrary.h"
#include "ProfilesOutputCatalog.h"
#include "RatiosOutputCatalog.h"
#include "SoilProfile.h"
#include "SoilTypesOutputCatalog.h"
#include "SubLayer.h"
#include "SpectraOutputCatalog.h"
#include "TextLog.h"
#include "TimeSeriesMotion.h"
#include "TimeSeriesOutputCatalog.h"
#include "Units.h"

#include <boost/lexical_cast.hpp>

OutputCatalog::OutputCatalog(QObject *parent) :
    QAbstractTableModel(parent), m_selectedOutput(0)
{    
    m_log = new TextLog(this);
    connect(m_log, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    m_frequency = new Dimension(this);
    m_frequency->setMin(0.05);
    m_frequency->setMax(100);
    m_frequency->setSize(512);
    m_frequency->setSpacing(Dimension::Log);
    m_frequencyIsNeeded = false;

    m_damping = 5.;
    m_period = new Dimension(this);
    m_period->setMin(0.01);
    m_period->setMax(10.0);
    m_period->setSize(91);
    m_period->setSpacing(Dimension::Log);
    m_periodIsNeeded = false;

    m_profilesOutputCatalog = new ProfilesOutputCatalog(this);
    connect(m_profilesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    m_catalogs << m_profilesOutputCatalog;

    m_ratiosOutputCatalog = new RatiosOutputCatalog(this);
    connect(m_ratiosOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(m_ratiosOutputCatalog, SIGNAL(periodIsNeededChanged(bool)),
            this, SLOT(setPeriodIsNeeded(bool)));
    connect(m_ratiosOutputCatalog, SIGNAL(frequencyIsNeededChanged(bool)),
            this, SLOT(setFrequencyIsNeeded(bool)));

    m_catalogs << m_ratiosOutputCatalog;

    m_soilTypesOutputCatalog = new SoilTypesOutputCatalog(this);
    connect(m_soilTypesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    m_catalogs << m_soilTypesOutputCatalog;

    m_spectraOutputCatalog = new SpectraOutputCatalog(this);
    connect(m_spectraOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(m_spectraOutputCatalog, SIGNAL(frequencyIsNeededChanged(bool)),
            this, SLOT(setFrequencyIsNeeded(bool)));
    connect(m_spectraOutputCatalog, SIGNAL(periodIsNeededChanged(bool)),
            this, SLOT(setPeriodIsNeeded(bool)));

    m_catalogs << m_spectraOutputCatalog;

    m_timeSeriesOutputCatalog = new TimeSeriesOutputCatalog(this);
    connect(m_timeSeriesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(m_timeSeriesOutputCatalog, SIGNAL(timesAreNeededChanged(bool)),
            this, SLOT(setTimesAreNeeded(bool)));

    m_catalogs << m_timeSeriesOutputCatalog;

}

int OutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    if (m_selectedOutput) {
        return m_selectedOutput->motionCount() * m_selectedOutput->siteCount();
    } else {
        return 0;
    }
}

int OutputCatalog::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 3;
}

QVariant OutputCatalog::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    int site;
    int motion;
    m_selectedOutput->intToSiteMotion(index.row(), &site, &motion);

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case SiteColumn:
            return site;
        case MotionColumn:
            return m_motionNames.at(motion);
        case EnabledColumn:
        default:
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole && index.column() == EnabledColumn) {
        if (site < m_enabled.size() && motion < m_enabled.at(site).size())
            return m_enabled.at(site).at(motion) ?
                    Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

bool OutputCatalog::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.parent()!=QModelIndex())
        return false;

    if (index.column() == EnabledColumn && role == Qt::CheckStateRole) {
        const bool b = value.toBool();
        int site;
        int motion;
        m_selectedOutput->intToSiteMotion(index.row(), &site, &motion);

        // Adjust what is changed based on what is currently shown. If the
        // initially shear-wave velocity profile (which is motionIndepedent) is
        // currently active and the user disables the site, then all of those
        // sites should be disabled.
        if (m_selectedOutput->motionIndependent()) {
            setSiteEnabled(site, b);
        } else if (m_selectedOutput->siteIndependent()) {
            setMotionEnabled(motion, b);
        } else {
            m_enabled[site][motion] = b;
        }

        emit enabledChanged(index.row());
    }

    emit dataChanged(index, index);
    emit wasModified();

    return true;
}

QVariant OutputCatalog::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case EnabledColumn:
            return tr("Enabled");
        case SiteColumn:
            return tr("Site");
        case MotionColumn:
            return tr("Motion");
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

Qt::ItemFlags OutputCatalog::flags(const QModelIndex & index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == EnabledColumn) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

ProfilesOutputCatalog* OutputCatalog::profilesCatalog()
{
    return m_profilesOutputCatalog;
}

RatiosOutputCatalog* OutputCatalog::ratiosCatalog()
{
    return m_ratiosOutputCatalog;
}

SoilTypesOutputCatalog* OutputCatalog::soilTypesCatalog()
{
    return m_soilTypesOutputCatalog;
}

SpectraOutputCatalog* OutputCatalog::spectraCatalog()
{
    return m_spectraOutputCatalog;
}

TimeSeriesOutputCatalog* OutputCatalog::timeSeriesCatalog()
{
    return m_timeSeriesOutputCatalog;
}

const QString& OutputCatalog::title() const
{
    return m_title;
}

void OutputCatalog::setTitle(const QString &title)
{
    m_title = title;

    emit wasModified();
}

const QString& OutputCatalog::filePrefix() const
{
    return m_filePrefix;
}

void OutputCatalog::setFilePrefix(const QString &prefix)
{
    m_filePrefix = prefix;

    emit wasModified();
}

AbstractOutput* OutputCatalog::setSelectedOutput(int index)
{
    Q_ASSERT(index >= 0 && index < m_outputs.size());

    m_selectedOutput = m_outputs[index];
    reset();

    return m_selectedOutput;
}

TextLog* OutputCatalog::log()
{
    return m_log;
}

const QVector<double>& OutputCatalog::depth() const
{
    return m_depth;
}

const QVector<double>& OutputCatalog::time(int motion) const
{
    return m_time.at(motion);
}

Dimension* OutputCatalog::frequency()
{
    return m_frequency;
}

Dimension* OutputCatalog::period()
{
    return m_period;
}

double OutputCatalog::damping() const
{
    return m_damping;
}

void OutputCatalog::setDamping(double damping)
{
    m_damping = damping;

    emit wasModified();
}

int OutputCatalog::motionCount() const
{
    return m_motionCount;
}

int OutputCatalog::siteCount() const
{
    return m_siteCount;
}

bool OutputCatalog::enabledAt(int site, int motion) const
{
    return m_enabled.at(site).at(motion);
}

bool OutputCatalog::enabledAt(int row) const
{
    Q_ASSERT(m_selectedOutput);

    int motion;
    int site;
    m_selectedOutput->intToSiteMotion(row, &site, &motion);

    return m_enabled.at(site).at(motion);
}

bool OutputCatalog::siteEnabled(int row) const
{
    Q_ASSERT(m_selectedOutput);

    const int site = m_selectedOutput->intToSite(row);

    if (site >= m_enabled.size())
        return false;

    for (int i = 0; i < m_motionCount; ++i) {
        if (!m_enabled.at(site).at(i))
            return false;
    }

    return true;
}

void OutputCatalog::setSiteEnabled(int row, bool enabled)
{
    Q_ASSERT(m_selectedOutput);

    const int site = m_selectedOutput->intToSite(row);

    for (int i = 0; i < m_motionCount; ++i)
        m_enabled[site][i] = enabled;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit wasModified();
}

bool OutputCatalog::motionEnabled(int row) const
{
    Q_ASSERT(m_selectedOutput);

    const int motion = m_selectedOutput->intToMotion(row);

    for (int i = 0; i < m_siteCount; ++i) {
        if (!m_enabled.at(i).at(motion))
            return false;
    }

    return true;
}

void OutputCatalog::setMotionEnabled(int row, bool enabled)
{
    Q_ASSERT(m_selectedOutput);

    const int motion = m_selectedOutput->intToMotion(row);

    for (int i = 0; i < m_siteCount; ++i)
        m_enabled[i][motion] = enabled;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit wasModified();
}

int OutputCatalog::siteNumberAt(int row) const
{
    Q_ASSERT(m_selectedOutput);

    return m_selectedOutput->intToSite(row);
}

const QString OutputCatalog::motionNameAt(int row) const
{
    Q_ASSERT(m_selectedOutput);

    int site;
    int motion;
    m_selectedOutput->intToSiteMotion(row, &site, &motion);

    return m_motionNames.at(motion);
}


void OutputCatalog::initialize(int siteCount, MotionLibrary* motionLibrary)
{
    // Create a list of all enabled outputs
    m_outputs.clear();
    foreach (AbstractOutputCatalog* catalog, m_catalogs) {
        foreach (AbstractOutput* output, catalog->outputs()) {
            if (!output->needsTime() ||
                (output->needsTime()
                 && motionLibrary->approach() == MotionLibrary::TimeSeries)) {
                connect(output, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
                m_outputs << output;
            }
        }
    }

    m_siteCount = siteCount;

    m_motionNames.clear();
    m_time.clear();
    // Generate a list of names of enabled motions
    for (int i = 0; i < motionLibrary->rowCount(); ++i) {
        if (motionLibrary->motionAt(i)->enabled()) {
            m_motionNames << motionLibrary->motionAt(i)->name();

            if (TimeSeriesMotion* tsm = qobject_cast<TimeSeriesMotion*>(
                    motionLibrary->motionAt(i)))
                m_time << tsm->time();
        }
    }
    m_motionCount = m_motionNames.size();

    m_enabled.clear();
    for (int i = 0; i < m_siteCount; ++i) {
        m_enabled << QList<bool>();
        for (int j = 0; j < m_motionCount; ++j)
            m_enabled.last() << true;
    }

    m_period->init();
    m_frequency->init();
}

void OutputCatalog::clear()
{
    m_log->clear();
    m_depth.clear();
    m_frequency->clear();
    m_period->clear();
    m_time.clear();
    m_motionNames.clear();
    m_enabled.clear();

    // Need to loop over the catalogs as m_outputs my have previously deleted pointers
    foreach (AbstractOutputCatalog* catalog, m_catalogs) {
        foreach (AbstractOutput* output, catalog->outputs()) {
            output->clear();
        }
    }

    emit wasModified();
}

void OutputCatalog::finalize()
{   
    foreach (AbstractOutput* output, m_outputs)
        output->finalize();

    emit wasModified();
}

void OutputCatalog::setReadOnly(bool readOnly)
{
    foreach (AbstractOutputCatalog* catalog, m_catalogs)
        catalog->setReadOnly(readOnly);
}


void OutputCatalog::saveResults(int motion, AbstractCalculator* const calculator)
{
    // Need to populate the depth vector based on the depth to the last
    // sublayer. These depths are updated as the velocity profile is varied.
    populateDepthVector(
                calculator->site()->subLayers().last().depthToBase());

    foreach (AbstractOutput* output, m_outputs) {
        output->addData(motion, calculator);
    }
}

void OutputCatalog::removeLastSite()
{
    foreach (AbstractOutput* output, m_outputs)
        output->removeLastSite();
}

bool OutputCatalog::timesAreNeeded() const
{
    return m_timesAreNeeded;
}

void OutputCatalog::setTimesAreNeeded(bool timesAreNeeded)
{
    m_timesAreNeeded = timesAreNeeded;

    emit timesAreNeededChanged(m_timesAreNeeded);
}

bool OutputCatalog::periodIsNeeded() const
{
    return m_periodIsNeeded;
}

void OutputCatalog::setPeriodIsNeeded(bool periodIsNeeded)
{
    m_periodIsNeeded = periodIsNeeded;

    emit periodIsNeededChanged(m_periodIsNeeded);
}

bool OutputCatalog::frequencyIsNeeded() const
{
    return m_frequencyIsNeeded;
}

void OutputCatalog::setFrequencyIsNeeded(bool frequencyIsNeeded)
{
    m_frequencyIsNeeded = frequencyIsNeeded;

    emit frequencyIsNeededChanged(m_frequencyIsNeeded);
}

QStringList OutputCatalog::outputNames() const
{
    QStringList list;

    foreach (AbstractOutput* output, m_outputs)
        list << output->fullName();

    return list;
}

const QList<AbstractOutput*> & OutputCatalog::outputs() const
{
    return m_outputs;
}

void OutputCatalog::exportData(const QString &path, const QString &separator, const QString &prefix)
{
    foreach (AbstractOutput* output, m_outputs) {
        if (output->exportEnabled())
            output->exportData(path, separator, prefix);
    }
}

void OutputCatalog::populateDepthVector(double maxDepth)
{
    // Add a point at the surface
    if (m_depth.isEmpty()) {
        m_depth << 0;
    }

    // Amount to increment the depth by
    double increment = 0;

    // If the depth exceeds the maxDepth stop.  The depth must exceed the
    // final depth so that values for the bedrock can be recorded.
    while (m_depth.last() < maxDepth) {
        if (m_depth.last() < 20) {
            increment = 1;
        } else if (m_depth.last() < 60) {
            increment = 2;
        } else if (m_depth.last() < 160) {
            increment = 5;
        } else if (m_depth.last() < 360) {
            increment = 10;
        } else {
            increment = 20;
        }

        // Convert the increment to meters
        if (Units::instance()->system() == Units::Metric)
            increment *=  0.3048;

        // Add the depth to the vector
        m_depth << m_depth.last() + increment;
    }
}

void OutputCatalog::ptRead(const ptree &pt)
{
    beginResetModel();

    m_title = QString::fromStdString(pt.get<std::string>("title"));
    m_filePrefix = QString::fromStdString(pt.get<std::string>("filePrefix"));

    ptree list = pt.get_child("enabled");
    foreach(const ptree::value_type &e, list)
    {
        QList<bool> qb = QList<bool>();
        foreach(const ptree::value_type &b, e.second)
        {
            if (boost::lexical_cast<std::string>(b.second.data()) == "true")
            {
                qb.push_back(true);
            }
            else
            {
                qb.push_back(false);
            }
        }
        m_enabled << qb;
    }

    ptree frequency = pt.get_child("frequency");
    m_frequency->ptRead(frequency);
    m_frequencyIsNeeded = pt.get<bool>("frequencyIsNeeded");

    ptree period = pt.get_child("period");
    m_period->ptRead(period);
    m_periodIsNeeded = pt.get<bool>("periodIsNeeded");

    m_damping = pt.get<double>("damping");

    ptree profilesOutputCatalog = pt.get_child("profilesOutputCatalog");
    m_profilesOutputCatalog->ptRead(profilesOutputCatalog);

    ptree ratiosOutputCatalog = pt.get_child("ratiosOutputCatalog");
    m_ratiosOutputCatalog->ptRead(ratiosOutputCatalog);

    ptree soilTypesOutputCatalog = pt.get_child("soilTypesOutputCatalog");
    m_soilTypesOutputCatalog->ptRead(soilTypesOutputCatalog);

    ptree spectraOutputCatalog = pt.get_child("spectraOutputCatalog");
    m_spectraOutputCatalog->ptRead(spectraOutputCatalog);

    ptree timeSeriesOutputCatalog = pt.get_child("timeSeriesOutputCatalog");
    m_timeSeriesOutputCatalog->ptRead(timeSeriesOutputCatalog);

    ptree log = pt.get_child("log");
    m_log->ptRead(log);

    double depth = pt.get<double>("depth");
    if (depth > 0)
    {
        populateDepthVector(depth);
    }



    endResetModel();
}

void OutputCatalog::ptWrite(ptree &pt) const
{
    pt.put<std::string>("title", m_title.toStdString());
    pt.put<std::string>("filePrefix", m_filePrefix.toStdString());

    ptree enabled;
    foreach(const QList<bool> & l, m_enabled)
    {
        ptree el;
        foreach(const bool & b, l)
        {
            ptree val;
            val.put("", b);
            el.push_back(std::make_pair("", val));
        }
        enabled.push_back(std::make_pair("", el));
    }
    pt.add_child("enabled", enabled);

    ptree frequency;
    m_frequency->ptWrite(frequency);
    pt.add_child("frequency", frequency);
    pt.put("frequencyIsNeeded", m_frequencyIsNeeded);

    ptree period;
    m_period->ptWrite(period);
    pt.add_child("period", period);
    pt.put("periodIsNeeded", m_periodIsNeeded);

    pt.put("damping", m_damping);

    ptree profilesOutputCatalog;
    m_profilesOutputCatalog->ptWrite(profilesOutputCatalog);
    pt.add_child("profilesOutputCatalog", profilesOutputCatalog);

    ptree ratiosOutputCatalog;
    m_ratiosOutputCatalog->ptWrite(ratiosOutputCatalog);
    pt.add_child("ratiosOutputCatalog", ratiosOutputCatalog);

    ptree soilTypesOutputCatalog;
    m_soilTypesOutputCatalog->ptWrite(soilTypesOutputCatalog);
    pt.add_child("soilTypesOutputCatalog", soilTypesOutputCatalog);

    ptree spectraOutputCatalog;
    m_spectraOutputCatalog->ptWrite(spectraOutputCatalog);
    pt.add_child("spectraOutputCatalog", spectraOutputCatalog);

    ptree timeSeriesOutputCatalog;
    m_timeSeriesOutputCatalog->ptWrite(timeSeriesOutputCatalog);
    pt.add_child("timeSeriesOutputCatalog", timeSeriesOutputCatalog);

    ptree log;
    m_log->ptWrite(log);
    pt.add_child("log", log);

    m_depth.size() ? pt.put("depth", m_depth.last()) : pt.put("depth", -1);  
}

QDataStream & operator<< (QDataStream & out, const OutputCatalog* oc)
{
    out << (quint8)1;

    out
            << oc->m_title
            << oc->m_filePrefix
            << oc->m_enabled
            << oc->m_frequency
            << oc->m_frequencyIsNeeded
            << oc->m_period
            << oc->m_periodIsNeeded
            << oc->m_damping
            << oc->m_profilesOutputCatalog
            << oc->m_ratiosOutputCatalog
            << oc->m_soilTypesOutputCatalog
            << oc->m_spectraOutputCatalog
            << oc->m_timeSeriesOutputCatalog
            << oc->m_log
            << (oc->m_depth.size() ? oc->m_depth.last() : -1);

    return out;
}

QDataStream & operator>> (QDataStream & in, OutputCatalog* oc)
{
    quint8 ver;
    in >> ver;

    double maxDepth;

    oc->beginResetModel();

    in >> oc->m_title;
    in >> oc->m_filePrefix;
    in >> oc->m_enabled;
    in >> oc->m_frequency;
    in >> oc->m_frequencyIsNeeded;
    in >> oc->m_period;
    in >> oc->m_periodIsNeeded;
    in >> oc->m_damping;
    in >> oc->m_profilesOutputCatalog;
    in >> oc->m_ratiosOutputCatalog;
    in >> oc->m_soilTypesOutputCatalog;
    in >> oc->m_spectraOutputCatalog;
    in >> oc->m_timeSeriesOutputCatalog;
    in >> oc->m_log;
    in >> maxDepth;

    if (maxDepth > 0)
        oc->populateDepthVector(maxDepth);

    oc->endResetModel();
    return in;
}
