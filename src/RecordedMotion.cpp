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

#include "RecordedMotion.h"
#include "Serializer.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>

#include <fftw3.h>

#include <gsl/gsl_multifit.h>

RecordedMotion::RecordedMotion()
{ 
    // Initialize the values
    m_type = Outcrop;
    m_isEnabled = true;
    m_isLoaded = false;
    m_timeStep = 0;
    m_pointCount = 0;
    m_scale = 1;
    m_format = Rows;
    m_dataColumn = 2;
    m_startLine = 4;
    m_stopLine = 0;
}

QStringList RecordedMotion::formatList()
{
    QStringList list;

    list << "Rows" << "Columns";

    return list;
}

void RecordedMotion::clear()
{
    m_accel.clear();
}

QString RecordedMotion::fileName() const
{
    return m_fileName;
}

void RecordedMotion::setFileName(const QString & fileName)
{
    // Save the relative path
    QFileInfo fileInfo(fileName);

    m_fileName = fileInfo.absoluteFilePath();
    // Reset loaded flag 
    m_isLoaded = false;
}

QString RecordedMotion::description() const
{
    return m_description;
}

void RecordedMotion::setDescription(const QString & description)
{
    m_description = description;
    // Reset loaded flag 
    m_isLoaded = false;
}

double RecordedMotion::timeStep() const
{
    return m_timeStep;
}

void RecordedMotion::setTimeStep(double timeStep)
{
    m_timeStep = timeStep;
    // Reset loaded flag 
    m_isLoaded = false;
}

int RecordedMotion::pointCount() const
{
    return m_pointCount;
}

void RecordedMotion::setPointCount(int count)
{
    m_pointCount = count;
    // Reset loaded flag 
    m_isLoaded = false;
}

double RecordedMotion::scale() const
{
    return m_scale;
}

void RecordedMotion::setScale(double scale) 
{
    if (m_isLoaded) {
        // Compute the ratio and apply this to the acceleration
        double ratio = scale / m_scale;
        // Compute the scaled acceleration, fas, and PGA
        for (int i=0; i < m_accel.size(); ++i)
            m_accel[i] = ratio * m_accel.at(i);

        for (int i=0; i < m_fas.size(); ++i)
            m_fas[i] = ratio * m_fas.at(i);
        
        m_pga = ratio * m_pga;
    } 
    // Save the new scale factor
    m_scale = scale;
}

bool RecordedMotion::isEnabled() const
{
    return m_isEnabled;
}

void RecordedMotion::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

RecordedMotion::Format RecordedMotion::format() const
{
    return m_format;
}

void RecordedMotion::setFormat(Format format)
{
    m_format = format;
    // Reset loaded flag 
    m_isLoaded = false;
}

int RecordedMotion::dataColumn() const
{
    return m_dataColumn;
}

void RecordedMotion::setDataColumn(int column)
{
    m_dataColumn = column;
    // Reset loaded flag 
    m_isLoaded = false;
}

int RecordedMotion::startLine() const
{
    return m_startLine;
}

void RecordedMotion::setStartLine(int line)
{
    m_startLine = line;
    // Reset loaded flag 
    m_isLoaded = false;
}

int RecordedMotion::stopLine() const
{
    return m_stopLine;
}

void RecordedMotion::setStopLine(int line)
{
    m_stopLine = line;
    // Reset loaded flag 
    m_isLoaded = false;
}

double RecordedMotion::pga() const
{
    return m_pga;
}

void RecordedMotion::setPga(double pga)
{
    // Set the new scale factor
    setScale( pga / m_pga * m_scale ); 
    // The new PGA value is computed in setScale
}

const QVector<double> & RecordedMotion::time() const
{
    return m_time;
}

const QVector<double> & RecordedMotion::accel() const
{
    return m_accel;
}

void RecordedMotion::timeSeries( const QVector<std::complex<double> > & tf,
                QVector<double> & accel,  QVector<double> & vel,
                QVector<double> & disp, const bool baselineCorrect )
{
    accel = timeSeries(tf);
    // Compute the velocity and displacement from the original acceleration
    integrate(accel, vel);
    integrate(vel, disp);

    if ( baselineCorrect ) {
        int term = 7;
        // Fix a polynominal to the data
        QVector<double> dispCoeffs = baselineFit( term, disp );

        // Compute the coeffs to be applied to the acceleration time series
        QVector<double> accelCoeffs;

        for ( int i = 2; i < dispCoeffs.size(); ++i )
            accelCoeffs << i * (i-1) * dispCoeffs.at(i);

        for ( int i = 0; i < accel.size(); ++i ) {
            double value = 0;

            for ( int j = 0; j < accelCoeffs.size(); ++j)
                value += accelCoeffs.at(j) * pow(m_time.at(i),j);
            // correct the accel
            accel[i] -= value;
        }
        // Correct the acceleration time-series
        integrate(accel, vel);
        integrate(vel, disp);
    }

    return;
}
    
const QVector<double> RecordedMotion::timeSeries(const QVector<std::complex<double> > & tf) const
{
    // Resize the response vector
    QVector<std::complex<double> > Y(m_fas.size());
    // Apply the transfer function to the motion
    for ( int i = 0; i < m_fas.size(); i++ )
        Y[i] = tf.at(i) * m_fas.at(i);
    // Compute the time series
    QVector<double> ts;
    ifft( Y, ts );

    // Resize the time series to the length of the original record
    ts.resize( m_pointCount );


    /*
    QFile data("output.csv");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);

        for (int i = 0; i < m_freq.size(); ++i )
            out << m_freq.at(i) << ","
                << tf.at(i).real() << ","
                << tf.at(i).imag() << ","
                << m_fas.at(i).real() << ","
                << m_fas.at(i).imag() << ","
                << Y.at(i).real() << ","
                << Y.at(i).imag() << ","
                << endl;
    } 

    exit(2);
    */

    return ts;
}

bool RecordedMotion::hasTime() const
{
    return true;
}

QString RecordedMotion::toString() const
{
    QFileInfo fileInfo(m_fileName);
    // Return the folder and file name
    return fileInfo.dir().dirName() + QString(QDir::separator()) + fileInfo.fileName();
}

QMap<QString, QVariant> RecordedMotion::toMap(bool saveData) const
{
    QMap<QString, QVariant> map;
	
    map.insert("enabled", m_isEnabled);
    map.insert("fileName", m_fileName);
    map.insert("description", m_description);

    map.insert("timeStep", m_timeStep);
    map.insert("pointCount", m_pointCount);
    map.insert("scale", m_scale);
    map.insert("type", m_type);

    map.insert("format", m_format);
    map.insert("dataColumn", m_dataColumn);

    map.insert("startLine", m_startLine);
    map.insert("stopLine", m_stopLine);

    map.insert("respSpec", m_respSpec->toMap());

    if (saveData)
        map.insert("accel", Serializer::toVariantList(m_accel));

    return map;
}

void RecordedMotion::fromMap( const QMap<QString, QVariant> & map)
{
    m_isEnabled = map.value("enabled").toBool();
    m_fileName = map.value("fileName").toString();
    m_description = map.value("description").toString();

    m_timeStep = map.value("timeStep").toDouble();
    m_pointCount = map.value("pointCount").toInt();
    m_scale = map.value("scale").toDouble();
    m_type = (Motion::Type)map.value("type").toInt();

    m_format = (Format)map.value("format").toInt();
    m_dataColumn = map.value("dataColumn").toInt();

    m_startLine = map.value("startLine").toInt();
    m_stopLine = map.value("stopLine").toInt();
    
    m_respSpec->fromMap(map.value("respSpec").toMap());

    if (map.contains("accel")) {
        m_accel = Serializer::fromVariantList(map.value("accel").toList()).toVector();
    }

    // Load the motion
    load();
}

QString RecordedMotion::toHtml() const
{
    QString html;

    html += QString(
                "<tr>"
                "<td>%1</td>"
                "<td>%2</td>"
                "<td>%3</td>"
                "<td>%4</td>"
                "<td>%5</td>"
                "</tr>"
            )
        .arg(toString())
        .arg(m_description)
        .arg(typeList().at(m_type))
        .arg(m_scale)
        .arg(m_pga);

    return html;
}

bool RecordedMotion::load()
{
    // Don't load the file if it has already been loaded
    if (m_isLoaded)
        return true;

    if ( m_accel.isEmpty() ) {
            // Load the file
    
            // Check the values
            if (m_timeStep <= 0) {
            qCritical("Time step must be greater than one");
            return false;
            }
            if (m_startLine < 0) {
            qCritical("Number of header lines must be positive");
            return false;
            }

            // Load the file
            QFile file(m_fileName);
            if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) {
            qCritical() << "Unable to open the time series file:" << qPrintable(m_fileName);
            return false;
            }

            // Create the textstream
            QTextStream stream( &file );

            int lineNum = 0;
            // Skip the header lines
            while ( lineNum < m_startLine ) {
                stream.readLine();
                ++lineNum;
            }

            // Initialize the length of m_accel
            m_accel.resize(m_pointCount);
            m_time.resize(m_pointCount);

            // Process each of the lines
            int index = 0;
            // Read the first line
            QString line =stream.readLine();
            // Using the first line, the delimeter can be found.  Right now we just
            // assume that the delimeter is whitespace, comma, or semi-colon
            QRegExp delimeter("[ ,;]");

            while(!line.isNull()) 
            {
                // Stop if line exceeds number of lines;
                if ( m_stopLine > 0 &&  lineNum >= m_stopLine )
                    break;

                // Read the line and split the line based on the deliminator        
                QStringList row = line.split(delimeter, QString::SkipEmptyParts);

                // Process the row based on the format
                bool ok;
                switch (m_format)
                {
                    case Rows:
                        // Use all parts of the data 
                        for(int i = 0; i < row.size(); ++i) {
                            if ( index == m_pointCount ) {
                                qWarning("Point count reached before end of data!");
                                break;
                            }
                            // Apply the scale factor and read the acceleration
                            m_accel[index] = m_scale * row.at(i).trimmed().toDouble(&ok);
                            // Increment the index
                            ++index;
                            // Stop if there was an error in the conversion
                            if (!ok)
                                continue;
                        }
                        break;
                    case Columns:
                        // Use only the important column, however at the end of the
                        // file that column may not exist -- this only happens when the
                        // row format is applied, but it still causes the program to
                        // crash.
                        if ( m_dataColumn - 1 < row.size() )
                            m_accel[index] = row.at(m_dataColumn-1).trimmed().toDouble(&ok);
                            
                        // Increment the index
                        ++index;
                        break;
                }

                // Throw an error if there was a problem
                if (!ok) {
                    qCritical("Error converting string to double. Check starting line.");
                    return false;
                }

                // Read the next line
                ++lineNum;
                line = stream.readLine();
            } 

            // Check that the number of points matches the number specified
            if ( index != m_pointCount ) {
                qCritical("Number of points read does not match specified number of points");
                return false;
            }
    } else {
        // Use stored acceleration values
        m_time.resize(m_accel.size());
    }

    // Compute the PGA
    m_pga = findMaxAbs(m_accel);

    // Create the time vector
    for (int i=0; i < m_time.size(); ++i)
        m_time[i] = i * m_timeStep;

    // Compute the next largest power of two
    int n = 1;
    while ( n < m_accel.size() ) n <<= 1;

    // Pad the fft data with zeroes
    QVector<double> data(n);
   
    for ( int i = 0; i < data.size(); ++i )
        data[i] = (i < m_accel.size()) ? m_accel.at(i) : 0.0;

    // Compute the Fourier amplitude spectrum.  The FAS computed through this
    // method is only the postive frequencies and is of length n/2+1 where n is
    // the lenght of the acceleration time history.
	fft( data, m_fas );
    // The frequency step is based on the sampling rate and the number of data
    // points. 
    m_freq.resize(m_fas.size());
   
	double delta = 1 / ( 2 * m_timeStep * (m_freq.size()-1) );

    for (int i=0; i < m_freq.size(); ++i)
            m_freq[i] = i * delta;

    m_isLoaded = true;
    return true;
}

double RecordedMotion::findMaxAbs( const QVector<double> & vector ) const
{
    //  Assume the first value is the largest
    double max = fabs( vector.at(0) );
    // Check the remaining values
    for ( int i = 1; i < vector.size(); i++ )
        if ( fabs( vector.at(i) ) > max )
            max = fabs( vector.at(i) );
    // Return the maximum
    return max;
}

double RecordedMotion::max( Motion::DurationType /*durationType*/, const QVector<std::complex<double> > & tf ) const
{
	// Return the maximum value in the time history
	return findMaxAbs(timeSeries(tf));
} 	

QVector<double> RecordedMotion::computeSa( Motion::DurationType durationType, const QVector<double> & period, double damping, const QVector<std::complex<double> > & accelTf )
{
    // Compute the SDOF
    computeSdofTf( period, damping );

    QVector<double> sa(period.size());
    QVector<std::complex<double> > tf(m_freq.size());
    // Compute the response at each period
    for ( int i = 0; i < sa.size(); ++i ) {
        // If there is an acceleration transfer function combine the SDOF and
        // acceleration transfer functions
        if ( !accelTf.isEmpty() ) {
            for (int j = 0; j < tf.size(); ++j)
                tf[j] = accelTf.at(j) * m_sdofTf.at(i).at(j);
        } else
            tf = m_sdofTf.at(i);

        // Compute the maximum response
        sa[i] = max( durationType, tf);
    }

    return sa;
}

void RecordedMotion::integrate( const QVector<double> & in, QVector<double> & out ) const
{
    out.resize(in.size());

    out[0] = 0.0;
    
    for ( int i = 1; i < in.size(); ++i )
        out[i] = out.at(i-1) + m_timeStep * ( in.at(i) + in.at(i-1) ) / 2;

}

const QVector<double> RecordedMotion::baselineFit( const int term, const QVector<double> & series ) const
{
    Q_ASSERT( term > 3);
    // Create the matrix of terms.  The first column is x_i^0 (1), second
    // column is x_i^1 (x), third is x_i^2, etc.
    gsl_matrix * X = gsl_matrix_alloc( series.size(), term );
    gsl_vector * y = gsl_vector_alloc( series.size() );

    for ( int i = 0; i < series.size(); ++i ) {
        gsl_vector_set( y, i, series.at(i) );

        for ( int j = 0; j < term; ++j ) {
            if ( j < 2 ) 
                // Don't use the first two terms in the fitting
                gsl_matrix_set( X, i, j, 0 );
            else
                gsl_matrix_set( X, i, j, pow( m_time.at(i), j) );
        }
    }

    // Co-variance matrix
    gsl_matrix * cov = gsl_matrix_alloc( term, term );

    // Coefficients
    gsl_vector * c = gsl_vector_alloc( term );

    // Fit the data series
    gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc( series.size(), term);

    double chisq = 0;
    gsl_multifit_linear( X, y, c, cov, &chisq, work);

    // Copy coefficients over to m_coeffs
    QVector<double> coeffs(term);

    for ( int i = 0; i < term; ++i )
        coeffs[i] = gsl_vector_get( c, i );

    // Clear the variables
    gsl_matrix_free( X );
    gsl_vector_free( y );
    gsl_vector_free( c );
    gsl_matrix_free( cov );
    
    gsl_multifit_linear_free (work);

    return coeffs;
}

void RecordedMotion::fft( const QVector<double>& in, QVector<std::complex<double> >& out ) const
{
	// The number of elements in the double array is 2 * n, but only the first
	// n are filled with data.  For the complex array, n/2 + 1 elements are
	// required.

	// Copy the input QVector into a double array
	double* inArray = (double*) fftw_malloc( sizeof(double) * 2 * in.size() );

	for( int i = 0; i < in.size(); i++ )
		inArray[i] = in.at(i);

	// Allocate the space for the output
	int n = in.size()/2 + 1;
	fftw_complex* outArray = (fftw_complex*)fftw_malloc( sizeof(fftw_complex) * n );

	// Create the plan and execute the FFT
	fftw_plan p = fftw_plan_dft_r2c_1d( in.size(), inArray, outArray, FFTW_ESTIMATE );
	fftw_execute(p);

	// Copy the data to the output QVector
	out.resize( n );
	for( int i = 0; i < n; i++ )
		out[i] = std::complex<double>( outArray[i][0], outArray[i][1] );

	// Free the memory
	fftw_destroy_plan(p);
	fftw_free(inArray);
	fftw_free(outArray);
}
	
void RecordedMotion::ifft( const QVector<std::complex<double> >& in, QVector<double>& out ) const
{
	// Copy the input QVector into a double array
	fftw_complex* inArray = (fftw_complex*) fftw_malloc( sizeof(fftw_complex) * in.size() );

	for( int i = 0; i < in.size(); i++ ) {
		inArray[i][0] = in.at(i).real();
		inArray[i][1] = in.at(i).imag();
	}

	// Allocate the space for the output
	int n = 2 * ( in.size() - 1 );
	double* outArray = (double*)fftw_malloc( sizeof(double) * n );

	// Create the plan and execute the FFT
	fftw_plan p = fftw_plan_dft_c2r_1d( n, inArray, outArray, FFTW_ESTIMATE );
	fftw_execute(p);

	// Copy the data to the output QVector and normalize by QVector length
	out.resize( 2 * ( in.size() - 1 ) );
	for( int i = 0; i < out.size(); i++ ) 
		out[i] = outArray[i] / out.size();

	// Free the memory
	fftw_destroy_plan(p);
	fftw_free(inArray);
	fftw_free(outArray);
}
