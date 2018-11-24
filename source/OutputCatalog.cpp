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

#include <QJsonArray>
#include <QJsonValue>

OutputCatalog::OutputCatalog(QObject *parent) :
    QAbstractTableModel(parent), _selectedOutput(0)
{    
    _log = new TextLog(this);
    connect(_log, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));

    _frequency = new Dimension(this);
    _frequency->setMin(0.05);
    _frequency->setMax(100);
    _frequency->setSize(512);
    _frequency->setSpacing(Dimension::Log);
    _frequencyIsNeeded = false;

    _damping = 5.;
    _period = new Dimension(this);
    _period->setMin(0.01);
    _period->setMax(10.0);
    _period->setSize(91);
    _period->setSpacing(Dimension::Log);
    _periodIsNeeded = false;

    _profilesOutputCatalog = new ProfilesOutputCatalog(this);
    connect(_profilesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    _catalogs << _profilesOutputCatalog;

    _ratiosOutputCatalog = new RatiosOutputCatalog(this);
    connect(_ratiosOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(_ratiosOutputCatalog, SIGNAL(periodIsNeededChanged(bool)),
            this, SLOT(setPeriodIsNeeded(bool)));
    connect(_ratiosOutputCatalog, SIGNAL(frequencyIsNeededChanged(bool)),
            this, SLOT(setFrequencyIsNeeded(bool)));

    _catalogs << _ratiosOutputCatalog;

    _soilTypesOutputCatalog = new SoilTypesOutputCatalog(this);
    connect(_soilTypesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    _catalogs << _soilTypesOutputCatalog;

    _spectraOutputCatalog = new SpectraOutputCatalog(this);
    connect(_spectraOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(_spectraOutputCatalog, SIGNAL(frequencyIsNeededChanged(bool)),
            this, SLOT(setFrequencyIsNeeded(bool)));
    connect(_spectraOutputCatalog, SIGNAL(periodIsNeededChanged(bool)),
            this, SLOT(setPeriodIsNeeded(bool)));

    _catalogs << _spectraOutputCatalog;

    _timeSeriesOutputCatalog = new TimeSeriesOutputCatalog(this);
    connect(_timeSeriesOutputCatalog, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
    connect(_timeSeriesOutputCatalog, SIGNAL(timesAreNeededChanged(bool)),
            this, SLOT(setTimesAreNeeded(bool)));

    _catalogs << _timeSeriesOutputCatalog;

}

int OutputCatalog::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    if (_selectedOutput) {
        return _selectedOutput->motionCount() * _selectedOutput->siteCount();
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
    _selectedOutput->intToSiteMotion(index.row(), &site, &motion);

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case SiteColumn:
            return site;
        case MotionColumn:
            return _motionNames.at(motion);
        case EnabledColumn:
        default:
            return QVariant();
        }
    } else if (role == Qt::CheckStateRole && index.column() == EnabledColumn) {
        if (site < _enabled.size() && motion < _enabled.at(site).size())
            return _enabled.at(site).at(motion) ?
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
        _selectedOutput->intToSiteMotion(index.row(), &site, &motion);

        // Adjust what is changed based on what is currently shown. If the
        // initially shear-wave velocity profile (which is motionIndepedent) is
        // currently active and the user disables the site, then all of those
        // sites should be disabled.
        if (_selectedOutput->motionIndependent()) {
            setSiteEnabled(site, b);
        } else if (_selectedOutput->siteIndependent()) {
            setMotionEnabled(motion, b);
        } else {
            _enabled[site][motion] = b;
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
    return _profilesOutputCatalog;
}

RatiosOutputCatalog* OutputCatalog::ratiosCatalog()
{
    return _ratiosOutputCatalog;
}

SoilTypesOutputCatalog* OutputCatalog::soilTypesCatalog()
{
    return _soilTypesOutputCatalog;
}

SpectraOutputCatalog* OutputCatalog::spectraCatalog()
{
    return _spectraOutputCatalog;
}

TimeSeriesOutputCatalog* OutputCatalog::timeSeriesCatalog()
{
    return _timeSeriesOutputCatalog;
}

const QString& OutputCatalog::title() const
{
    return _title;
}

void OutputCatalog::setTitle(const QString &title)
{
    _title = title;

    emit wasModified();
}

const QString& OutputCatalog::filePrefix() const
{
    return _filePrefix;
}

void OutputCatalog::setFilePrefix(const QString &prefix)
{
    _filePrefix = prefix;

    emit wasModified();
}

AbstractOutput* OutputCatalog::setSelectedOutput(int index)
{
    Q_ASSERT(index >= 0 && index < _outputs.size());
    beginResetModel();
    _selectedOutput = _outputs[index];
    endResetModel();
    return _selectedOutput;
}

TextLog* OutputCatalog::log()
{
    return _log;
}

const QVector<double>& OutputCatalog::depth() const
{
    return _depth;
}

const QVector<double>& OutputCatalog::time(int motion) const
{
    return _time.at(motion);
}

Dimension* OutputCatalog::frequency()
{
    return _frequency;
}

Dimension* OutputCatalog::period()
{
    return _period;
}

double OutputCatalog::damping() const
{
    return _damping;
}

void OutputCatalog::setDamping(double damping)
{
    _damping = damping;

    emit wasModified();
}

int OutputCatalog::motionCount() const
{
    return _motionCount;
}

int OutputCatalog::siteCount() const
{
    return _siteCount;
}

bool OutputCatalog::enabledAt(int site, int motion) const
{
    return _enabled.at(site).at(motion);
}

bool OutputCatalog::enabledAt(int row) const
{
    Q_ASSERT(_selectedOutput);

    int motion;
    int site;
    _selectedOutput->intToSiteMotion(row, &site, &motion);

    return _enabled.at(site).at(motion);
}

bool OutputCatalog::siteEnabled(int row) const
{
    Q_ASSERT(_selectedOutput);

    const int site = _selectedOutput->intToSite(row);

    if (site >= _enabled.size())
        return false;

    for (int i = 0; i < _motionCount; ++i) {
        if (!_enabled.at(site).at(i))
            return false;
    }

    return true;
}

void OutputCatalog::setSiteEnabled(int row, bool enabled)
{
    Q_ASSERT(_selectedOutput);

    const int site = _selectedOutput->intToSite(row);

    for (int i = 0; i < _motionCount; ++i)
        _enabled[site][i] = enabled;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit wasModified();
}

bool OutputCatalog::motionEnabled(int row) const
{
    Q_ASSERT(_selectedOutput);

    const int motion = _selectedOutput->intToMotion(row);

    for (int i = 0; i < _siteCount; ++i) {
        if (!_enabled.at(i).at(motion))
            return false;
    }

    return true;
}

void OutputCatalog::setMotionEnabled(int row, bool enabled)
{
    Q_ASSERT(_selectedOutput);

    const int motion = _selectedOutput->intToMotion(row);

    for (int i = 0; i < _siteCount; ++i)
        _enabled[i][motion] = enabled;

    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    emit wasModified();
}

int OutputCatalog::siteNumberAt(int row) const
{
    Q_ASSERT(_selectedOutput);

    return _selectedOutput->intToSite(row);
}

const QString OutputCatalog::motionNameAt(int row) const
{
    Q_ASSERT(_selectedOutput);

    int site;
    int motion;
    _selectedOutput->intToSiteMotion(row, &site, &motion);

    return _motionNames.at(motion);
}


void OutputCatalog::initialize(int siteCount, MotionLibrary* motionLibrary)
{
    // Create a list of all enabled outputs
    _outputs.clear();
    for (auto *catalog : _catalogs) {
        for (auto *output : catalog->outputs()) {
            if (!output->needsTime()
                || (output->needsTime()
                    && motionLibrary->approach() == MotionLibrary::TimeSeries)) {
                connect(output, SIGNAL(wasModified()), this, SIGNAL(wasModified()));
                _outputs << output;
            }
        }
    }

    _siteCount = siteCount;

    _motionNames.clear();
    _time.clear();
    // Generate a list of names of enabled motions
    for (int i = 0; i < motionLibrary->rowCount(); ++i) {
        if (motionLibrary->motionAt(i)->enabled()) {
            _motionNames << motionLibrary->motionAt(i)->name();

            if (auto *tsm = qobject_cast<TimeSeriesMotion *>(motionLibrary->motionAt(i)))
                _time << tsm->time();
        }
    }
    _motionCount = _motionNames.size();

    _enabled.clear();
    for (int i = 0; i < _siteCount; ++i) {
        _enabled << QList<bool>();
        for (int j = 0; j < _motionCount; ++j)
            _enabled.last() << true;
    }

    _period->init();
    _frequency->init();
}

void OutputCatalog::clear()
{
    _log->clear();
    _depth.clear();
    _frequency->clear();
    _period->clear();
    _time.clear();
    _motionNames.clear();
    _enabled.clear();

    // Need to loop over the catalogs as _outputs my have previously deleted pointers
    for (auto *catalog : _catalogs) {
        for (auto *output : catalog->outputs()) {
            output->clear();
        }
    }

    emit wasModified();
}

void OutputCatalog::finalize()
{
    for (AbstractOutput *output : _outputs)
        output->finalize();

    emit wasModified();
}

void OutputCatalog::setReadOnly(bool readOnly)
{
    for (AbstractOutputCatalog *catalog : _catalogs)
        catalog->setReadOnly(readOnly);
}

void OutputCatalog::saveResults(int motion, AbstractCalculator* const calculator)
{
    // Need to populate the depth vector based on the depth to the last
    // sublayer. These depths are updated as the velocity profile is varied.
    populateDepthVector(
                calculator->site()->subLayers().last().depthToBase());

    for (AbstractOutput *output : _outputs) {
        output->addData(motion, calculator);
    }
}

void OutputCatalog::removeLastSite()
{
    for (AbstractOutput *output : _outputs)
        output->removeLastSite();
}

bool OutputCatalog::timesAreNeeded() const
{
    return _timesAreNeeded;
}

void OutputCatalog::setTimesAreNeeded(bool timesAreNeeded)
{
    _timesAreNeeded = timesAreNeeded;

    emit timesAreNeededChanged(_timesAreNeeded);
}

bool OutputCatalog::periodIsNeeded() const
{
    return _periodIsNeeded;
}

void OutputCatalog::setPeriodIsNeeded(bool periodIsNeeded)
{
    _periodIsNeeded = periodIsNeeded;

    emit periodIsNeededChanged(_periodIsNeeded);
}

bool OutputCatalog::frequencyIsNeeded() const
{
    return _frequencyIsNeeded;
}

void OutputCatalog::setFrequencyIsNeeded(bool frequencyIsNeeded)
{
    _frequencyIsNeeded = frequencyIsNeeded;

    emit frequencyIsNeededChanged(_frequencyIsNeeded);
}

QStringList OutputCatalog::outputNames() const
{
    QStringList list;

    for (AbstractOutput *output : _outputs)
        list << output->fullName();

    return list;
}

const QList<AbstractOutput*> & OutputCatalog::outputs() const
{
    return _outputs;
}

void OutputCatalog::exportData(const QString &path, const QString &separator, const QString &prefix)
{
    for (AbstractOutput *output : _outputs) {
        if (output->exportEnabled())
            output->exportData(path, separator, prefix);
    }
}

void OutputCatalog::populateDepthVector(double maxDepth)
{
    // Add a point at the surface
    if (_depth.isEmpty()) {
        _depth << 0;
    }

    // Amount to increment the depth by
    double increment = 0;

    // If the depth exceeds the maxDepth stop.  The depth must exceed the
    // final depth so that values for the bedrock can be recorded.
    while (_depth.last() < maxDepth) {
        if (_depth.last() < 20) {
            increment = 1;
        } else if (_depth.last() < 60) {
            increment = 2;
        } else if (_depth.last() < 160) {
            increment = 5;
        } else if (_depth.last() < 360) {
            increment = 10;
        } else {
            increment = 20;
        }

        // Convert the increment to meters
        if (Units::instance()->system() == Units::Metric)
            increment *=  0.3048;

        // Add the depth to the vector
        _depth << _depth.last() + increment;
    }
}

void OutputCatalog::fromJson(const QJsonObject &json)
{
    beginResetModel();

    _title = json["title"].toString();
    _filePrefix = json["filePrefix"].toString();
    _frequencyIsNeeded = json["frequencyIsNeeded"].toBool();
    _frequency->fromJson(json["frequency"].toObject());
    _periodIsNeeded = json["periodIsNeeded"].toBool();
    _period->fromJson(json["period"].toObject());
    _damping = json["damping"].toDouble();

    _log->fromJson(json["log"].toObject());
    _profilesOutputCatalog->fromJson(json["profilesOutputCatalog"].toArray());
    _ratiosOutputCatalog->fromJson(json["ratiosOutputCatalog"].toArray());
    _soilTypesOutputCatalog->fromJson(json["soilTypesOutputCatalog"].toArray());
    _spectraOutputCatalog->fromJson(json["spectraOutputCatalog"].toArray());
    _timeSeriesOutputCatalog->fromJson(json["timeSeriesOutputCatalog"].toArray());

    double depth = json["depth"].toDouble();
    if (depth > 0) {
        populateDepthVector(depth);
    }

    _enabled.clear();
    for (const QJsonValue &value : json["enabled"].toArray()) {
        QList<bool> l;
        for (const QJsonValue &v : value.toArray())
            l << v.toBool();
        _enabled << l;
    }

    endResetModel();
}

QJsonObject OutputCatalog::toJson() const
{
    QJsonObject json;
    json["title"] = _title;
    json["filePrefix"] = _filePrefix;
    json["frequencyIsNeeded"] = _frequencyIsNeeded;
    json["frequency"] = _frequency->toJson();
    json["periodIsNeeded"] = _periodIsNeeded;
    json["period"] = _period->toJson();
    json["damping"] = _damping;
    json["log"] = _log->toJson();

    json["profilesOutputCatalog"] = _profilesOutputCatalog->toJson();
    json["ratiosOutputCatalog"] = _ratiosOutputCatalog->toJson();
    json["soilTypesOutputCatalog"] = _soilTypesOutputCatalog->toJson();
    json["spectraOutputCatalog"] = _spectraOutputCatalog->toJson();
    json["timeSeriesOutputCatalog"] = _timeSeriesOutputCatalog->toJson();

    json["depth"] = _depth.size() ? _depth.last() : -1;

    QJsonArray enabled;
    for (const QList<bool> &l : _enabled) {
        QJsonArray qja;
        for (const bool &b : l)
            qja << QJsonValue(b);

        enabled << qja;
    }
    json["enabled"] = enabled;

    return json;
}


QDataStream & operator<< (QDataStream & out, const OutputCatalog* oc)
{
    out << (quint8)1;

    out
            << oc->_title
            << oc->_filePrefix
            << oc->_enabled
            << oc->_frequency
            << oc->_frequencyIsNeeded
            << oc->_period
            << oc->_periodIsNeeded
            << oc->_damping
            << oc->_profilesOutputCatalog
            << oc->_ratiosOutputCatalog
            << oc->_soilTypesOutputCatalog
            << oc->_spectraOutputCatalog
            << oc->_timeSeriesOutputCatalog
            << oc->_log
            << (oc->_depth.size() ? oc->_depth.last() : -1);

    return out;
}

QDataStream & operator>> (QDataStream & in, OutputCatalog* oc)
{
    quint8 ver;
    in >> ver;

    double maxDepth;

    oc->beginResetModel();

    in >> oc->_title;
    in >> oc->_filePrefix;
    in >> oc->_enabled;
    in >> oc->_frequency;
    in >> oc->_frequencyIsNeeded;
    in >> oc->_period;
    in >> oc->_periodIsNeeded;
    in >> oc->_damping;
    in >> oc->_profilesOutputCatalog;
    in >> oc->_ratiosOutputCatalog;
    in >> oc->_soilTypesOutputCatalog;
    in >> oc->_spectraOutputCatalog;
    in >> oc->_timeSeriesOutputCatalog;
    in >> oc->_log;
    in >> maxDepth;

    if (maxDepth > 0)
        oc->populateDepthVector(maxDepth);

    oc->endResetModel();
    return in;
}
