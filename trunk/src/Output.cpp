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

#include "Output.h"
#include "SiteResponseOutput.h"
#include "Serializer.h"

#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <gsl/gsl_interp.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

#include <cmath>

Output::Output( Output::Type type, int refIndex )
    : m_type(type), m_refIndex(refIndex)
{
    m_enabled = false;
    m_exportEnabled = true;

    m_parent = 0;
    
    m_prefix = "";
    m_suffix = "";
}

void Output::reset()
{
    m_enabled = false;
    m_exportEnabled = true;
    m_refIndex = -1;

    m_prefix = "";
    m_suffix = "";

    m_average.clear();
    m_stdev.clear();

    m_avgPlusStdev.clear();
    m_avgMinusStdev.clear();
    
    m_data.clear();
}

bool Output::enabled() const
{
    return m_enabled;
}

void Output::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool Output::exportEnabled() const
{
    return m_exportEnabled;
}

void Output::setExportEnabled(bool exportEnabled)
{
    m_exportEnabled = exportEnabled;
}

int Output::refIndex() const
{
    return m_refIndex;
}

void Output::setRefIndex(int refIndex)
{
    m_refIndex = refIndex;
}

void Output::setPrefix( const QString & prefix)
{
    m_prefix = prefix;
}

void Output::setSuffix( const QString & suffix)
{
    m_suffix = suffix;
}

void Output::setParent( SiteResponseOutput * parent )
{
    m_parent = parent;
}

const QString Output::name() const
{
    switch (m_type)
    {
        case ModulusCurve:
            return QString(QObject::tr("Non-Linear Curve -- %1 -- Shear Modulus"))
                .arg(m_prefix);
        case DampingCurve:
            return QString(QObject::tr("Non-Linear Curve -- %1 -- Damping"))
                .arg(m_prefix);
        case AccelTimeSeries:
            return QString(QObject::tr("Response -- %1 -- Accel. Time Series -- %2"))
                .arg(m_prefix).arg(m_suffix);
        case VelTimeSeries:
            return QString(QObject::tr("Response -- %1 -- Vel. Time Series -- %2"))
                .arg(m_prefix).arg(m_suffix);
        case DispTimeSeries:
            return QString(QObject::tr("Response -- %1 -- Disp. Time Series -- %2"))
                .arg(m_prefix).arg(m_suffix);
        case StrainTimeSeries:
            return QString(QObject::tr("Response -- %1 -- Shear Strain Time Series"))
                .arg(m_prefix);
        case StressTimeSeries:
            return QString(QObject::tr("Response -- %1 -- Shear Stress Time Series"))
                .arg(m_prefix);
        case ResponseSpectrum:
            return QString(QObject::tr("Response -- %1 -- Accel. Response Spectrum"))
                .arg(m_prefix);
        case SpectralRatio:
            return QString(QObject::tr("Ratio -- %1 -- Accel. Response Spectrum Ratio"))
                .arg(m_prefix);
        case TransferFunction:
            return QString(QObject::tr("Ratio -- %1 -- Transfer Function"))
                .arg(m_prefix);
        case MaxAccelProfile:
            return QObject::tr("Profile -- Max. Acceleration");
        case MaxStrainProfile:
            return QObject::tr("Profile -- Max. Shear Strain");
        case MaxStressProfile:
            return QObject::tr("Profile -- Max. Shear Stress");
        case StressRatioProfile:
            return QObject::tr("Profile -- Stress Ratio");
        case VerticalStressProfile:
            return QObject::tr("Profile -- Vertical Stress");
        case InitialVelProfile:
            return QObject::tr("Profile -- Initial Shear Velocity");
        case FinalVelProfile:
            return QObject::tr("Profile -- Final Shear Velocity");
        case ModulusProfile:
            return QObject::tr("Profile -- Shear Modulus");
        case DampingProfile:
            return QObject::tr("Profile -- Damping");
        case MaxErrorProfile:
            return QObject::tr("Profile -- Maximum Error");
        case Undefined:
            return "";
    }
    
    return QString();
    /*
    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxAccelProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
    }
    */
}

void Output::configurePlot( QwtPlot * plot ) const
{
    // Set axis spacing
    switch (m_type)
    {
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case TransferFunction:
        case MaxErrorProfile:
            plot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLinearScaleEngine);
            plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLinearScaleEngine);
            break;
        case ResponseSpectrum:
        case SpectralRatio:
        case MaxAccelProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case ModulusCurve:
        case DampingCurve:
            plot->setAxisScaleEngine( QwtPlot::xBottom, new QwtLog10ScaleEngine);
            plot->setAxisScaleEngine( QwtPlot::yLeft, new QwtLinearScaleEngine);
            break;
        case Undefined:
            break;
    }

    // Set axis labels
    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
            plot->setAxisTitle( QwtPlot::xBottom, referenceLabel() );
            plot->setAxisTitle( QwtPlot::yLeft, dataLabel() );
            break;
        case MaxAccelProfile:
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
            plot->setAxisTitle( QwtPlot::xBottom, dataLabel() );
            plot->setAxisTitle( QwtPlot::yLeft, referenceLabel() );
            break;
        case Undefined:
            break;
    }

    // Modify the axis
    switch (m_type)
    {
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
            plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Symmetric, true);
            plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::IncludeReference, true);
            break;
        case ModulusCurve:
        case DampingCurve:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
            plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::IncludeReference, true);
            break;
        case MaxAccelProfile:
        case MaxErrorProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
            plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted, true);
            break;
        case Undefined:
            break;
    }
}

void Output::dataToCurve( const int motionIndex, const int siteIndex, QwtPlotCurve * curve) const
{
    curve->setPen(QPen(Qt::darkGray));
    
    // If strain is used skip the first point
    int incr = usesStrain() ? 1 : 0;

    curve->setData( 
            xData( motionIndex, siteIndex ).data() + incr,
            yData( motionIndex, siteIndex ).data() + incr,
            dataSize( motionIndex, siteIndex ) - incr);
        

    if ( constantWithinLayer() )
        curve->setStyle(QwtPlotCurve::Steps);
    else
        curve->setStyle(QwtPlotCurve::Lines);
}

void Output::quantilesToCurves( QList<QwtPlotCurve*> & quantiles ) const
{
    if (!hasStats()) 
        return;

    // If strain is used skip the first point
    int incr = usesStrain() ? 1 : 0;

    if (usesDepth()) {
        quantiles[0]->setData( m_average.data() + incr, m_parent->depths().data() + incr, m_average.size() - incr);
        quantiles[1]->setData( m_avgPlusStdev.data() + incr, m_parent->depths().data() + incr, m_stdev.size() - incr);
        quantiles[2]->setData( m_avgMinusStdev.data() + incr, m_parent->depths().data() + incr, m_stdev.size() - incr);

        // Use steps for the curves
        /*
        for ( int i = 0; i < quantiles.size(); ++i)
            quantiles[i]->setStyle(QwtPlotCurve::Steps);
            */

    } else {
        quantiles[0]->setData( xData().data() + incr, m_average.data() + incr, m_average.size() - incr);
        quantiles[1]->setData( xData().data() + incr, m_avgPlusStdev.data() + incr, m_stdev.size() - incr);
        quantiles[2]->setData( xData().data() + incr, m_avgMinusStdev.data() + incr, m_stdev.size() - incr);
        
        // Use lines for the curves
        /*
        for ( int i = 0; i < quantiles.size(); ++i)
            quantiles[i]->setStyle(QwtPlotCurve::Lines);
            */
    }
    
    // Set their color
    quantiles[0]->setPen(QPen(QBrush(Qt::blue), 2, Qt::SolidLine));
    quantiles[1]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
    quantiles[2]->setPen(QPen(QBrush(Qt::blue), 2, Qt::DashLine));
}    
        
void Output::clear()
{
    m_data.clear();
}

void Output::addData( const QVector<double> & data )
{
    m_data.push_back( data );
}

void Output::addInterpData( const QVector<double> & data, const QList<SubLayer> & subLayers, const QVector<double> & interpDepths)
{
    // Interpolated data
    QVector<double> interpData;
       
    // Index of the interpolated depth
    int interpIdx = 0;

    if ( constantWithinLayer() ) {
        // Initial depth corresponds to the surface
        for (int i = 0; i < subLayers.size(); ++i ) {
            bool dataAdded = false;

            // Add the data
            while ( interpIdx < interpDepths.size() && interpDepths.at(interpIdx) < subLayers.at(i).depthToBase() ) {
                // Add the data
                interpData << data.at(i);

                ++interpIdx;

                // Set that data was added
                dataAdded = true;
            }

            if (!dataAdded) {
                qDebug() << "Data point ignored!" << i;
                ++interpIdx;
            }
        }
    } else {
        // Linearly interpolate
        
        // The data needs to be modified to reflect the values at the top of
        // each of the layers
        QVector<double> modData;
        QVector<double> modDepth;
        
        if ( usesStrain() ) {
            // The values are zero at the surface and equal at layer
            // boundaries.  The value at the base of the layer is found by
            // adding twice the difference  between the top and mid to the top
            // value
            modData << 0;
            modDepth << 0;

            for ( int i = 0; i < subLayers.size(); ++i ){
                modDepth << subLayers.at(i).depthToMid();
                modData << data.at(i);
            }

            // Linearly exptrapolate to the base of the profile
            int n = subLayers.size() - 1;
            double diffDepth = subLayers.at(n).depthToMid() - subLayers.at(n-1).depthToMid();
            double slope = ( data.at(n) - data.at(n-1) ) / diffDepth;

            modDepth << subLayers.last().depthToBase();
            modData << data.last() + slope * subLayers.last().thickness() / ( 2 * diffDepth );

        } else {
            modData = data;
            
            for ( int i = 0; i < subLayers.size(); ++i )
                modDepth << subLayers.at(i).depth();
        }

        // Allocate the interpolator
        gsl_interp * interpolator = gsl_interp_alloc(gsl_interp_linear, modDepth.size());
        gsl_interp_init( interpolator, modDepth.data(), modData.data(), modDepth.size());
        gsl_interp_accel * accelerator =  gsl_interp_accel_alloc();

        for ( int i = 0; i < interpDepths.size(); ++i) {
            if ( interpDepths.at(i) < modDepth.last() )
                // Interpolate the values
                interpData <<
                    gsl_interp_eval( interpolator, modDepth.data(), modData.data(), interpDepths.at(i), accelerator);
            else
                // Stop
                break;
        }

        // Delete the interpolator and accelerator
        gsl_interp_free( interpolator );
        gsl_interp_accel_free( accelerator );

    }

    // Add bedrock stuff
    if ( subLayers.size() < data.size() )
        interpData << data.last();

    // Interpolate the data
    m_data.push_back( interpData );
}

void Output::computeStats()
{
    // Skip if the stats aren't meaningful for the output
    if (!hasStats())
        return;

    // Clear the average and standard deviations
    m_average.clear();
    m_stdev.clear();
    
    double dataPt = 0;

    // Compute the average and standard deviation value
    int count = 0;
    int index = 0;
    do {
        count = 0;
        double sum = 0;
        double sqrSum = 0;
        for (int i = 0; i < m_data.size(); ++i) {
            /* All data that is fixed in the x-direction depends on the
             * motion and site, therefore maps one-to-one with the
             * seriesEnabled.  However, this may change if input motions
             * are provided for output.
             */
            if ( !m_parent->seriesEnabledAt(i) || m_data.at(i).size() <= index )
                // Skip the data series if it is not enabled
                continue;

            dataPt = m_data.at(i).at(index);

            if ( isLogNormal() )
                dataPt = log(dataPt);

            sum += dataPt;
            sqrSum += dataPt * dataPt;

            ++count;
        }

        // If there was data, compute the average and standard deviation
        if ( count > 0 )
            m_average << sum / count;

        if ( count > 2 ) {
            // Compute the standard deviation.  The absolute value is used to
            // deal with rounding errors.  At times the difference with go
            // negative when all of the values are the same number.
            m_stdev << sqrt(fabs( sqrSum - ( sum * sum ) / count ) / (count-1));
        } else if ( count > 0 )
            m_stdev << 0;

        // Increment the index of interest
        ++index;
    } while ( count > 0 );

    // Convert the average in log space into normal space
    if ( isLogNormal() ) {
        for (int i = 0; i < m_average.size(); ++i)
            m_average[i] = exp(m_average.at(i));
    }

    m_avgPlusStdev.resize(m_average.size());
    m_avgMinusStdev.resize(m_average.size());
    // Compute the quantiles
    if ( isLogNormal() ) {
        for (int i = 0; i < m_stdev.size(); ++i) {
            m_avgPlusStdev[i] = m_average.at(i) * exp(m_stdev.at(i));
            m_avgMinusStdev[i] = m_average.at(i) * exp(-m_stdev.at(i));
        }
    } else {
        for (int i = 0; i < m_stdev.size(); ++i) {
            m_avgPlusStdev[i] = m_average.at(i) + m_stdev.at(i);
            m_avgMinusStdev[i] = m_average.at(i) - m_stdev.at(i);
        }
    }
}

bool Output::hasStats() const
{
    if ( m_data.size() < 2 )
        return false;

    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
            return true;
        case MaxErrorProfile:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        default:
            return false;
    }
}

bool Output::usesDepth() const
{
    switch (m_type)
    {
        case MaxAccelProfile:
        case MaxErrorProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
            return true;
        case ModulusCurve:
        case DampingCurve:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        default:
            return false;
    }
}

bool Output::usesStrain() const
{
    switch (m_type)
    {
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxErrorProfile:
            return true;
        case ModulusCurve:
        case DampingCurve:
        case MaxAccelProfile:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        default:
            return false;
    }
}

bool Output::constantWithinLayer() const
{
    switch (m_type)
    {
        case ModulusProfile:
        case MaxErrorProfile:
        case DampingProfile:
        case InitialVelProfile:
        case FinalVelProfile:
            return true;
        case MaxAccelProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        default:
            return false;
    }
}

bool Output::isLogNormal() const
{
    switch (m_type)
    {
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ModulusCurve:
        case DampingCurve:
        case MaxErrorProfile:
            return false;
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
        default:
            return true;
    }
}

Qt::Orientation Output::orientation() const
{
    switch (m_type)
    {
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case ModulusCurve:
        case DampingCurve:
            return Qt::Horizontal;
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
        default:
            return Qt::Vertical;
    }
}

const QVector<QVector<double> > & Output::data() const
{
    return m_data;
}

QMap<QString, QVariant> Output::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;
   
    map.insert("refIndex", m_refIndex);
    map.insert("enabled", m_enabled);
    map.insert("exportEnabled", m_exportEnabled);
    map.insert("prefix", m_prefix);
    map.insert("suffix", m_suffix);

    if (saveData) {
        QList<QVariant> list;
        // Save the data
        list.clear(); 
        for ( int i = 0; i < m_data.size(); ++i )
            list << QVariant(Serializer::toVariantList(m_data.at(i)));

        map.insert("data", list);
        map.insert("average", Serializer::toVariantList(m_average));
        map.insert("stdev", Serializer::toVariantList(m_stdev));
        map.insert("avgPlusStdev", Serializer::toVariantList(m_avgPlusStdev));
        map.insert("avgMinsStdev", Serializer::toVariantList(m_avgPlusStdev));
    }
    
    return map;
}

void Output::fromMap(const QMap<QString, QVariant> & map)
{
    m_refIndex = map.value("refIndex").toInt();
    m_enabled = map.value("enabled").toBool();
    m_exportEnabled = map.value("exportEnabled").toBool();
    m_prefix = map.value("prefix").toString();
    m_suffix = map.value("suffix").toString();

    if (map.contains("data")) {
        // Data
        m_data.clear();

        QList<QVariant> list = map.value("data").toList();
        for ( int i = 0; i < list.size(); ++i)
            m_data << Serializer::fromVariantList(list.at(i).toList()).toVector();
      
        m_average = Serializer::fromVariantList(map.value("average").toList()).toVector();
        m_stdev = Serializer::fromVariantList(map.value("stdev").toList()).toVector();
        m_avgPlusStdev = Serializer::fromVariantList(map.value("avgPlusStdev").toList()).toVector();
        m_avgMinusStdev = Serializer::fromVariantList(map.value("avgMinusStdev").toList()).toVector();
    }
}

bool Output::isTimeSeries() const
{
    switch (m_type)
    {
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
            return true;
        case InitialVelProfile:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxErrorProfile:
        case MaxStrainProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case ModulusProfile:
        case DampingProfile:
        case FinalVelProfile:
        case MaxAccelProfile:
        case ModulusCurve:
        case DampingCurve:
        default:
            return false;
    }
}

bool Output::isMotionIndependent() const
{
    switch (m_type)
    {
        case InitialVelProfile:
        case ModulusCurve:
        case DampingCurve:
            return true;
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case ModulusProfile:
        case DampingProfile:
        case FinalVelProfile:
        case MaxAccelProfile:
        default:
            return false;
    }
}

bool Output::isSiteIndependent() const
{
    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case ModulusProfile:
        case DampingProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case MaxAccelProfile:
        default:
            return false;
    }
}

void Output::toTextFile( QString path, const int motionIndex, const QString & separator, const QString & prefix) const
{
    if ( !path.endsWith( QDir::separator() ) )
        path += QDir::separator();

    QFile file( path + prefix + fileName() );

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Unable to open file:" << file.fileName();
        return;
    }

    // Create the text stream
    QTextStream out(&file);

    // Create the header
    out << "# Strata OUTPUT -- " << name();
   
    if ( !(motionIndex < 0) )
        out << " -- " << m_parent->motionNames().at(motionIndex);
   
    out << endl << "# " << referenceLabel();

    if ( !(motionIndex < 0) || isMotionIndependent() ) {
        for (int i = 0; i < m_parent->siteCount(); ++i )
            out << separator << QString("S-%1").arg(i+1);
    } else if ( isSiteIndependent() ) {
        for (int i = 0; i < m_parent->motionCount(); ++i )
            out << separator << QString("M-%1").arg(m_parent->motionNames().at(i));
    } else {
        for (int i = 0; i < m_parent->siteCount(); ++i ) {
            for (int j = 0; j < m_parent->motionCount(); ++j ) {
                out << separator << QString("S-%1-M-%2").arg(i+1).arg(m_parent->motionNames().at(j));
            }
        }
    }

    if ( hasStats() )
        out << separator << "Median" << separator << "ln Stdev.";
    // End the line
    out << endl;

    // Export the data
    const QVector<double> & ref = (usesDepth()) ? yData() : xData(motionIndex);
    int ini = (motionIndex < 0) ? 0 : motionIndex;
    int inc = (motionIndex < 0) ? 1 : m_parent->motionCount();

    for ( int i = 0; i < ref.size(); ++i ) {
        out << ref.at(i);
        for ( int j = ini; j < m_data.size(); j += inc) {
            if ( i < m_data.at(j).size() )
                out << separator << m_data.at(j).at(i);
            else
                out << separator;
        }
    
        if ( hasStats() )
            out << separator << m_average.at(i) << separator << m_stdev.at(i);
           
        // End the line
        out << endl;
    }
}

const QString Output::fileName() const
{
    QString name;

    switch (m_type)
    {
        case ModulusCurve:
            name = "nlCurve-" + m_prefix + "-shearMod.csv";
            break;
        case DampingCurve:
            name = "nlCurve-" + m_prefix + "-damping.csv";
            break;
        case AccelTimeSeries:
            name = m_prefix + "-accelTs-" + m_suffix + ".csv";
            break;
        case VelTimeSeries:
            name = m_prefix + "-velTs-" + m_suffix + ".csv";
            break;
        case DispTimeSeries:
            name = m_prefix + "-dispTs-" + m_suffix + ".csv";
            break;
        case StrainTimeSeries:
            name = m_prefix + "-strainTs.csv";
            break;
        case StressTimeSeries:
            name = m_prefix + "-stressTs.csv";
            break;
        case ResponseSpectrum:
            name = m_prefix + "-respSpec.csv";
            break;
        case SpectralRatio:
            name = m_prefix + "-specRatio.csv";
            break;
        case TransferFunction:
            name = m_prefix + "-transFunc.csv";
            break;
        case MaxStrainProfile:
            name = "profile-maxStrain.csv";
            break;
        case MaxStressProfile:
            name = "profile-maxStress.csv";
            break;
        case StressRatioProfile:
            name = "profile-stressRatio.csv";
            break;
        case VerticalStressProfile:
            name = "profile-vStress.csv";
            break;
        case InitialVelProfile:
            name = "profile-iniShearVel.csv";
            break;
        case FinalVelProfile:
            name = "profile-finalShearVel.csv";
            break;
        case ModulusProfile:
            name = "profile-modulus.csv";
            break;
        case DampingProfile:
            name = "profile-damping.csv";
            break;
        case MaxAccelProfile:
            name = "profile-maxAccel.csv";
            break;
        case MaxErrorProfile:
            name = "profile-maxError.csv";
            break;
        default:
            return "";
    }

    return name;
}

const QString Output::referenceLabel() const
{
    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
            return QObject::tr("Strain (%)");
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
            return QObject::tr("Time (s)");
        case ResponseSpectrum:
        case SpectralRatio:
            return QObject::tr("Period (s)");
        case TransferFunction:
            return QObject::tr("Frequency (Hz)");
        case MaxStrainProfile:
        case MaxStressProfile:
        case MaxErrorProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
        case InitialVelProfile:
            return QString(QObject::tr("Depth (%1)")).arg(m_parent->units()->length());
        default:
            return "";
    }
}

const QString Output::dataLabel() const
{
    switch (m_type)
    {
        case ModulusCurve:
            return QObject::tr("Normalized Shear Modulus (G/G_max)");
        case AccelTimeSeries:
            return QString(QObject::tr("Acceleration (%1)")).arg(m_parent->units()->accel());
        case VelTimeSeries:
            return QString(QObject::tr("Velocity (%1)")).arg(m_parent->units()->velTs());
        case DispTimeSeries:
            return QString(QObject::tr("Displacement (%1)")).arg(m_parent->units()->dispTs());
        case StrainTimeSeries:
            return QObject::tr("Shear strain (%)");
        case StressTimeSeries:
            return QObject::tr("Shear stress (%)");
        case ResponseSpectrum:
            return QString(QObject::tr("Spec. Acceleration (%1)")).arg(m_parent->units()->accel());
        case SpectralRatio:
            return QObject::tr("Sa_j / Sa_k");
        case TransferFunction:
            return QObject::tr("FAS_j / FAS_k");
        case MaxAccelProfile:
            return QString(QObject::tr("Maximum Acceleration (%1)")).arg(m_parent->units()->accel());
        case MaxStrainProfile:
            return QObject::tr("Maximum Shear Strain (%)");
        case MaxStressProfile:
            return QString(QObject::tr("Maximum Shear Stress (%1)")).arg(m_parent->units()->stress());
        case StressRatioProfile:
            return QObject::tr("Stress Ratio");
        case VerticalStressProfile:
            return QString(QObject::tr("Vertical Stress (%1)")).arg(m_parent->units()->stress());
        case InitialVelProfile:
        case FinalVelProfile:
            return QString(QObject::tr("Shear-wave velocity (%1)")).arg(m_parent->units()->vel());
        case ModulusProfile:
            return QString(QObject::tr("Shear modulus (%1)")).arg(m_parent->units()->stress());
        case MaxErrorProfile:
            return QObject::tr("Maximum Error in Properties (%)");
        case DampingCurve:
        case DampingProfile:
            return QObject::tr("Damping (%)");
        default:
            return "";
    }
}

const QVector<double> & Output::xData( const int motionIndex, const int siteIndex ) const
{
    switch (m_type)
    {
        case ModulusCurve:
            return m_parent->strains().at(m_refIndex);
        case DampingCurve:
            return m_parent->strains().at(m_refIndex);
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
            // Use time
            return m_parent->times().at(motionIndex);
        case ResponseSpectrum:
        case SpectralRatio:
            // Use period
            return m_parent->period().data();
        case TransferFunction:
            // Use frequency
            return m_parent->freq().data();
        case InitialVelProfile:
            return m_data.at(siteIndex);
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
        default:
            return m_data.at(m_parent->generalIndex(motionIndex,siteIndex));
    }
}

const QVector<double> & Output::yData( const int motionIndex, const int siteIndex ) const
{
    switch (m_type)
    {
        case ModulusCurve:
        case DampingCurve:
            return m_data.at(siteIndex);
        case AccelTimeSeries:
        case VelTimeSeries:
        case DispTimeSeries:
        case StrainTimeSeries:
        case StressTimeSeries:
        case ResponseSpectrum:
        case SpectralRatio:
        case TransferFunction:
            return m_data.at(m_parent->generalIndex(motionIndex,siteIndex));
        case MaxStrainProfile:
        case MaxErrorProfile:
        case MaxStressProfile:
        case StressRatioProfile:
        case VerticalStressProfile:
        case InitialVelProfile:
        case FinalVelProfile:
        case ModulusProfile:
        case DampingProfile:
        case MaxAccelProfile:
        default:
            // Use top-depth
            return m_parent->depths();
    }
}

const int Output::dataSize( const int motionIndex, const int siteIndex ) const
{
    if ( isMotionIndependent() )
        return m_data.at(siteIndex).size();
    else if ( isSiteIndependent() )
        return m_data.at(motionIndex).size();
    else
        return m_data.at( m_parent->generalIndex(motionIndex, siteIndex ) ).size();
}
