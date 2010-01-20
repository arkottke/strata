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

#ifndef OUTPUT_H_
#define OUTPUT_H_

#include "SubLayer.h"

#include <QVector>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class SiteResponseOutput; // Forward declaration of SiteResponseOutput

//! A class to contain an switch and data for output
class Output : public QObject
{
    Q_OBJECT

    public:
        //! Types of output
        enum Type {
            ModulusCurve, //!< Normalized Shear modulus reduction curve
            DampingCurve, //!< Damping ratio curve
            AccelTimeSeries, //!< Acceleration time series
            VelTimeSeries, //!< Velocity time series
            DispTimeSeries, //!< Displacement time series
            StrainTimeSeries, //!< Shear-strain time series
            StressTimeSeries, //!< Shear-stress time series
            FourierSpectrum, //!< Absolute value of the Fourier Spectrum
            ResponseSpectrum, //!< Acceleration response spectrum
            SpectralRatio, //!< Acceleration response spectrum ratio
            TransferFunction, //!< Acceleration transfer function
            StrainTransferFunction, //!< Strain transfer function
            MaxAccelProfile, //!< Maximum acceleration profile
            MaxVelProfile, //!< Maximum velocity profile
            MaxStrainProfile, //!< Maximum shear-strain profile
            MaxStressProfile, //!< Maximum shear-stress profile
            StressReducCoeffProfile, //!< Stress reduction coefficient profile
            MaxErrorProfile, //!< Maximum error profile
            StressRatioProfile, //!< Shear-stress to vertical stress ratio profile
            VerticalStressProfile, //!< Vertical total stress
            InitialVelProfile, //!< Initial shear-wave velocity profile
            FinalVelProfile, //!< Final shear-wave velocity profile
            ModulusProfile, //!< Final shear-modulus profile
            DampingProfile, //!< Final damping profile
            Undefined   //!< Not defined
        };

        Output( Type type = Undefined, int refIndex = -1, QObject * parent = 0 );

        //! Reset the object to the default values
        void reset();
        
        Type type() const;

        bool enabled() const;

        bool exportEnabled() const;
        void setExportEnabled(bool b);

        int refIndex() const;
        void setRefIndex(int refIndex);

        void setPrefix( const QString & prefix);
        void setSuffix( const QString & suffix);

        void setParent( SiteResponseOutput * parent = 0 );

        const QString name() const;
        
        //! Access the data
        const QVector<QVector<double> > & data() const;

        //! Configure the plot 
        void configurePlot( QwtPlot * plot ) const;

        //! Create the curve of plot curve Qt::Orientationfrom the data
        void dataToCurve( const int motionIndex, const int siteIndex, QwtPlotCurve * curve ) const;
        
        //! Set the quantiles to the curves
        void quantilesToCurves( QList<QwtPlotCurve*> & quantiles ) const;

        //! Clear the data
        void clear();
        
        //! Add data containing just magnitude
        void addData( const QVector<double> & magnitude );
        
        /*! Add data and interpolate between depths
         * \param magnitude magnitude values of the data
         * \param subLayers subLayers of the current site profile
         * \param interpDepths interpolate data to these depths
         */
        void addInterpData( const QVector<double> & magnitude, const QList<SubLayer> & subLayers, const QVector<double> & interpDepths);

        //! Remove the last result
        void removeLast();

        //! Compute statistics
        void computeStats();

        //! If the statistics of the data are meaningful
        bool hasStats() const;

        //! Return if the curve is referenced to depth
        bool usesDepth() const;

        //! Return if the curve is referenced to the mid layer depth
        bool usesStrain() const;
        
        //! Constant value throughout layer
        bool constantWithinLayer() const;

        //! If the output is a time series
        bool hasMotionSpecificReference() const;

        //! Independent of motion
        bool isMotionIndependent() const;

        //! Independent of the site
        bool isSiteIndependent() const;

        /*! Create a text file from the data
         *
         * \param path location to save the files
         * \param motionIndex index of the motion file for the time series
         * \param separator character used to separate the columns of data
         * \param prefix prefix to append to the start of filename
         */
        void toTextFile( QString path, const int motionIndex = -1 , const QString & separator = ",",  const QString & prefix = "") const;
        
        /*! If data is log-normally distributed.
         *
         * Data is assumed to be either normally or log-normally distributed.
         * If the data is not log normally distributed then it is normally
         * distributed.
         */
        bool isLogNormal() const;

        //! Orientation of the data
        Qt::Orientation orientation() const;

        // //! Determine the index of the closest reference point
        // const int indexOfClosestRef(const double value) const;

        QMap<QString, QVariant> toMap() const;
		void fromMap(const QMap<QString, QVariant> & map);

    public slots:
        void setEnabled(bool b);

    signals:
        void wasModified();

    protected:
        //! Type of data series
        Type m_type;

        //! Parent contains information shared acrossed outputs
        SiteResponseOutput * m_parent;

        //! Index of reference data
        int m_refIndex;

        //! Boolean for if the output is to be computed
        bool m_enabled;

        //! If the output should be exported
        bool m_exportEnabled;

        //! Prefix of the data series
        QString m_prefix;

        //! Suffix of the data series
        QString m_suffix;

        //! Average value
        QVector<double> m_average;

        //! Standard deviation
        QVector<double> m_stdev;

        //! Plus and Minus one standard deviation curves for plots
        QVector<double> m_avgPlusStdev;
        QVector<double> m_avgMinusStdev;

        //! Holds information about the data
        QVector<QVector<double> > m_data;

        //! File name
        const QString fileName(const int motionIndex = -1) const;

        //! Name of the reference label
        const QString referenceLabel() const;
        
        //! Name of the data label
        const QString dataLabel() const;

        //! Reference to the horizontal data
        const QVector<double> & xData( const int motionIndex = -1, const int siteIndex = -1) const;
        
        //! Reference to the vertical data
        const QVector<double> & yData( const int motionIndex = -1, const int siteIndex = -1) const;

        //! Size of the data
        int dataSize( const int motionIndex = -1, const int siteIndex = -1 ) const;
};
#endif
