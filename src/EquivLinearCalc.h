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

#ifndef EQUIV_LINEAR_CALC_H_
#define EQUIV_LINEAR_CALC_H_

#include "Motion.h"
#include "SiteProfile.h"

#include <complex>

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

//! Calculates the site response using the equivalent linear framework.
	
class EquivLinearCalc : public QObject
{
    Q_OBJECT

    public:
        EquivLinearCalc( bool * okToContinue = 0, QObject * parent = 0);
        
        //! Reset the object to the default values
        void reset();

        int maxIterations() const;
        double strainRatio() const;
        double errorTolerance() const;

        bool converged() const;

        //! Set text log to record calculation steps
        void setTextLog(TextLog * textLog);

        bool run( Motion * motion, SiteProfile * site);

        SiteProfile * site() const;
        Motion * motion() const; 

        //! Compute the peak ground acceleration at the surface
        double surfacePGA() const;

        //! Compute the maximum acceleration profile
        const QVector<double> maxAccelProfile() const;

        //! Compute the maximum velocity profile -- not baseline corrected
        const QVector<double> maxVelProfile() const;

        /*! Compute the acceleration transfer functions for a specific layer.
         *
         * \param inLocation input location
         * \param inputType type of input 
         * \param outLocation output location
         * \param outputType type of output 
         * \return the transfer function
         */
        const QVector<std::complex<double> > calcAccelTf(
                const Location & inLocation, const Motion::Type inputType,
                const Location & outLocation, const Motion::Type outputType ) const;

        /*! Compute the acceleration to strain transfer function.
         *
         * \param inLocation input location
         * \param inputType type of input
         * \param outKocation location of the desired transfer fucntion
         * \param tf the strain transfer function
         * \return if the calculation is successful
         */
        bool calcStrainTf( const Location & inLocation, const Motion::Type inputType,
                           const Location & outLocation, QVector<std::complex<double> > & tf ) const;

        QMap<QString, QVariant> toMap() const;
        void fromMap(const QMap<QString, QVariant> & map);

    signals:
        void wasModified();

    public slots:
        void stop();
        void setMaxIterations( int maxIterations);	
        void setStrainRatio( double strainRatio );
        void setErrorTolerance( double errorTolerance );

    private:
        //! Compute the complex shear modulus
        std::complex<double> calcCompShearMod( const double shearMod, const double damping ) const;

        /*! Compute the up-going and down-going waves
         * \return if the calculation is successful
         */
        bool calcWaves();

        /*! Return the combined waves for a given set of conditions
         * \param freqIdx index of the frequency
         * \param location the location in the site profile
         * \param type type of motion
         */
        const std::complex<double> waves( const int freqIdx, const Location & location, const Motion::Type type ) const;

        //! Maximum number of iterations in the equivalent linear loop
        int m_maxIterations;

        //! Error tolerance of the equivalent linear loop -- percent
        double m_errorTolerance;

        //! Ratio between the maximum strain and the strain of the layer
        double m_strainRatio;

        //! Site profile
        SiteProfile * m_site;

        //! Motion that is propagated through the site profile.
        Motion * m_motion;

        //! Number of SubLayers in the site profile
        int m_nsl;

        //! Number of frequency points in the motion
        int m_nf;

        //! If the error tolerance was achieved
        bool m_converged;

        //! If the calculation should continue
        bool m_okToContinue;

        /*! @name Wave propagation parameters
         *
         * The first dimension of the vectors are the number of soil layers, the
         * second is the frequency.
         */
        //@{
        //! Complex shear modulus
        QVector<std::complex<double> > m_shearMod;

        //! Up-going wave
        QVector<QVector<std::complex<double> > > m_waveA;

        //! Down-going wave
        QVector<QVector<std::complex<double> > > m_waveB;

        //! Complex wave number
        QVector<QVector<std::complex<double> > > m_waveNum;
        //@}
        
        //! Text log to record calculation steps
        TextLog * m_textLog;
};
#endif
