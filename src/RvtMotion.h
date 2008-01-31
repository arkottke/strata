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

#ifndef RVT_MOTION_H_
#define RVT_MOTION_H_

#include "Motion.h"

#include <QVector>
#include <QProgressBar>

//! A class that uses random vibration theory to compute the reponse
/*!
 * The motion is characterized by the Fourier Amplitude and random vibration
 * thereory is used to predict the expected response.
 */

class RvtMotion : public Motion
{
  Q_OBJECT

  public:
    //! Constructor
    RvtMotion();

    //! Source of the RVT motion
    enum Source {
      DefinedFourierSpectrum, //!< Direct use of Fourier spectrum
      DefinedResponseSpectrum //!< Derived from a response spectrum 
    };

    static QStringList sourceList();

    //! Reset the object to the default values
    void reset();

    //! Compute the maximum reponse of the motion and the applied transfer function    
    double max( DurationType durationType, const QVector<std::complex<double> > & tf = QVector<std::complex<double> >()) const;

    QVector<double> computeSa( DurationType durationType, const QVector<double> & period, double damping,
        const QVector<std::complex<double> > & accelTf = QVector<std::complex<double> >() );

    //! A reference to the Fourier amplitude spectrum 
    QVector<double> & fas();

    double duration() const;
    void setDuration(double duration);

    double strainFactor() const;
    void setStrainFactor(double strainFactor);

    double soilFactor() const;
    void setSoilFactor(double soilFactor);

    Source source() const;
    void setSource( Source source );

    //! Target response spectrum
    ResponseSpectrum * targetRespSpec();

    //! Compute the FAS from a response spectrum.
    /*!
     * the targetPeriod, targetSa, and targetDamping must be set ahead of calling the inversion
     * \param okToContinue bool that must be true for the calculation to continue
     * \param progress a display of the status
     * \return returns true if the inversion provided a valid result (determined by rmse value)
     */
    bool invert( const bool * okToContinue = 0, QProgressBar * progress = 0);

    QString toString() const;

    //! If the motion can provide a time series
    bool hasTime() const;

    //! Convert the motion to a QMap for saving
    QMap<QString, QVariant> toMap( bool saveData = false ) const;

    //! Load the motion from a QMap
    void fromMap( const QMap<QString, QVariant> & map);

    //! Create a html document containing the information of the model
    QString toHtml() const;

  signals:
    void fourierSpectrumChanged();

  private:
    //! Source of the motion 
    Source m_source;

    //! Target response spectrum
    ResponseSpectrum * m_targetRespSpec;

    //! Amplitudes of the Fourier amplitude spectrum
    QVector<double> m_fas;

    //! Duration of the ground motion at the rock
    double m_duration;

    //! Rock to strain duration conversion factor
    double m_strainFactor;

    //! Rock to soil duration conversion factor
    double m_soilFactor;

    //! Compute the appropriate duration for the ground motion
    double durationOfGm(DurationType durationType) const;

    //! Compute the FAS based on the Brune Power Spectrum
    /*!
     * \param magnitude magnitude of the earthquake
     * \param distance distance from the earthquake (km)
     * \param stressDrop stress drop (bars)
     * \param kappa kappa value (seconds)
     */
    void brune( double magnitude = 7, double distance = 30, double stressDrop = 100, double kappa = 0.02);

    //! Compute the FAS based on a linear sections
    void simpleFas();

    /*! Compute the FAS based on the Vanmarcke 1976 inversion technique.
     * \param sa acceleration response spectrum with same spacing as desired FAS
     * \param damping damping of the response spectrum in percent
     */
    void vanmarckeInversion(const QVector<double> & sa, double damping);

    //! Compute the moment of a function using the trapzoid rule.
    /*!
     * \param power the moment power
     * \param fasSqr the squared Fourier amplitude spectrum 
     */
    double moment( int power, const QVector<double> & fasSqr) const;

    /*! Compute the maximum response using extreme value statistics.
     * Computes the peak response of a motion based on the procedure
     * described in Rathje and Ozbey 2004.  The method is called Random
     * Vibration Theory and uses extreme value statistics to predict the
     * expected maximum value.
     * 
     * \param fas the Fourier Amplitude Spectrum of the motion
     * \param durationGm duration of the ground motion
     * \param durationRms the root-mean-squared duration
     * \return the expected maximum value
     */
    double rvt( const QVector<double> & fas, double durationGm, double durationRms = -1 ) const;

    /*! Force the tails of the FAS down.
     * During the ratio correction process the tails of the Fourier Amplitude
     * Spectrum are increased because beyond the tails the transfer function is not
     * defined.  The result is an non-sensical FAS.
     *
     * \param dir The direction of the checking (1 or -1)
     */
    void forceDown(int dir = 1);

    /*! Compute the peak factor.
     * Peak factor propsed by Cartwright and Longuet-Higgins (1956).
     *
     * \param bandWidth bandwidth of the peak factor
     * \param numExtrema number of the extrema
     * \return the peak factor
     */
    double calcPeakFactor( const double bandWidth, const double numExtrema ) const;
};
#endif
