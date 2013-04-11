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

#include "TimeSeriesMotion.h"

#include "Serializer.h"
#include "ResponseSpectrum.h"
#include "Units.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <QDebug>

#include <fftw3.h>

#include <gsl/gsl_multifit.h>

#include <cmath>

TimeSeriesMotion::TimeSeriesMotion(QObject * parent)
        : AbstractMotion(parent)
{
    m_isLoaded = false;
    m_saveData = true;
    // Initialize the values -- appropriate values for an AT2 file
    m_inputUnits = Gravity;
    m_timeStep = 0;
    m_pointCount = 0;
    m_format = Rows;
    m_dataColumn = 2;
    m_startLine = 5;
    m_stopLine = 0;
    m_scale = 1.0;
}

TimeSeriesMotion::TimeSeriesMotion(const QString & fileName, double scale,  AbstractMotion::Type type, bool * successful, QObject * parent)
    : AbstractMotion(parent), m_fileName(QDir::cleanPath(fileName))
{ 
    m_isLoaded = false;
    m_saveData = true;
    m_type = type;
    // Initialize the values -- appropriate values for an AT2 file
    m_inputUnits = Gravity;
    m_timeStep = 0;
    m_pointCount = 0;
    m_format = Rows;
    m_dataColumn = 2;
    m_startLine = 5;
    m_stopLine = 0;
    m_scale = 1.0;

    if (!m_fileName.isEmpty() && load(m_fileName, true, scale))
        *successful = true;
    else
        *successful = false;
}

TimeSeriesMotion::~TimeSeriesMotion()
{
}

QStringList TimeSeriesMotion::formatList()
{
    return QStringList() << tr("Rows") << tr("Columns");
}


QString TimeSeriesMotion::fileName() const
{
    return m_fileName;
}

void TimeSeriesMotion::setFileName(const QString & fileName)
{    
    QFileInfo fileInfo(fileName);
    const QString _fileName = QDir::cleanPath(fileInfo.absoluteFilePath());

    if (m_fileName != _fileName) {
        emit fileNameChanged(_fileName);
    }

    m_fileName = _fileName;   
    setIsLoaded(false);
}

double TimeSeriesMotion::freqMax() const
{
    return freqNyquist();
}

double TimeSeriesMotion::freqNyquist() const
{
    return 1. / (2. * m_timeStep);
}

QStringList TimeSeriesMotion::inputUnitsList()
{
    return QStringList() << tr("Gravity") << tr("cm/sec^2") << tr("in/sec^2");
}

TimeSeriesMotion::InputUnits TimeSeriesMotion::inputUnits() const
{
    return m_inputUnits;
}

void TimeSeriesMotion::setInputUnits(int inputUnits)
{
    setInputUnits((InputUnits)inputUnits);
}

void TimeSeriesMotion::setInputUnits(TimeSeriesMotion::InputUnits inputUnits)
{
    if (m_inputUnits != inputUnits) {
        // Save the previously used conversation and scale factors
        const double prevFactor = unitConversionFactor();
        const double prevScale = m_scale;

        m_inputUnits = inputUnits;
        setModified(true);

        // Need to modify the scale to correct for the change in units.
        // This is done by modifying the m_scale parameter which is then
        // used in rescale the appropriate values.
        m_scale *= prevFactor / unitConversionFactor();
        setScale(prevScale);

        emit inputUnitsChanged(inputUnits);
    }

    setIsLoaded(false);
}

double TimeSeriesMotion::unitConversionFactor() const
{
    switch (m_inputUnits) {
    case CentimetersPerSecondSquared:
        return 1. / (100. * 9.80665);
    case InchesPerSecondSquared:
        return 1. / (12. * 32.174);
    default:
        return 1.;
    }
}

int TimeSeriesMotion::pointCount() const
{
    return m_pointCount;
}

void TimeSeriesMotion::setPointCount(int count)
{
    if (m_pointCount != count) {
        m_pointCount = count;
        emit pointCountChanged(count);

        setIsLoaded(false);
    }
}

double TimeSeriesMotion::timeStep() const
{
    return m_timeStep;
}

void TimeSeriesMotion::setTimeStep(double timeStep)
{
    if (fabs(m_timeStep - timeStep) > DBL_EPSILON) {
        m_timeStep = timeStep;

        emit timeStepChanged(m_timeStep);

        setIsLoaded(false);
    }
}

double TimeSeriesMotion::scale() const
{
    return m_scale;
}

void TimeSeriesMotion::setScale(double scale)
{
    if (fabs(m_scale - scale) > DBL_EPSILON) {
        // Compute the ratio and apply this to the acceleration
        const double ratio = scale / m_scale;

        m_scale = scale;
        emit scaleChanged(scale);

        if (m_isLoaded) {
            // Scale the various spectra
            for (int i = 0; i < m_accel.size(); ++i)
                m_accel[i] = ratio * m_accel.at(i);

            for (int i = 0; i < m_fourierAcc.size(); ++i) {
                m_fourierAcc[i] *= ratio;
                m_fourierVel[i] *= ratio;
            }

            m_respSpec->scaleBy(ratio);

            setPga(ratio * m_pga);
            setPgv(ratio * m_pgv);
        }
    }
}

TimeSeriesMotion::Format TimeSeriesMotion::format() const
{
    return m_format;
}

void TimeSeriesMotion::setFormat(Format format)
{
    if (m_format != format) {
        m_format = format;
        emit formatChanged(format);

        setIsLoaded(false);
    }
}

void TimeSeriesMotion::setFormat(int format)
{
    setFormat((Format)format);
}

int TimeSeriesMotion::dataColumn() const
{
    return m_dataColumn;
}

void TimeSeriesMotion::setDataColumn(int column)
{
    if (m_dataColumn != column) {
        m_dataColumn = column;
        emit dataColumnChanged(column);

        setIsLoaded(false);
    }
}

int TimeSeriesMotion::startLine() const
{
    return m_startLine;
}

void TimeSeriesMotion::setStartLine(int line)
{
    if (m_startLine != line) {
        m_startLine = line;
        emit startLineChanged(line);

        setIsLoaded(false);
    }
}

int TimeSeriesMotion::stopLine() const
{
    return m_stopLine;
}

void TimeSeriesMotion::setStopLine(int line)
{
    if (m_stopLine != line) {
        m_stopLine = line;
        emit stopLineChanged(line);

        setIsLoaded(false);
    }
}

const QVector<double> & TimeSeriesMotion::freq() const
{
    return m_freq;
}

QVector<double> TimeSeriesMotion::time() const
{    
    // Time step based on the max frequency
    const double dt = timeStep();

    QVector<double> v(m_pointCount);

    for (int i = 0; i < m_pointCount; ++i)
        v[i] = i * dt;

    return v;
}

const QVector<double> & TimeSeriesMotion::accel() const
{
    return m_accel;
}

QVector<double> TimeSeriesMotion::timeSeries(
        MotionType type, const QVector<std::complex<double> > & tf, const bool baselineCorrect) const
{
    // Compute the time series
    QVector<double> ts = calcTimeSeries(m_fourierAcc, tf);

    // Remove the zero padded values from the time series
    ts.resize(m_pointCount);

    if (baselineCorrect) {
        const double dt = timeStep();

        // Compute the displacement time series
        QVector<double> disp = ts;
        for (int i = 0; i < 2; ++i)
            disp = integrate(disp);

        // Use a fourth order polynomial
        int term = 4;
        // Fix a polynominal to the data
        QVector<double> dispCoeffs = baselineFit(term, disp);

        // Compute the coeffs to be applied to the acceleration time series
        QVector<double> accelCoeffs;

        for ( int i = 2; i < dispCoeffs.size(); ++i ) {
            accelCoeffs << i * (i-1) * dispCoeffs.at(i);
        }

        for ( int i = 0; i < ts.size(); ++i ) {
            double value = 0;

            for ( int j = 0; j < accelCoeffs.size(); ++j) {
                value += accelCoeffs.at(j) * pow(i * dt,j);
            }

            // correct the accel
            ts[i] -= value;
        }
    }

    // Integrate to the appropriate time series
    for (int i = 0; i < (int)type; ++i)
        ts = integrate(ts);

    // Scale to the appropriate units
    if (type == Velocity || type == Displacement) {
        for ( int i = 0; i < ts.size(); ++i )
            ts[i] *= Units::instance()->tsConv();
    }

    return ts;
}

QString TimeSeriesMotion::name() const
{
    QFileInfo fileInfo(m_fileName);
    // Return the folder and file name
    return fileInfo.dir().dirName() + QString(QDir::separator()) + fileInfo.fileName();
}

QString TimeSeriesMotion::toHtml() const
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
            .arg(name())
            .arg(m_description)
            .arg(typeList().at(m_type))
            .arg(m_scale)
            .arg(m_pga);

    return html;
}

bool TimeSeriesMotion::load(const QString &fileName, bool defaults, double scale)
{
    m_accel.clear();

    setFileName(fileName);
    const QString ext = m_fileName.right(3).toUpper();

    // Load the file
    QFile file(m_fileName);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qWarning() << "Unable to open the time series file:" << qPrintable(m_fileName);
        return false;
    }
    QTextStream stream(&file);

    if (defaults) {
        if (ext == "AT2") {
            // Set format
            setFormat(Rows);
            setStartLine(5);
            setStopLine(0);
            setScale(scale);

            // Read in the header information
            Q_UNUSED(stream.readLine());

            setDescription(stream.readLine());

            Q_UNUSED(stream.readLine());

            QStringList parts = stream.readLine().split(QRegExp("\\s+"));

            bool b;
            const int count = parts.at(0).toInt(&b);
            if (b && count > 1) {
                // Greater than one condition to catch if an acceleration value is read
                setPointCount(count);
            } else {
                qCritical() << "Unable to parse the point count in AT2 file!";
                return false;
            }

            const double timeStep = parts.at(1).toDouble(&b);

            if (b) {
                setTimeStep(timeStep);
            } else {
                qCritical() << "Unable to parse the time step in AT2 file!";
                return false;
            }
        } else {
            // Unknown file format can't process with default settings
            return false;
        }
    } else {
        // Check the input
        if (m_timeStep <= 0) {
            qCritical("Time step must be greater than one");
            return false;
        }
        if (m_startLine < 0) {
            qCritical("Number of header lines must be positive");
            return false;
        }
    }

    // Move back to the start of the stream
    stream.seek(0);

    int lineNum = 1;
    // Skip the header lines
    while (lineNum < m_startLine) {
        Q_UNUSED(stream.readLine());
        ++lineNum;
    }

    // Initialize the length of m_accel
    m_accel.resize(m_pointCount);

    // Process each of the lines
    int index = 0;
    // Read the first line
    QString line =stream.readLine();

    bool finished = false;
    bool stopLineReached = false;

    // Modify the scale for unit conversion
    scale = unitConversionFactor() * m_scale;

    while (!finished && !line.isNull()) {
        // Stop if line exceeds number of lines.  The line number has
        // to be increased by one because the user display starts at
        // line number 1 instead of 0
        if (m_stopLine > 0 &&  m_stopLine <= lineNum+1) {
            stopLineReached = true;
            break;
        }

        // Read the line and split the line
        QRegExp rx("(-?\\d*\\.\\d+(?:[eE][+-]?\\d+)?)");
        int pos = 0;
        QStringList row;

        while ((pos = rx.indexIn(line, pos)) != -1) {
            row << rx.cap(1);
            pos += rx.matchedLength();
        }

        // Process the row based on the format
        bool ok;
        switch (m_format) {
        case Rows:
            // Use all parts of the data
            for(int i = 0; i < row.size(); ++i) {
                if ( index == m_pointCount ) {
                    qWarning("Point count reached before end of data!");
                    finished = true;
                    break;
                }
                // Apply the scale factor and read the acceleration
                m_accel[index] = scale * row.at(i).trimmed().toDouble(&ok);
                // Increment the index
                ++index;
                // Stop if there was an error in the conversion
                if (!ok) {
                    continue;
                }
            }
            break;
                case Columns:
            // Use only the important column, however at the end of the
            // file that column may not exist -- this only happens when the
            // row format is applied, but it still causes the program to
            // crash.
            if ( m_dataColumn - 1 < row.size() ) {
                m_accel[index] = scale * row.at(m_dataColumn-1).trimmed().toDouble(&ok);
            }

            // Increment the index
            ++index;
            break;
        }

        // Throw an error if there was a problem
        if (!ok) {
            qCritical() << "Error converting string to double in line: \n\""
                    << qPrintable(line) << "\"\nCheck starting line.";
            return false;
        }

        // Read the next line
        ++lineNum;
        line = stream.readLine();
    }

    if (m_pointCount != index) {
        if (stopLineReached) {
            qWarning() << "Number of points limited by stop line.";
        } else {
            qCritical() << "Number of points read does not equal specified point count!";
            return false;
        }
    }

    // Compute motion properties
    calculate();

    setIsLoaded(true);
    return true;
}

void TimeSeriesMotion::calculate()
{      
    // Compute the next largest power of two
    int n = 1;
    while (n <= m_accel.size())
        n <<= 1;

    // Pad the acceleration data with zeroes
    QVector<double> accel(n, 0);

    for (int i = 0; i < m_accel.size(); ++i)
        accel[i] = m_accel.at(i);

    // Compute the Fourier amplitude spectrum.  The FAS computed through this
    // method is only the postive frequencies and is of length n/2+1 where n is
    // the lenght of the acceleration time history.
    fft(accel, m_fourierAcc);

    // Compute FAS of the velocity time series
    fft(integrate(accel), m_fourierVel);

    // Create the frequency array truncated at the maximum frequency
    const double delta = 1 / (2. * m_timeStep * (m_fourierAcc.size() - 1));
    m_freq.resize(m_fourierAcc.size());
    for (int i = 0; i < m_freq.size(); ++i)
        m_freq[i] = i * delta;

    // Compute PGA and PGV
    setPga(findMaxAbs(accel));
    setPgv(findMaxAbs(integrate(accel)) * Units::instance()->tsConv());

    // Compute the response spectrum
    m_respSpec->setSa(computeSa(m_respSpec->period(), m_respSpec->damping()));
}

int TimeSeriesMotion::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return m_accel.size();
}

int TimeSeriesMotion::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant TimeSeriesMotion::data(const QModelIndex & index, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
    case TimeColumn:
        return QString::number(m_timeStep * index.row(), 'f', 3);
    case AccelColumn:
        return QString::number(m_accel.at(index.row()), 'e', 2);
    }

    return QVariant();
}

QVariant TimeSeriesMotion::headerData( int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case TimeColumn:
            return tr("Time (s)");
        case AccelColumn:
            return tr("Accel. (g)");
        }
    case Qt::Vertical:
        return section+1;
    }

    return QVariant();
}

void TimeSeriesMotion::setSaveData(bool b)
{
    m_saveData = b;
}

bool TimeSeriesMotion::saveData() const
{
    return m_saveData;
}

bool TimeSeriesMotion::isLoaded() const
{
    return m_isLoaded;
}

void TimeSeriesMotion::setIsLoaded(bool isLoaded)
{
    m_isLoaded = isLoaded;
}

double TimeSeriesMotion::findMaxAbs(const QVector<double> & v) const
{
    //  Assume the first value is the largest
    double max = fabs(v.at(0));
    // Check the remaining values
    foreach (double d, v)
        if (fabs(d) > max)
            max = fabs(d);

    // Return the maximum
    return max;
}

double TimeSeriesMotion::max(const QVector<std::complex<double> > & tf) const
{
    // Return the maximum value in the time history
    return findMaxAbs(calcTimeSeries(m_fourierAcc, tf));
} 	

double TimeSeriesMotion::maxVel(const QVector<std::complex<double> > &tf, bool applyScale) const
{
    const double scale = applyScale ? Units::instance()->tsConv() : 1.;

    return scale * findMaxAbs(timeSeries(Velocity, tf, false));
}

double TimeSeriesMotion::maxDisp(const QVector<std::complex<double> > &tf, bool applyScale) const
{
    const double scale = applyScale ? Units::instance()->tsConv() : 1.;

    return scale * findMaxAbs(timeSeries(Displacement, tf, false));
}


QVector<double> TimeSeriesMotion::computeSa(const QVector<double> & period, double damping, const QVector<std::complex<double> > & accelTf )
{
    if (!accelTf.isEmpty())
        Q_ASSERT(accelTf.size() == m_freq.size());

    QVector<double> sa(period.size());

    const double deltaFreq = 1 / (m_timeStep * m_accel.size());

    // Compute the response at each period
    for ( int i = 0; i < sa.size(); ++i ) {
        /*
        The FAS needs to extend to have a sampling rate that is 10 times larger
        than the maximum frequency which corresponds to 5 times larger Nyquist
        frequency. This is required for sufficient resolution of the peak value
        in the time domain. For example, at a frequency 100 Hz (period of 0.01
        sec) the FAS has to extend to 500 Hz. The FAS is initialized to be zero
        which allows for resolution of the time series.
        */
        const double f = 1 / period.at(i);       
        const int minSize = qMax(m_freq.size(), int((f * 5.0) / deltaFreq));

        int size = 1;
        while (size < minSize)
            size <<= 1;

        // Only apply the SDOF transfer function over frequencies defined by the original motion
        QVector<std::complex<double> > tf = calcSdofTf(period.at(i), damping);

        // The amplitude of the FAS needs to be scaled to reflect the increased
        // number of points.
        const double scale = double(size) / double(m_freq.size());

        for (int j = 0; j < tf.size(); ++j )
            tf[j] *= scale;

        // If there is an acceleration transfer function combine the SDOF and
        // acceleration transfer functions
        if (!accelTf.isEmpty()) {
            for (int j = 0; j < tf.size(); ++j)
                tf[j] *= accelTf.at(j);
        }

        // Pad with zeros to achieve the required point count
        if (tf.size() < size)
            tf.resize(size);

        // Compute the maximum response
        sa[i] = max(tf);
    }

    return sa;
}

const QVector<double> TimeSeriesMotion::absFourierAcc(const QVector<std::complex<double> >& tf) const
{
    return absFourier(m_fourierAcc, tf);
}

const QVector<double> TimeSeriesMotion::absFourierVel(const QVector<std::complex<double> >& tf) const
{
    return absFourier(m_fourierVel, tf);
}

double TimeSeriesMotion::calcMaxStrain(const QVector<std::complex<double> >& tf) const
{
    return findMaxAbs(strainTimeSeries(tf));
}

QVector<double> TimeSeriesMotion::strainTimeSeries(const QVector<std::complex<double> >& tf, const bool baseLineCorrect) const
{
    QVector<double> strain = calcTimeSeries(m_fourierVel, tf);

    // Use a simple subtraction of the average to correct the record.
    if (baseLineCorrect) {
        // Compute the average
        double sum = 0;

        for (int i = 0; i < strain.size(); ++i)
            sum += strain.at(i);

        double avg = sum / strain.size();

        for (int i = 0; i < strain.size(); ++i)
            strain[i] -= avg;
    }

    return strain;
}

QVector<double> TimeSeriesMotion::ariasIntensity(const QVector<std::complex<double> > & tf) const
{
    QVector<double> accelTs = calcTimeSeries(m_fourierAcc, tf);

    QVector<double> accelTsSqr = QVector<double>(accelTs.size());
    for (int i = 0; i < accelTs.size(); ++i)
        accelTsSqr[i] = pow(accelTs.at(i), 2);

    QVector<double> ai = QVector<double>(accelTs.size(), 0.0);

    /* The Arias Intensity is defined by:
     * IA = PI / (2 * g) \int_0^Td a(t) ^ dt
     * In this calculation, the integration is peformed using the trapezoid rule. The time step and 1/2 factor are pulled out and combined with the constant factor.
     */
    const double foo = m_timeStep * M_PI / (4 * Units::instance()->gravity());

    for (int i = 1; i < ai.size(); ++i)
        ai[i] = foo * (accelTsSqr.at(i - 1) + accelTsSqr.at(i)) + ai[i - 1];

    return ai;
}

QVector<double> TimeSeriesMotion::integrate(const QVector<double> & in) const
{
    QVector<double> out(in.size());
    const double dt = timeStep();

    out[0] = 0.0;
    
    for (int i = 1; i < in.size(); ++i)
        out[i] = out.at(i-1) + dt * (in.at(i) + in.at(i-1)) / 2;

    return out;
}

const QVector<double> TimeSeriesMotion::baselineFit( const int term, const QVector<double> & series ) const
{
    Q_ASSERT(term >= 3);
    // Create the matrix of terms.  The first column is x_i^0 (1), second
    // column is x_i^1 (x), third is x_i^2, etc.
    gsl_matrix* X = gsl_matrix_alloc(series.size(), term);
    gsl_vector* y = gsl_vector_alloc(series.size());

    for (int i = 0; i < series.size(); ++i) {
        gsl_vector_set( y, i, series.at(i));

        for (int j = 0; j < term; ++j) {
            if ( j < 2 ) {
                // Don't use the first two terms in the fitting
                gsl_matrix_set(X, i, j, 0);
            } else {
                gsl_matrix_set(X, i, j, pow(m_timeStep * i, j));
            }
        }
    }

    // Co-variance matrix
    gsl_matrix * cov = gsl_matrix_alloc(term, term);

    // Coefficients
    gsl_vector * c = gsl_vector_alloc(term);

    // Fit the data series
    gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc(series.size(), term);

    double chisq = 0;
    gsl_multifit_linear(X, y, c, cov, &chisq, work);

    // Copy coefficients over to m_coeffs
    QVector<double> coeffs(term);

    for ( int i = 0; i < term; ++i )
        coeffs[i] = gsl_vector_get(c, i);

    // Clear the variables
    gsl_matrix_free(X);
    gsl_vector_free(y);
    gsl_vector_free(c);
    gsl_matrix_free(cov);
    
    gsl_multifit_linear_free (work);

    return coeffs;
}

QVector<double> TimeSeriesMotion::absFourier(const QVector< std::complex<double> >& fa, const QVector<std::complex<double> >& tf) const
{
    QVector<double> absFa(fa.size());
    const double dt = timeStep();

    if (!tf.isEmpty()) {
        Q_ASSERT(fa.size() == tf.size());
        // Apply the transfer function to the fas
        for (int i = 0; i < fa.size(); ++i)
            absFa[i] = abs(tf.at(i) * fa.at(i)) * dt;
    } else {
        // The FAS is scaled by the time-step when it is compared with another motion
        for (int i = 0; i < fa.size(); ++i)
            absFa[i] = abs(fa.at(i)) * dt;
    }

    return absFa;
}

void TimeSeriesMotion::fft( const QVector<double>& in, QVector<std::complex<double> >& out ) const
{
    // The number of elements in the double array is 2 * n, but only the first
    // n are filled with data.  For the complex array, n/2 + 1 elements are
    // required.

    // Copy the input QVector into a double array
    double* inArray = (double*) fftw_malloc( sizeof(double) * 2 * in.size() );

    for (int i = 0; i < in.size(); i++) {
        inArray[i] = in.at(i);
    }

    // Allocate the space for the output
    int n = in.size() / 2 + 1;
    fftw_complex* outArray = (fftw_complex*)fftw_malloc( sizeof(fftw_complex) * n);

    // Create the plan and execute the FFT
    fftw_plan p = fftw_plan_dft_r2c_1d( in.size(), inArray, outArray, FFTW_ESTIMATE);
    fftw_execute(p);

    // Copy the data to the output QVector
    out.resize(n);
    for (int i = 0; i < n; i++)
        out[i] = std::complex<double>(outArray[i][0], outArray[i][1]);

    // Free the memory
    fftw_destroy_plan(p);
    fftw_free(inArray);
    fftw_free(outArray);
}

void TimeSeriesMotion::ifft(const QVector<std::complex<double> >& in, QVector<double>& out ) const
{
    // Copy the input QVector into a double array
    fftw_complex* inArray = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * in.size());

    for( int i = 0; i < in.size(); i++ ) {
        inArray[i][0] = in.at(i).real();
        inArray[i][1] = in.at(i).imag();
    }    

    // Allocate the space for the output
    int n = 2 * (in.size() - 1);
    double* outArray = (double*)fftw_malloc(sizeof(double) * n);

    // Create the plan and execute the FFT
    fftw_plan p = fftw_plan_dft_c2r_1d(n, inArray, outArray, FFTW_ESTIMATE);
    fftw_execute(p);

    // Copy the data to the output QVector and normalize by QVector length
    out.resize(n);
    for (int i = 0; i < out.size(); i++)
        out[i] = outArray[i] / out.size();

    // Free the memory
    fftw_destroy_plan(p);
    fftw_free(inArray);
    fftw_free(outArray);
}

QVector<double> TimeSeriesMotion::calcTimeSeries(QVector<std::complex<double> > fa, const QVector<std::complex<double> > & tf) const
{
    // Apply the transfer fucntion
    if (!tf.isEmpty()) {

        // If needed, zero pad the Fourier amplitudes such that is has the same length as the incoming transfer function.
        if (fa.size() << tf.size())
            fa.resize(tf.size());

        for (int i = 0; i < fa.size(); ++i)
            fa[i] *= tf.at(i);
    }

    // Compute the time series
    QVector<double> ts;
    ifft(fa, ts);

    return ts;
}

QDataStream & operator<< (QDataStream & out, const TimeSeriesMotion* tsm)
{
    out << (quint8)3;

    out << qobject_cast<const AbstractMotion*>(tsm);

    // Properties for TimeSeriesMotion
    out
            << tsm->m_saveData
            << tsm->m_fileName
            << tsm->m_timeStep
            << tsm->m_pointCount
            << tsm->m_scale
            << (int)tsm->m_format
            << tsm->m_dataColumn
            << tsm->m_startLine
            << tsm->m_stopLine;

    // Added in version 2
    out << (int)tsm->m_inputUnits;

    // Save the data internally if requested
    if (tsm->m_saveData) {
        out << tsm->m_accel;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, TimeSeriesMotion* tsm)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractMotion*>(tsm);

    // Propertise for TimeSeriesMotion
    int format;

    in >> tsm->m_saveData;

    if (ver < 3) {
        // Removed in version 3
        double freqMax;
        in >> freqMax;
    }

    in >> tsm->m_fileName
            >> tsm->m_timeStep
            >> tsm->m_pointCount
            >> tsm->m_scale
            >> format
            >> tsm->m_dataColumn
            >> tsm->m_startLine
            >> tsm->m_stopLine;

    tsm->setFormat(format);

    if (ver > 1) {
        // Units added in version 2
        int units;
        in >> units;
        tsm->setInputUnits(units);
    }

    // Save the data internally if requested
    if (tsm->m_saveData) {
        in >> tsm->m_accel;
    } else {
        tsm->load(tsm->m_fileName, false, tsm->m_scale);
    }

    if (tsm->m_accel.size()) {
        tsm->calculate();
        tsm->m_isLoaded = true;
    }

    return in;
}
