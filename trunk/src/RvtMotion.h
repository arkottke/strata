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
#include "PointSourceModel.h"

#include <QVector>
#include <QProgressBar>

#include <gsl/gsl_integration.h>

/*! A class that uses random vibration theory to compute the reponse.
 * The motion is characterized by the Fourier Amplitude and random vibration
 * thereory is used to predict the expected response.
 */

class RvtMotion : public Motion
{
    Q_OBJECT

    public:
        //! Constructor
        RvtMotion();

        //! Deconstructor
        ~RvtMotion();

        //! Source of the RVT motion
        enum Source {
            DefinedFourierSpectrum, //!< Direct use of Fourier spectrum
            DefinedResponseSpectrum, //!< Derived from a response spectrum 
            CalculatedFourierSpectrum //!< Fourier spectrum computed with source theory
        };

        //! Oscillator duration correction
        enum OscillatorCorrection{
            BooreJoyner, //!< Boore and Joyner (1984)
            LiuPezeshk //!< Liu and Pezeshk (1999)
        };

        static QStringList sourceList();

        //! Reset the object to the default values
        void reset();

        //! Compute the maximum reponse of the motion and the applied transfer function    
        double max(const QVector<std::complex<double> > & tf = QVector<std::complex<double> >()) const;

        QVector<double> computeSa(const QVector<double> & period, double damping,
                const QVector<std::complex<double> > & tf = QVector<std::complex<double> >());

        const QVector<double> absFas( const QVector<std::complex<double> > & tf = QVector<std::complex<double> >()) const;

        //! A reference to the Fourier amplitude spectrum 
        QVector<double> & fas();

        double duration() const;

        double strainFactor() const;

        double soilFactor() const;

        Source source() const;
        void setSource( Source source );

        bool limitFas() const;

        //! Target response spectrum
        ResponseSpectrum * targetRespSpec();
        
        //! Point source model
        PointSourceModel * pointSourceModel();

        /*! Compute the FAS from a response spectrum.
         * The targetPeriod, targetSa, and targetDamping must be set ahead of calling the inversion
         * \return returns true if the inversion provided a valid result (determined by rmse value)
         */
        bool invert();

        /*! Compute the FAS from the point source model.
         */
        void calcPointSource();

        QString toString() const;

        //! If the motion can provide a time series
        bool hasTime() const;

        //! Convert the motion to a QMap for saving
        QMap<QString, QVariant> toMap( bool saveData = false ) const;

        //! Load the motion from a QMap
        void fromMap( const QMap<QString, QVariant> & map);

        //! Create a html document containing the information of the model
        QString toHtml() const;

    public slots:
        //! Stop the current calculation
        void stop();
        
        void setSource(int source);
        void setDuration(double duration);
        void setLimitFas(bool limitFas);

    signals:
        void fourierSpectrumChanged();

        void progressRangeChanged(int minimum, int maximum);
        void progressValueChanged(int value);

    private:
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
         * \return Fourier amplitude spectrum spaced at the same periods as the target response spectrum
         */
        QVector<double> vanmarckeInversion() const;


        /*! Vary the magnitude and distance of the Brune model to fit the target response spectrum
        */
        //void fitBruneModel();

        /*!
         * Compute the minimum value assuming that the function
         */


        /*! Compute the moment of a function using the trapzoid rule.
         *
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
        double calcMax( const QVector<double> & fas, double durationRms = -1 ) const;

        /*! Compute the maximum response of an oscillator.
         */
        double calcOscillatorMax(QVector<double> fas, const double period, const double damping) const;

        /*! Compute the peak factor.
         * Peak factor propsed by Cartwright and Longuet-Higgins (1956).
         *
         */
        static double peakFactorEqn( double z, void * p );

        /*! Comptue the rms duration for a oscillator natural frequency.
         *
         * Apply the correct that was recommended by Boore 1984 to correct for changes in duration
         * when a motion is applied to an oscillator.
         *
         * \param durationGm duration of the ground motion
         * \param period natural period of the oscillator
         * \param damping damping of the oscillator in percent
         * \return rms duration
         */
        double calcRmsDuration(const double period, const double damping, const QVector<double> & fas = QVector<double>()) const;
        
        //! Source of the motion 
        Source m_source;

        //! Target response spectrum
        ResponseSpectrum * m_targetRespSpec;

        //! Oscillator corretion method
        OscillatorCorrection m_oscCorrection;

        //! Amplitudes of the Fourier amplitude spectrum
        QVector<double> m_fas;

        //! Duration of the ground motion at the rock
        double m_duration;

        //! If the FAS should be corrected to better fit theory
        bool m_limitFas;

        //! If the current calculation should continue
        bool m_okToContinue;

        //! Workspace for the peak factor integration
        gsl_integration_workspace * m_workspace;

        //! Point source model
        PointSourceModel * m_pointSourceModel;
};
#endif
