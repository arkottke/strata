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

#ifndef RECORDED_MOTION_H_
#define RECORDED_MOTION_H_

#include "Motion.h"
#include <QStringList>

#include <complex>

class RecordedMotion : public Motion
{
    public:
        RecordedMotion();
        
        //! Format of the text file
        enum Format{ 
            Rows,   /*!< Data is in rows */
            Columns /*!< Data is in columns */ 
        };

        static QStringList formatList();
        
		double max( DurationType duration, const QVector<std::complex<double> > & tf = QVector<std::complex<double> >() ) const;

        QVector<double> computeSa( DurationType durationType, const QVector<double> & period, double damping,
                const QVector<std::complex<double> > & accelTf = QVector<std::complex<double> >() );

        //! Clear the acceleration data
        void clear();

        QString fileName() const;
        void setFileName(const QString & fileName);

        QString description() const;
        void setDescription(const QString & description);

        double timeStep() const;
        void setTimeStep(double timeStep);

        int pointCount() const;
        void setPointCount(int count); 

        double scale() const;
        void setScale(double scale);

        Format format() const;
        void setFormat(Format format);

        int dataColumn() const;
        void setDataColumn(int column);        

        int startLine() const;
        void setStartLine(int lines);
        
        int stopLine() const;
        void setStopLine(int lines);

        double pga() const;
        void setPga(double pga);        // Allow users to define a PGA and scale the motion appropriately

        bool isEnabled() const;
        void setIsEnabled(bool isEnabled);

        const QVector<double> & time() const;
        const QVector<double> & accel() const;

        /*! Compute the acceleration, velocity, and displacement time series.
         *
         * \param tf transfer function to be applied to the Fourier amplitude spectrum
         * \param accel reference to the acceleration time series
         * \param vel reference to the velocity time series
         * \param disp reference to the displacement time series
         * \param baselineCorrect if the time series should be baseline corrected
         */
        void timeSeries( const QVector<std::complex<double> > & tf,
                QVector<double> & accel,  QVector<double> & vel,
                QVector<double> & disp, const bool baselineCorrect );

        /*! Compute the acceleration time series for a given transfer function.
         * \param tf transfer function to be applied to the Fourier amplitude spectrum
         */
        const QVector<double> timeSeries( const QVector<std::complex<double> > & tf) const;
        
        //! If the motion can provide a time series
        bool hasTime() const;

        QString toString() const; 

        QMap<QString, QVariant> toMap(bool saveData = false) const;
        void fromMap( const QMap<QString, QVariant> & map);
        
        //! Create a html document containing the information of the model
        QString toHtml() const;
        
        //! Load the file
        bool load();

    private:
        //! States if the file has been loaded or not 
        bool m_isLoaded;

        //! Controls if the RecordedMotion is to be used
        bool m_isEnabled;

        //! The filename of the time series
        QString m_fileName;

        //! The description of the time series
        QString m_description;

        //! The time step of the time series
        double m_timeStep;

        //! The number of points in the time series
        int m_pointCount;

        //! The scale factor that is applied to the motion 
        double m_scale;

        //! The format of the text file
        Format m_format;

        //! The column of the acceleration data
        int m_dataColumn;

        //! The line the data starts on
        int m_startLine;            

        //! The line the data stops on, 0 for complete file.
        int m_stopLine;             

        //! The time values associated with the acceleration
        QVector<double> m_time;	    

        //! The acceleration data points in g
        QVector<double> m_accel;	

		//! The complex Fourier Amplitude Spectrum
        QVector<std::complex<double> > m_fas;  

        //! The maximum acceleration of the record
        double m_pga;

        //! Call the readFile and computeSpecAccel functions
        void processFile( std::ifstream* );

        //! Find the maximum absolute value of a vector
        double findMaxAbs( const QVector<double> & vector ) const;

        /*! Compute the integral of the time series using the trapezoid rule.
         * \param in time series to be integrated
         * \param out integrated time series
         */
        void integrate( const QVector<double> & in, QVector<double> & out ) const;

        /*! Fit a polynomial to the time series using least squares regression.
         * \param term number of terms in the polynomial (order + 1)
         * \param series the time series
         */
        const QVector<double> baselineFit( const int term, const QVector<double> & series ) const;
        
        //! Compute the Fast Fourier Transform from real to complex using the FFTW
        void fft( const QVector<double>& in, QVector< std::complex<double> >& out) const;

        //! Compute the Inverse Fast Fourier Transformation from complex to real using
        void ifft( const QVector< std::complex<double> >& in, QVector<double>& out ) const;
};
#endif
