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

#ifndef TIME_SERIES_MOTION_H_
#define TIME_SERIES_MOTION_H_

#include "AbstractMotion.h"

#include <QDataStream>
#include <QJsonObject>

#include <complex>

class TimeSeriesMotion : public AbstractMotion {
  Q_OBJECT

  friend auto operator<<(QDataStream &out, const TimeSeriesMotion *tsm)
      -> QDataStream &;
  friend auto operator>>(QDataStream &in, TimeSeriesMotion *tsm)
      -> QDataStream &;

public:
  TimeSeriesMotion(QObject *parent = nullptr);
  TimeSeriesMotion(const QString &fileName = "", double scale = 1.0,
                   AbstractMotion::Type type = AbstractMotion::Outcrop,
                   bool *successful = 0, QObject *parent = nullptr);
  virtual ~TimeSeriesMotion();

  //! Format of the text file
  enum Format {
    Rows,   /*!< Data is in rows */
    Columns /*!< Data is in columns */
  };

  //! Type of time series
  enum MotionType {
    Acceleration, //!< Acceleration time series
    Velocity, //!< Velocity time series -- single integrated acceleration time
              //!< series
    Displacement //!< Displacement time series -- double integrated acceleration
                 //!< time series
  };

  //! Units of the motion
  enum InputUnits {
    Gravity,                     //!< Gravity -- no unit conversion required
    CentimetersPerSecondSquared, //!< Centimenters per second squared
    InchesPerSecondSquared,      //!< Inches per second squared
    MetersPerSecondSquared       //!< Menters per second squared
  };

  static auto formatList() -> QStringList;

  virtual auto max(const QVector<std::complex<double>> &tf =
                       QVector<std::complex<double>>()) const -> double;
  virtual auto maxDisp(const QVector<std::complex<double>> &tf =
                           QVector<std::complex<double>>()) const -> double;
  virtual auto maxVel(const QVector<std::complex<double>> &tf =
                          QVector<std::complex<double>>()) const -> double;

  auto computeSa(const QVector<double> &period, double damping,
                 const QVector<std::complex<double>> &accelTf =
                     QVector<std::complex<double>>()) -> QVector<double>;

  virtual auto absFourierAcc(const QVector<std::complex<double>> &tf =
                                 QVector<std::complex<double>>()) const
      -> const QVector<double>;
  virtual auto absFourierVel(const QVector<std::complex<double>> &tf =
                                 QVector<std::complex<double>>()) const
      -> const QVector<double>;
  virtual auto calcMaxStrain(const QVector<std::complex<double>> &tf =
                                 QVector<std::complex<double>>()) const
      -> double;

  auto fileName() const -> QString;

  static auto inputUnitsList() -> QStringList;
  auto inputUnits() const -> InputUnits;
  void setInputUnits(InputUnits inputUnits);

  //! Time step of the motion
  auto timeStep() const -> double;

  virtual auto freq() const -> const QVector<double> &;

  virtual auto freqMax() const -> double;

  //! Nyquist frequency -- maximum frequency defined by rawTimeStep
  auto freqNyquist() const -> double;

  //! Point count of the untruncated time series
  auto pointCount() const -> int;
  auto scale() const -> double;

  auto format() const -> Format;
  void setFormat(Format format);

  auto dataColumn() const -> int;
  auto startLine() const -> int;
  auto stopLine() const -> int;

  auto time() const -> QVector<double>;
  auto accel() const -> const QVector<double> &;

  //! Compute time series for a given transfer function
  auto timeSeries(
      MotionType type = Acceleration,
      const QVector<std::complex<double>> &tf = QVector<std::complex<double>>(),
      const bool baselineCorrect = false) const -> QVector<double>;

  //! Compute a strain time series based on the velocity Fourier amplitude
  //! spectrum
  auto strainTimeSeries(
      const QVector<std::complex<double>> &tf = QVector<std::complex<double>>(),
      const bool baseLineCorrect = false) const -> QVector<double>;

  //! Compute the Arias Intensity of the time series for a given transfer
  //! function
  auto ariasIntensity(const QVector<std::complex<double>> &tf =
                          QVector<std::complex<double>>()) const
      -> QVector<double>;

  virtual auto name() const -> QString;

  //! Create a html document containing the information of the model
  auto toHtml() const -> QString;

  /*! Load the time series information from a file
   * \param fileName pathname of the file to load
   * \param if the default parameters should be used
   * \param scale factor to apply to the motion
   * \return if the motion was successfully loaded.
   */
  auto load(const QString &fileName, bool defaults = true, double scale = 1.0)
      -> bool;

  //!@{ Methods for viewing the Fourier amplitude spectrum of the motion
  virtual auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;
  virtual auto columnCount(const QModelIndex &parent = QModelIndex()) const
      -> int;

  virtual auto data(const QModelIndex &index, int role = Qt::DisplayRole) const
      -> QVariant;
  virtual auto headerData(int section, Qt::Orientation orientation,
                          int role = Qt::DisplayRole) const -> QVariant;
  //!@}

  void setSaveData(bool b);
  auto saveData() const -> bool;
  auto isLoaded() const -> bool;

  void fromJson(const QJsonObject &json);
  auto toJson() const -> QJsonObject;

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
  void setFileName(QString fileName);
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
  void processFile(std::ifstream *);

  //! Find the maximum absolute value of a vector
  auto findMaxAbs(const QVector<double> &vector) const -> double;

  /*! Compute the integral of the time series using the trapezoid rule.
   * \param in time series to be integrated
   */
  auto integrate(const QVector<double> &in) const -> QVector<double>;

  /*! Fit a polynomial to the time series using least squares regression.
   * \param term number of terms in the polynomial (order + 1)
   * \param series the time series
   */
  auto baselineFit(const int term, const QVector<double> &series) const
      -> const QVector<double>;

  //! Apply the transfer function to the Fourier amplitude and compute the
  //! absolute value
  auto absFourier(const QVector<std::complex<double>> &fa,
                  const QVector<std::complex<double>> &tf) const
      -> QVector<double>;

  //! Compute the Fast Fourier Transform from real to complex using the FFTW
  void fft(const QVector<double> &in, QVector<std::complex<double>> &out) const;

  //! Compute the Inverse Fast Fourier Transformation from complex to real using
  void ifft(const QVector<std::complex<double>> &in,
            QVector<double> &out) const;

  //! Compute the time series by applying a transfer function to a specified
  //! Fourier amplitude spectrum
  auto calcTimeSeries(QVector<std::complex<double>> fa,
                      const QVector<std::complex<double>> &tf) const
      -> QVector<double>;

  //! The conversion factor for the input motion
  auto unitConversionFactor() const -> double;

  //! Columns for the data view
  enum Column { TimeColumn, AccelColumn };

  //! The filename of the time series
  QString _fileName;

  //! The time step of the time series from the input file
  double _timeStep;

  //! The number of points in the time series
  int _pointCount;

  //! The scale factor that is applied to the motion
  double _scale;

  //! Units of the motion in the input file
  InputUnits _inputUnits;

  //! The format of the text file
  Format _format;

  //! The column of the acceleration data
  int _dataColumn;

  //! The line the data starts on
  int _startLine;

  //! The line the data stops on, 0 for complete file.
  int _stopLine;

  //! The acceleration data points in g
  QVector<double> _accel;

  //! Frequency of the Fourier amplitude spectrum
  QVector<double> _freq;

  //! The complex valued Fourier amplitude of the acceleration time series
  QVector<std::complex<double>> _fourierAcc;

  //! The complex valued Fourier amplitude of the velocity time series
  QVector<std::complex<double>> _fourierVel;

  //! If the acceleration data should be saved
  bool _saveData;

  //! If the motion has been loaded from the file
  bool _isLoaded;
};
#endif
