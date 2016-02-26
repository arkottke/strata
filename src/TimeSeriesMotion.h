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

#ifndef TIME_SERIES_MOTION_H_
#define TIME_SERIES_MOTION_H_

#include "AbstractMotion.h"

#include <QDataStream>
#include <QJsonObject>

#include <complex>

class TimeSeriesMotion : public AbstractMotion
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const TimeSeriesMotion* tsm);
    friend QDataStream & operator>> (QDataStream & in, TimeSeriesMotion* tsm);

public:
    TimeSeriesMotion(QObject * parent = 0);
    TimeSeriesMotion(const QString & fileName = "", double scale = 1.0,
                     AbstractMotion::Type type = AbstractMotion::Outcrop,
                     bool * successful = 0, QObject * parent = 0);
    virtual ~TimeSeriesMotion();

    //! Format of the text file
    enum Format {
        Rows,   /*!< Data is in rows */
        Columns /*!< Data is in columns */
    };

    //! Type of time series
    enum MotionType {
        Acceleration, //!< Acceleration time series
        Velocity, //!< Velocity time series -- single integrated acceleration time series
        Displacement //!< Displacement time series -- double integrated acceleration time series
    };

    //! Units of the motion
    enum InputUnits {
        Gravity, //!< Gravity -- no unit conversion required
        CentimetersPerSecondSquared, //!< Centimenters per second squared
        InchesPerSecondSquared //!< Inches per second squared
    };

    static QStringList formatList();

    virtual double max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    virtual double maxDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    virtual double maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;

    QVector<double> computeSa(const QVector<double> & period, double damping,
                               const QVector<std::complex<double> > & accelTf = QVector<std::complex<double> >() );

    virtual const QVector<double> absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    virtual const QVector<double> absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;    
    virtual double calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;

    QString fileName() const;

    static QStringList inputUnitsList();
    InputUnits inputUnits() const;
    void setInputUnits(InputUnits inputUnits);

    //! Time step of the motion
    double timeStep() const;

    virtual const QVector<double> & freq() const;

    virtual double freqMax() const;

    //! Nyquist frequency -- maximum frequency defined by rawTimeStep
    double freqNyquist() const;

    //! Point count of the untruncated time series
    int pointCount() const;
    double scale() const;

    Format format() const;
    void setFormat(Format format);

    int dataColumn() const;
    int startLine() const;
    int stopLine() const;

    QVector<double> time() const;
    const QVector<double> & accel() const;

    //! Compute time series for a given transfer function
    QVector<double> timeSeries(MotionType type = Acceleration, const QVector<std::complex<double> > & tf = QVector<std::complex<double> >(), const bool baselineCorrect = false) const;

    //! Compute a strain time series based on the velocity Fourier amplitude spectrum
    QVector<double> strainTimeSeries(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >(), const bool baseLineCorrect = false) const;

    //! Compute the Arias Intensity of the time series for a given transfer function
    QVector<double> ariasIntensity(const QVector<std::complex<double> > & tf = QVector<std::complex<double> >()) const;

    virtual QString name() const;

    //! Create a html document containing the information of the model
    QString toHtml() const;

    /*! Load the time series information from a file
      * \param fileName pathname of the file to load
      * \param if the default parameters should be used
      * \param scale factor to apply to the motion
      * \return if the motion was successfully loaded.
      */
    bool load(const QString &fileName, bool defaults = true, double scale = 1.0);

    //!@{ Methods for viewing the Fourier amplitude spectrum of the motion
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    //!@}

    void setSaveData(bool b);
    bool saveData() const;
    bool isLoaded() const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void fileNameChanged(QString fileName);
    void timeStepChanged(double timeStep);
    void pointCountChanged(int pointCount);
    void scaleChanged(double scale);
    void formatChanged(int format);
    void dataColumnChanged(int column);
    void startLineChanged(int lines);
    void stopLineChanged(int lines);

    void inputUnitsChanged(int units);

public slots:
    void setFileName(const QString & fileName);
    void setTimeStep(double timeStep);
    void setPointCount(int count);
    void setScale(double scale);
    void setFormat(int format);
    void setDataColumn(int column);
    void setStartLine(int lines);
    void setStopLine(int lines);

    void setInputUnits(int inputsUnits);

    //! Compute the Fourier amplitudes, response spectrum, and time values
    void calculate();
protected:
    //! Set that the file is loaded
    void setIsLoaded(bool isLoaded);

    //! Call the readFile and computeSpecAccel functions
    void processFile(std::ifstream*);

    //! Find the maximum absolute value of a vector
    double findMaxAbs( const QVector<double> & vector ) const;

    /*! Compute the integral of the time series using the trapezoid rule.
     * \param in time series to be integrated
     */
    QVector<double> integrate(const QVector<double> & in) const;

    /*! Fit a polynomial to the time series using least squares regression.
         * \param term number of terms in the polynomial (order + 1)
         * \param series the time series
         */
    const QVector<double> baselineFit( const int term, const QVector<double> & series ) const;

    //! Apply the transfer function to the Fourier amplitude and compute the absolute value
    QVector<double> absFourier(const QVector< std::complex<double> >& fa, const QVector<std::complex<double> >& tf) const;

    //! Compute the Fast Fourier Transform from real to complex using the FFTW
    void fft( const QVector<double>& in, QVector< std::complex<double> >& out) const;

    //! Compute the Inverse Fast Fourier Transformation from complex to real using
    void ifft(const QVector< std::complex<double> >& in, QVector<double>& out ) const;

    //! Compute the time series by applying a transfer function to a specified Fourier amplitude spectrum
    QVector<double> calcTimeSeries(QVector<std::complex<double> > fa, const QVector<std::complex<double> > & tf) const;

    //! The conversion factor for the input motion
    double unitConversionFactor() const;

    //! Columns for the data view
    enum Column {
        TimeColumn,
        AccelColumn
    };

    //! The filename of the time series
    QString m_fileName;

    //! The time step of the time series from the input file
    double m_timeStep;

    //! The number of points in the time series
    int m_pointCount;

    //! The scale factor that is applied to the motion
    double m_scale;

    //! Units of the motion in the input file
    InputUnits m_inputUnits;

    //! The format of the text file
    Format m_format;

    //! The column of the acceleration data
    int m_dataColumn;

    //! The line the data starts on
    int m_startLine;

    //! The line the data stops on, 0 for complete file.
    int m_stopLine;

    //! The acceleration data points in g
    QVector<double> m_accel;

    //! Frequency of the Fourier amplitude spectrum
    QVector<double> m_freq;

    //! The complex valued Fourier amplitude of the acceleration time series
    QVector<std::complex<double> > m_fourierAcc;

    //! The complex valued Fourier amplitude of the velocity time series
    QVector<std::complex<double> > m_fourierVel;

    //! If the acceleration data should be saved
    bool m_saveData;

    //! If the motion has been loaded from the file
    bool m_isLoaded;
};
#endif
