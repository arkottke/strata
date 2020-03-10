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

#include "TimeSeriesMotion.h"

#include "ResponseSpectrum.h"
#include "Serialize.h"
#include "Units.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonValue>
#include <QRegExp>
#include <QTextStream>


#ifdef USE_FFTW
#include <fftw3.h>
#else
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_errno.h>
#endif

#include <gsl/gsl_multifit.h>

#include <cmath>

TimeSeriesMotion::TimeSeriesMotion(QObject * parent)
        : AbstractMotion(parent)
{
    _isLoaded = false;
    _saveData = true;
    // Initialize the values -- appropriate values for an AT2 file
    _inputUnits = Gravity;
    _timeStep = 0;
    _pointCount = 0;
    _format = Rows;
    _dataColumn = 2;
    _startLine = 5;
    _stopLine = 0;
    _scale = 1.0;
}

TimeSeriesMotion::TimeSeriesMotion(const QString & fileName, double scale,  AbstractMotion::Type type, bool * successful, QObject * parent)
    : AbstractMotion(parent), _fileName(QDir::cleanPath(fileName))
{ 
    _isLoaded = false;
    _saveData = true;
    _type = type;
    // Initialize the values -- appropriate values for an AT2 file
    _inputUnits = Gravity;
    _timeStep = 0;
    _pointCount = 0;
    _format = Rows;
    _dataColumn = 2;
    _startLine = 5;
    _stopLine = 0;
    _scale = 1.0;

    if (!_fileName.isEmpty() && load(_fileName, true, scale)) {
        *successful = true;
    } else {
        *successful = false;
    }
}

TimeSeriesMotion::~TimeSeriesMotion()
{
}

auto TimeSeriesMotion::formatList() -> QStringList
{
    return QStringList() << tr("Rows") << tr("Columns");
}


auto TimeSeriesMotion::fileName() const -> QString
{
    return _fileName;
}

void TimeSeriesMotion::setFileName(QString fileName)
{    
    // Convert to an absolute path.
    QFileInfo fileInfo(fileName);
    fileName = QDir::cleanPath(fileInfo.absoluteFilePath());

    if (_fileName != fileName) {
        emit fileNameChanged(fileName);
    }

    _fileName = fileName;   
    setIsLoaded(false);
}

auto TimeSeriesMotion::freqMax() const -> double
{
    return freqNyquist();
}

auto TimeSeriesMotion::freqNyquist() const -> double
{
    return 1. / (2. * _timeStep);
}

auto TimeSeriesMotion::inputUnitsList() -> QStringList
{
    return QStringList() << tr("Gravity") << tr("cm/sec^2") << tr("in/sec^2");
}

auto TimeSeriesMotion::inputUnits() const -> TimeSeriesMotion::InputUnits
{
    return _inputUnits;
}

void TimeSeriesMotion::setInputUnits(int inputUnits)
{
    setInputUnits((InputUnits)inputUnits);
}

void TimeSeriesMotion::setInputUnits(TimeSeriesMotion::InputUnits inputUnits)
{
    if (_inputUnits != inputUnits) {
        // Save the previously used conversation and scale factors
        const double prevFactor = unitConversionFactor();
        const double prevScale = _scale;

        _inputUnits = inputUnits;
        setModified(true);

        // Need to modify the scale to correct for the change in units.
        // This is done by modifying the _scale parameter which is then
        // used in rescale the appropriate values.
        _scale *= prevFactor / unitConversionFactor();
        setScale(prevScale);

        emit inputUnitsChanged(inputUnits);
    }

    setIsLoaded(false);
}

auto TimeSeriesMotion::unitConversionFactor() const -> double
{
    switch (_inputUnits) {
    case CentimetersPerSecondSquared:
        return 1. / (100. * 9.80665);
    case InchesPerSecondSquared:
        return 1. / (12. * 32.174);
    default:
        return 1.;
    }
}

auto TimeSeriesMotion::pointCount() const -> int
{
    return _pointCount;
}

void TimeSeriesMotion::setPointCount(int count)
{
    if (_pointCount != count) {
        _pointCount = count;
        emit pointCountChanged(count);

        setIsLoaded(false);
    }
}

auto TimeSeriesMotion::timeStep() const -> double
{
    return _timeStep;
}

void TimeSeriesMotion::setTimeStep(double timeStep)
{
    if (abs(_timeStep - timeStep) > DBL_EPSILON) {
        _timeStep = timeStep;

        emit timeStepChanged(_timeStep);

        setIsLoaded(false);
    }
}

auto TimeSeriesMotion::scale() const -> double
{
    return _scale;
}

void TimeSeriesMotion::setScale(double scale)
{
    if (abs(_scale - scale) > DBL_EPSILON) {
        // Compute the ratio and apply this to the acceleration
        const double ratio = scale / _scale;

        _scale = scale;
        emit scaleChanged(scale);

        if (_isLoaded) {
            // Scale the various spectra
            for (int i = 0; i < _accel.size(); ++i) {
                _accel[i] = ratio * _accel.at(i);
            }

            for (int i = 0; i < _fourierAcc.size(); ++i) {
                _fourierAcc[i] *= ratio;
                _fourierVel[i] *= ratio;
            }

            _respSpec->scaleBy(ratio);

            setPga(ratio * _pga);
            setPgv(ratio * _pgv);
        }
    }
}

auto TimeSeriesMotion::format() const -> TimeSeriesMotion::Format
{
    return _format;
}

void TimeSeriesMotion::setFormat(Format format)
{
    if (_format != format) {
        _format = format;
        emit formatChanged(format);

        setIsLoaded(false);
    }
}

void TimeSeriesMotion::setFormat(int format)
{
    setFormat((Format)format);
}

auto TimeSeriesMotion::dataColumn() const -> int
{
    return _dataColumn;
}

void TimeSeriesMotion::setDataColumn(int column)
{
    if (_dataColumn != column) {
        _dataColumn = column;
        emit dataColumnChanged(column);

        setIsLoaded(false);
    }
}

auto TimeSeriesMotion::startLine() const -> int
{
    return _startLine;
}

void TimeSeriesMotion::setStartLine(int line)
{
    if (_startLine != line) {
        _startLine = line;
        emit startLineChanged(line);

        setIsLoaded(false);
    }
}

auto TimeSeriesMotion::stopLine() const -> int
{
    return _stopLine;
}

void TimeSeriesMotion::setStopLine(int line)
{
    if (_stopLine != line) {
        _stopLine = line;
        emit stopLineChanged(line);

        setIsLoaded(false);
    }
}

auto TimeSeriesMotion::freq() const -> const QVector<double> &
{
    return _freq;
}

auto TimeSeriesMotion::time() const -> QVector<double>
{    
    // Time step based on the max frequency
    const double dt = timeStep();

    QVector<double> v(_pointCount);

    for (int i = 0; i < _pointCount; ++i) {
        v[i] = i * dt;
    }

    return v;
}

auto TimeSeriesMotion::accel() const -> const QVector<double> &
{
    return _accel;
}

auto TimeSeriesMotion::timeSeries(
        MotionType type, const QVector<std::complex<double> > & tf, const bool baselineCorrect) const -> QVector<double>
{
    // Compute the time series
    QVector<double> ts = calcTimeSeries(_fourierAcc, tf);
    // Remove the zero padded values from the time series
    ts.resize(_pointCount);

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
    for (int i = 0; i < (int)type; ++i) {
        ts = integrate(ts);
    }

    // Scale to the appropriate units
    if (type == Velocity || type == Displacement) {
        for ( int i = 0; i < ts.size(); ++i ) {
            ts[i] *= Units::instance()->tsConv();
        }
    }

    return ts;
}

auto TimeSeriesMotion::name() const -> QString
{
    QFileInfo fileInfo(_fileName);
    // Return the folder and file name
    return fileInfo.dir().dirName() + QString(QDir::separator()) + fileInfo.fileName();
}

auto TimeSeriesMotion::toHtml() const -> QString
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
            .arg(_description)
            .arg(typeList().at(_type))
            .arg(_scale)
            .arg(_pga);

    return html;
}

auto TimeSeriesMotion::load(const QString &fileName, bool defaults, double scale) -> bool
{
    _accel.clear();

    setFileName(fileName);
    const QString ext = _fileName.right(3).toUpper();

    // Load the file
    QFile file(_fileName);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        qWarning() << "Unable to open the time series file:" << qPrintable(_fileName);
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

            QList<QRegExp> patterns = {
                // Example: 8751    0.0040    NPTS, DT
                QRegExp("(\\d+)\\s+([0-9.]+)\\s+NPTS, DT"),
                // Example: NPTS=   7998, DT=   .0050 SEC,                                             
                // Example: NPTS=   3666, DT=   0.025 SEC
                QRegExp("NPTS=\\s+(\\d+), DT=\\s+([0-9.]+) SEC,?"),
            };

            QString line = stream.readLine();
            bool success = false;
            for (QRegExp &pattern: patterns) {
                int pos = pattern.indexIn(line);
                if (pos < 0) {
                    continue;
                }

                bool b;
                const int count = pattern.cap(1).toInt(&b);
                if (b && count > 1) {
                    // Greater than one condition to catch if an acceleration value is read
                    setPointCount(count);
                } else {
                    qCritical() << "Unable to parse the point count in AT2 file!";
                    return false;
                }

                const double timeStep = pattern.cap(2).toDouble(&b);
                if (b) {
                    setTimeStep(timeStep);
                } else {
                    qCritical() << "Unable to parse the time step in AT2 file!";
                    return false;
                }
                success = true;
            }
            if (!success) {
                qCritical() << "Unrecognized header format!" << line;
                return false;
            }
        } else {
            // Unknown file format can't process with default settings
            return false;
        }
    } else {
        // Check the input
        if (_timeStep <= 0) {
            qCritical("Time step must be greater than one");
            return false;
        }
        if (_startLine < 0) {
            qCritical("Number of header lines must be positive");
            return false;
        }
    }

    // Move back to the start of the stream
    stream.seek(0);

    int lineNum = 1;
    // Skip the header lines
    while (lineNum < _startLine) {
        Q_UNUSED(stream.readLine());
        ++lineNum;
    }

    // Read the first line
    QString line = stream.readLine();

    bool finished = false;
    bool stopLineReached = false;

    // Modify the scale for unit conversion
    scale = unitConversionFactor() * _scale;

    while (!finished && !line.isNull()) {
        // Stop if line exceeds number of lines.  The line number has
        // to be increased by one because the user display starts at
        // line number 1 instead of 0
        if (_stopLine > 0 &&  _stopLine <= lineNum+1) {
            stopLineReached = true;
            break;
        }

        // Read the line and split the line
        QRegExp rx("(-?[0-9.]+(?:[eE][+-]?\\d+)?)");
        int pos = 0;
        QStringList row;

        while ((pos = rx.indexIn(line, pos)) != -1) {
            row << rx.cap(1);
            pos += rx.matchedLength();
        }

        // Process the row based on the format
        bool ok;
        switch (_format) {
        case Rows:
            // Use all parts of the data
            for(int i = 0; i < row.size(); ++i) {
                if ( _pointCount && _accel.size() >= _pointCount ) {
                    qWarning("Point count reached before end of data!");
                    finished = true;
                    break;
                }
                // Apply the scale factor and read the acceleration
                _accel << scale * row.at(i).trimmed().toDouble(&ok);
                // Stop if there was an error in the conversion
                if (!ok) {
                    continue;
                }
            }
            break;
        case Columns:
            if ( _pointCount && _accel.size() >= _pointCount ) {
                qWarning("Point count reached before end of data!");
                finished = true;
                break;
            }
            // Use only the important column, however at the end of the
            // file that column may not exist -- this only happens when the
            // row format is applied, but it still causes the program to
            // crash.
            if ( _dataColumn - 1 < row.size() ) {
                _accel << scale * row.at(_dataColumn-1).trimmed().toDouble(&ok);
            }
            break;
        }
        // Throw an error if there was a problem
        if (!ok) {
            qCritical() << "Error converting string to double in line: \n\""
                        << qPrintable(line) << "\"\nCheck starting line.";
            return false;
        }

        if (stream.atEnd()) {
            break;
        } else {
            // Read the next line
            line = stream.readLine();
        }
    }

    if (_pointCount && _pointCount != _accel.size()) {
        if (stopLineReached) {
            qWarning() << "Number of points limited by stop line.";
        } else {
            qCritical() << "Number of points read does not equal specified point count!";
            return false;
        }
    }

    // Update counts that might not have been set
    if (_pointCount == 0)
        setPointCount(_accel.size());

    if (_stopLine == 0)
        setStopLine(_pointCount + _startLine - 1);

    // Compute motion properties
    calculate();

    setIsLoaded(true);
    return true;
}

void TimeSeriesMotion::calculate()
{      
    // Compute the next largest power of two
    int n = 1;
    while (n <= _accel.size())
        n <<= 1;

    // Pad the acceleration data with zeroes
    QVector<double> accel(n, 0);

    for (int i = 0; i < _accel.size(); ++i)
        accel[i] = _accel.at(i);

    // Compute the Fourier amplitude spectrum.  The FAS computed through this
    // method is only the postive frequencies and is of length n/2+1 where n is
    // the lenght of the acceleration time history.
    fft(accel, _fourierAcc);

    // Test the FFT/IFFT methods
    // QVector<double> test;
    // ifft(_fourierAcc, test);
    // for (int i = 0; i < test.size(); ++i)
    //     Q_ASSERT(abs(test.at(i) - accel.at(i)) < 1E-5);

    // Compute FAS of the velocity time series
    fft(integrate(accel), _fourierVel);

    // Create the frequency array truncated at the maximum frequency
    const double delta = 1 / (2. * _timeStep * (_fourierAcc.size() - 1));
    _freq.resize(_fourierAcc.size());
    for (int i = 0; i < _freq.size(); ++i)
        _freq[i] = i * delta;

    // Compute PGA and PGV
    setPga(findMaxAbs(accel));
    setPgv(findMaxAbs(integrate(accel)) * Units::instance()->tsConv());

    // Compute the response spectrum
    _respSpec->setSa(computeSa(_respSpec->period(), _respSpec->damping()));
}

auto TimeSeriesMotion::rowCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);
    return _accel.size();
}

auto TimeSeriesMotion::columnCount(const QModelIndex & parent) const -> int
{
    Q_UNUSED(parent);
    return 2;
}

auto TimeSeriesMotion::data(const QModelIndex & index, int role) const -> QVariant
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (index.column()) {
    case TimeColumn:
        return QString::number(_timeStep * index.row(), 'f', 3);
    case AccelColumn:
        return QString::number(_accel.at(index.row()), 'e', 2);
    }

    return QVariant();
}

auto TimeSeriesMotion::headerData( int section, Qt::Orientation orientation, int role) const -> QVariant
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
    _saveData = b;
}

auto TimeSeriesMotion::saveData() const -> bool
{
    return _saveData;
}

auto TimeSeriesMotion::isLoaded() const -> bool
{
    return _isLoaded;
}

void TimeSeriesMotion::setIsLoaded(bool isLoaded)
{
    _isLoaded = isLoaded;
}

auto TimeSeriesMotion::findMaxAbs(const QVector<double> & v) const -> double
{
    //  Assume the first value is the largest
    double max = abs(v.at(0));
    // Check the remaining values
    for (const double &d : v)
        if (abs(d) > max)
            max = abs(d);

    // Return the maximum
    return max;
}

auto TimeSeriesMotion::max(const QVector<std::complex<double> > & tf) const -> double
{
    // Return the maximum value in the time history
    return findMaxAbs(calcTimeSeries(_fourierAcc, tf));
} 	

auto TimeSeriesMotion::maxVel(const QVector<std::complex<double> > &tf) const -> double
{
    return findMaxAbs(timeSeries(Velocity, tf, false));
}

auto TimeSeriesMotion::maxDisp(const QVector<std::complex<double> > &tf) const -> double
{
    return findMaxAbs(timeSeries(Displacement, tf, false));
}

auto TimeSeriesMotion::computeSa(const QVector<double> & period, double damping, const QVector<std::complex<double> > & accelTf ) -> QVector<double>
{
    if (!accelTf.isEmpty())
        Q_ASSERT(accelTf.size() == _freq.size());

    QVector<double> sa(period.size());

    const double deltaFreq = 1 / (_timeStep * _accel.size());

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
        const int minSize = qMax(_freq.size(), int((f * 5.0) / deltaFreq));

        int size = 1;
        while (size < minSize)
            size <<= 1;

        // Need to have an odd number of points in the frequency domain for a
        // equal number in the time domain.
        size += 1;

        // Only apply the SDOF transfer function over frequencies defined by the original motion
        QVector<std::complex<double> > tf = calcSdofTf(period.at(i), damping);

        // The amplitude of the FAS needs to be scaled to reflect the increased
        // number of points.
        const double scale = double(size) / double(_freq.size());

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

auto TimeSeriesMotion::absFourierAcc(const QVector<std::complex<double> >& tf) const -> const QVector<double>
{
    return absFourier(_fourierAcc, tf);
}

auto TimeSeriesMotion::absFourierVel(const QVector<std::complex<double> >& tf) const -> const QVector<double>
{
    return absFourier(_fourierVel, tf);
}

auto TimeSeriesMotion::calcMaxStrain(const QVector<std::complex<double> >& tf) const -> double
{
    return findMaxAbs(strainTimeSeries(tf));
}

auto TimeSeriesMotion::strainTimeSeries(const QVector<std::complex<double> >& tf, const bool baseLineCorrect) const -> QVector<double>
{
    QVector<double> strain = calcTimeSeries(_fourierVel, tf);
    // Remove the zero padded values from the time series
    strain.resize(_pointCount);

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

auto TimeSeriesMotion::ariasIntensity(const QVector<std::complex<double> > & tf) const -> QVector<double>
{
    QVector<double> accelTs = calcTimeSeries(_fourierAcc, tf);

    QVector<double> accelTsSqr = QVector<double>(accelTs.size());
    for (int i = 0; i < accelTs.size(); ++i)
        accelTsSqr[i] = pow(accelTs.at(i), 2);

    QVector<double> ai = QVector<double>(accelTs.size(), 0.0);

    /* The Arias Intensity is defined by:
     * IA = PI / (2 * g) \int_0^Td a(t) ^ dt
     * In this calculation, the integration is peformed using the trapezoid
     * rule. The time step and 1/2 factor are pulled out and combined with the
     * constant factor.
     */
    const double foo = _timeStep * M_PI / (4 * Units::instance()->gravity());

    for (int i = 1; i < ai.size(); ++i)
        ai[i] = foo * (accelTsSqr.at(i - 1) + accelTsSqr.at(i)) + ai[i - 1];

    return ai;
}

auto TimeSeriesMotion::integrate(const QVector<double> & in) const -> QVector<double>
{
    QVector<double> out(in.size());
    const double dt = timeStep();

    out[0] = 0.0;
    
    for (int i = 1; i < in.size(); ++i)
        out[i] = out.at(i-1) + dt * (in.at(i) + in.at(i-1)) / 2;

    return out;
}

auto TimeSeriesMotion::baselineFit( const int term, const QVector<double> & series ) const -> const QVector<double>
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
                gsl_matrix_set(X, i, j, pow(_timeStep * i, j));
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

    // Copy coefficients over to _coeffs
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

auto TimeSeriesMotion::absFourier(const QVector< std::complex<double> >& fa, const QVector<std::complex<double> >& tf) const -> QVector<double>
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
/*
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
    for (int i = 0; i < n; i++) {
        out[i] = std::complex<double>(outArray[i][0], outArray[i][1]);
    }

    // Free the memory
    fftw_destroy_plan(p);
    fftw_free(inArray);
    fftw_free(outArray);
*/
    // Load the buffer with the initial values
    const int n = in.size();
    auto *d = new double[n];
    // Load the data
    memcpy(d, in.data(), n * sizeof(double));

#ifdef USE_FFTW
    fftw_plan p = fftw_plan_r2r_1d(n, d, d, FFTW_R2HC, FFTW_ESTIMATE);
    fftw_execute(p);
#else
    // Execute FFT
    gsl_fft_real_radix2_transform(d, 1, n);
#endif

    // Load the data into out
    out.resize(1 + n / 2);
    out[0] = std::complex<double>(d[0], 0);
    for (int i = 1; i < (out.size() - 1); ++i) {
        out[i] = std::complex<double>(d[i], d[n - i]);
    }
    out[out.size() - 1] = std::complex<double>(d[n - 1], 0);

    // Delete the buffer
    delete [] d;
}

void TimeSeriesMotion::ifft(const QVector<std::complex<double> >& in, QVector<double>& out ) const
{
/*
#ifdef USE_IFFTW
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
    for (int i = 0; i < out.size(); i++) {
        out[i] = outArray[i] / out.size();
    }

    // Free the memory
    fftw_destroy_plan(p);
    fftw_free(inArray);
    fftw_free(outArray);
*/
    const int n = 2 * (in.size() - 1);
    auto *d = new double[n];

    d[0] = in.first().real();
    for (int i = 1; i < in.size(); ++i) {
        d[i] = in.at(i).real();
        d[n - i] = in.at(i).imag();
    }
    d[n / 2] = in.last().real();

#if USE_FFTW
    fftw_plan p = fftw_plan_r2r_1d(n, d, d, FFTW_HC2R, FFTW_ESTIMATE);
    fftw_execute(p);

    for (int i = 0; i < n; ++i) {
        // Scale by n
        d[i] /= n;
    }
#else
    gsl_fft_halfcomplex_radix2_inverse(d, 1, n);
#endif

    // Copy results to output
    out.resize(n);
    memcpy(out.data(), d, n * sizeof(double));

    delete [] d;
}

auto TimeSeriesMotion::calcTimeSeries(QVector<std::complex<double> > fa, const QVector<std::complex<double> > & tf) const -> QVector<double>
{
    // Apply the transfer fucntion
    if (!tf.isEmpty()) {
        // If needed, zero pad the Fourier amplitudes such that is has the same length as the incoming transfer function.
        if (fa.size() < tf.size())
            fa.resize(tf.size());

        for (int i = 0; i < fa.size(); ++i)
            fa[i] *= tf.at(i);
    }

    // Compute the time series
    QVector<double> ts;
    ifft(fa, ts);

    return ts;
}

void TimeSeriesMotion::fromJson(const QJsonObject &json)
{
    AbstractMotion::fromJson(json);

    _saveData = json["saveData"].toBool();
    _fileName = json["fileName"].toString();
    _timeStep = json["timeStep"].toDouble();
    _pointCount = json["pointCount"].toInt();
    _scale = json["scale"].toDouble();
    _dataColumn = json["dataColumn"].toInt();
    _startLine = json["startLine"].toInt();
    _stopLine = json["stopLine"].toInt();

    setFormat(json["format"].toInt());
    setInputUnits(json["inputUnits"].toInt());

    if (_saveData) {
        Serialize::toDoubleVector(json["accel"], _accel);
    } else {
        load(_fileName, false, _scale);
    }

    if (_accel.size()) {
        calculate();
        _isLoaded = true;
    }
}

auto TimeSeriesMotion::toJson() const -> QJsonObject
{
    QJsonObject json = AbstractMotion::toJson();
    json["saveData"] = _saveData;
    json["fileName"] = _fileName;
    json["timeStep"] = _timeStep;
    json["pointCount"] = _pointCount;
    json["scale"] = _scale;
    json["format"] = _format;
    json["dataColumn"] = _dataColumn;
    json["startLine"] = _startLine;
    json["stopLine"] = _stopLine;
    json["inputUnits"] = (int) _inputUnits;

    if (_saveData) {
        json["accel"] = Serialize::toJsonArray(_accel);
    }
    return json;
}


auto operator<< (QDataStream & out, const TimeSeriesMotion* tsm) -> QDataStream &
{
    out << (quint8)3;

    out << qobject_cast<const AbstractMotion*>(tsm);

    // Properties for TimeSeriesMotion
    out
            << tsm->_saveData
            << tsm->_fileName
            << tsm->_timeStep
            << tsm->_pointCount
            << tsm->_scale
            << (int)tsm->_format
            << tsm->_dataColumn
            << tsm->_startLine
            << tsm->_stopLine;

    // Added in version 2
    out << (int)tsm->_inputUnits;

    // Save the data internally if requested
    if (tsm->_saveData) {
        out << tsm->_accel;
    }

    return out;
}

auto operator>> (QDataStream & in, TimeSeriesMotion* tsm) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractMotion*>(tsm);

    // Propertise for TimeSeriesMotion
    int format;

    in >> tsm->_saveData;

    if (ver < 3) {
        // Removed in version 3
        double freqMax;
        in >> freqMax;
    }

    in >> tsm->_fileName
            >> tsm->_timeStep
            >> tsm->_pointCount
            >> tsm->_scale
            >> format
            >> tsm->_dataColumn
            >> tsm->_startLine
            >> tsm->_stopLine;

    tsm->setFormat(format);

    if (ver > 1) {
        // Units added in version 2
        int units;
        in >> units;
        tsm->setInputUnits(units);
    }

    // Save the data internally if requested
    if (tsm->_saveData) {
        in >> tsm->_accel;
    } else {
        tsm->load(tsm->_fileName, false, tsm->_scale);
    }

    if (tsm->_accel.size()) {
        tsm->calculate();
        tsm->_isLoaded = true;
    }

    return in;
}
