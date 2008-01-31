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

#include "ResponseLocation.h"
#include "Algorithms.h"
#include "RecordedMotion.h"
#include <QDebug>
#include <QObject>

ResponseLocation::ResponseLocation()
{
    m_type = Motion::Outcrop;
    m_isBaselineCorrected = true;
    m_depth = 0;

    m_respSpec = new Output(Output::ResponseSpectrum);
    m_accelTs = new Output(Output::AccelTimeSeries);
    m_velTs = new Output(Output::VelTimeSeries);
    m_dispTs = new Output(Output::DispTimeSeries);
    m_strainTs = new Output(Output::StrainTimeSeries);
    m_stressTs = new Output(Output::StressTimeSeries);
    
    renameOutputs();
}

double ResponseLocation::depth() const
{
    return m_depth;
}

void ResponseLocation::setDepth(double depth)
{
    m_depth = depth;

    renameOutputs();
}

Motion::Type ResponseLocation::type() const
{
    return m_type;
}

void ResponseLocation::setType(Motion::Type type)
{
    m_type = type;
    
    renameOutputs();
}

bool ResponseLocation::isBaselineCorrected() const
{
    return m_isBaselineCorrected;
}

void ResponseLocation::setBaselineCorrected(bool baselineCorrected)
{
    m_isBaselineCorrected = baselineCorrected;
    
    renameOutputs();
}

Output * ResponseLocation::respSpec()
{
	return m_respSpec;
}

Output * ResponseLocation::accelTs()
{
	return m_accelTs;
}

Output * ResponseLocation::velTs()
{
	return m_velTs;
}

Output * ResponseLocation::dispTs()
{
	return m_dispTs;
}

Output * ResponseLocation::strainTs()
{
	return m_strainTs;
}

Output * ResponseLocation::stressTs()
{
	return m_stressTs;
}

bool ResponseLocation::needsTime() const
{
    if ( m_accelTs->enabled() || m_velTs->enabled() || m_dispTs->enabled()
            || m_strainTs->enabled() || m_stressTs->enabled() )
        return true;
    else 
        return false;
}

bool ResponseLocation::needsPeriod() const
{
    if ( m_respSpec->enabled() )
        return true;
    else 
        return false;
}

void ResponseLocation::clear()
{
    m_respSpec->clear();
    m_accelTs->clear();
    m_velTs->clear();
    m_dispTs->clear();
    m_strainTs->clear();
    m_stressTs->clear();
}

void ResponseLocation::saveResults( const EquivLinearCalc & calc, const QVector<double> & period, double damping )
{
    // Recorded motion used to compute the time series
    RecordedMotion * recordedMotion = static_cast<RecordedMotion*>(calc.motion());

    // Layer that the depth is associated with
    const Location location = calc.site()->depthToLocation(m_depth);
    const Motion::DurationType durationType = (m_depth < 0 ) ? Motion::Rock : Motion::Soil;

    if ( m_respSpec->enabled() || m_accelTs->enabled() || m_velTs->enabled() || m_dispTs->enabled() ) {
        // Compute the acceleration transfer function for the response
        const QVector<std::complex<double> > accelTf = calc.calcAccelTf(
                calc.site()->inputLocation(),calc.motion()->type(),
                location, m_type);

        // Save response spectrum
        if ( m_respSpec->enabled() )
            m_respSpec->addData(calc.motion()->computeSa( durationType, period, damping, accelTf ));

        if ( recordedMotion 
              && ( m_accelTs->enabled() || m_velTs->enabled() || m_dispTs->enabled() )) {


            // Initialize time series
            QVector<double> accel;
            QVector<double> vel;
            QVector<double> disp;

            recordedMotion->timeSeries( accelTf, accel, vel, disp, m_isBaselineCorrected );
        
            // Apply conversion factor to velocity and displacement
            double conv = calc.site()->units()->tsConv();

            for ( int i = 0; i < vel.size(); ++i ) {
                vel[i] *= conv;
                disp[i] *= conv;
            }


            if ( m_accelTs->enabled() )
                m_accelTs->addData( accel );

            if ( m_velTs->enabled() )
                m_velTs->addData(vel);

            if ( m_dispTs->enabled() )
                m_dispTs->addData(disp);

        }

    }

    if ( recordedMotion && ( m_strainTs->enabled() || m_stressTs->enabled())) {
        // Compute the strain transfer function for the response
        QVector<std::complex<double> > strainTf;
        
        calc.calcStrainTf(location, strainTf );

        // Compute the strain time series
        QVector<double> strainTs = recordedMotion->timeSeries(strainTf);

        // The motion is in gravity and needs to be converted to the
        // appropriate units.  The strain is then converted to percent
        double factor = 100 * calc.site()->units()->gravity();

        for (int i = 0; i < strainTs.size(); ++i )
            strainTs[i] *= factor;

        // Use a simple subtraction of the average to correct the record.
        if (m_isBaselineCorrected) {
            // Compute the average
            double sum = 0;

            for (int i = 0; i < strainTs.size(); ++i )
                sum += strainTs.at(i);
            
            double avg = sum / strainTs.size();

            for (int i = 0; i < strainTs.size(); ++i )
                strainTs[i] -= avg;
        }

        // Save strain time history
        if ( m_strainTs->enabled() )
            m_strainTs->addData(strainTs);

        if ( m_stressTs->enabled() ) {
            // Convert from strain to stress using the shear-modulus of the layer
            QVector<double> stressTs(strainTs.size());
            
            double shearMod = calc.site()->shearMod(location.layer());
            
            for (int i = 0; i < stressTs.size(); ++i)
                stressTs[i] = shearMod * strainTs.at(i) / 100;

            // Add the data series
            m_stressTs->addData(stressTs);
        }
    }
}

QMap<QString, QVariant> ResponseLocation::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;

    map.insert("depth", m_depth);
    map.insert("type", m_type);
    map.insert("isBaselineCorrected", m_isBaselineCorrected);

    map.insert("respSpec", m_respSpec->toMap(saveData));
    map.insert("accelTs", m_accelTs->toMap(saveData));
    map.insert("velTs", m_velTs->toMap(saveData));
    map.insert("dispTs", m_dispTs->toMap(saveData));
    map.insert("strainTs", m_strainTs->toMap(saveData));
    map.insert("stressTs", m_stressTs->toMap(saveData));

    return map;
}

void ResponseLocation::fromMap(const QMap<QString, QVariant> & map)
{
    m_depth = map.value("depth").toDouble();
    m_type = (Motion::Type)map.value("type").toInt();
    m_isBaselineCorrected = map.value("isBaselineCorrected").toBool();

    m_respSpec->fromMap(map.value("respSpec").toMap());
    m_accelTs->fromMap(map.value("accelTs").toMap());
    m_velTs->fromMap(map.value("velTs").toMap());
    m_dispTs->fromMap(map.value("dispTs").toMap());
    m_strainTs->fromMap(map.value("strainTs").toMap());
    m_stressTs->fromMap(map.value("stressTs").toMap());
}

void ResponseLocation::renameOutputs()
{
    // Prefix
    QString prefix = QString(QObject::tr("%1 (%2)"))
        .arg(locationToString(m_depth))
        .arg(Motion::typeList().at(m_type));


    // Suffix
    QString suffix;
    if (m_isBaselineCorrected)
        suffix = QObject::tr("corrected");
    else
        suffix = QObject::tr("uncorrected");

    m_respSpec->setPrefix(prefix);
    m_respSpec->setSuffix(suffix);

    m_accelTs->setPrefix(prefix);
    m_accelTs->setSuffix(suffix);

    m_velTs->setPrefix(prefix);
    m_velTs->setSuffix(suffix);

    m_dispTs->setPrefix(prefix);
    m_dispTs->setSuffix(suffix);

    m_strainTs->setPrefix(prefix);
    m_strainTs->setSuffix(suffix);

    m_stressTs->setPrefix(prefix);
    m_stressTs->setSuffix(suffix);
}
