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

#include "AbstractProfileOutput.h"

#include "AbstractCalculator.h"
#include "AbstractMotion.h"
#include "MyQwtCompatibility.h"
#include "LinearOutputInterpolater.h"
#include "OutputCatalog.h"
#include "OutputStatistics.h"
#include "SoilProfile.h"
#include "SubLayer.h"
#include "Units.h"

#include <QDebug>

#include <qwt_scale_engine.h>

AbstractProfileOutput::AbstractProfileOutput(OutputCatalog* catalog, bool interpolated)
    : AbstractOutput(catalog), _enabled(false)
{    
    if (interpolated)
        _interp = new LinearOutputInterpolater;

    _statistics = new OutputStatistics(this);
    connect(_statistics, SIGNAL(wasModified()),
            this, SIGNAL(wasModified()));
}

QString AbstractProfileOutput::fullName() const
{
    return "Profile -- " + name();
}

bool AbstractProfileOutput::enabled() const
{
    return _enabled;
}

void AbstractProfileOutput::setEnabled(bool enabled)
{
    _enabled = enabled;
    emit wasModified();
}

AbstractOutput::CurveType AbstractProfileOutput::curveType() const
{
    return AbstractOutput::Xfy;
}

bool AbstractProfileOutput::needsDepth() const
{
    return true;
}
QString AbstractProfileOutput::fileName(int motion) const
{
    Q_UNUSED(motion);

    return "profile-" + shortName();
}

QwtScaleEngine* AbstractProfileOutput::xScaleEngine() const
{
    return logScaleEngine();
}

QwtScaleEngine* AbstractProfileOutput::yScaleEngine() const
{
    QwtLinearScaleEngine *scaleEngine = new QwtLinearScaleEngine;
    scaleEngine->setAttribute(QwtScaleEngine::Inverted, true);

    return scaleEngine;
}

const QString AbstractProfileOutput::yLabel() const
{
    return tr("Depth (%1)").arg(Units::instance()->length());
}

const QVector<double>& AbstractProfileOutput::ref(int motion) const
{
    Q_UNUSED(motion);
    return _catalog->depth();
}

void AbstractProfileOutput::extract(AbstractCalculator* const calculator,
                         QVector<double> & ref, QVector<double> & data) const
{
    Q_UNUSED(data);
    const QList<SubLayer> & subLayers = calculator->site()->subLayers();

    // Populate the reference with the depth to the top of the layers
    ref.clear();

    foreach (const SubLayer & sl, subLayers)
        ref << sl.depth();

    // Add the depth at the surface of the bedrock
    ref << subLayers.last().depthToBase();
}

void AbstractProfileOutput::extrap(const QVector<double> & ref, QVector<double> & data, double layerThickness) const
{
    Q_ASSERT(ref.size() - 1  == data.size());
    // Compute the slope based on the last two values and extrapolate.
    const int n = ref.size() - 2;
    const double slope = (data.at(n) - data.at(n-1)) /
                   (ref.at(n) - ref.at(n-1));

    data << data.last() + slope * layerThickness / 2.;
}

void AbstractProfileOutput::fromJson(const QJsonObject &json)
{
    AbstractOutput::fromJson(json);
    _enabled = json["enabled"].toBool();
}

QJsonObject AbstractProfileOutput::toJson() const
{
    QJsonObject json = AbstractOutput::toJson();
    json["enabled"] = _enabled;
    return json;
}

QDataStream & operator<< (QDataStream & out, const AbstractProfileOutput* apo)
{
    out << (quint8)1;

    out << apo->_enabled << qobject_cast<const AbstractOutput*>(apo);

    return out;
}

QDataStream & operator>> (QDataStream & in, AbstractProfileOutput* apo)
{
    quint8 ver;
    in >> ver;

    in >> apo->_enabled >> qobject_cast<AbstractOutput*>(apo);

    return in;
}
