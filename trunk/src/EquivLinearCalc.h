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
#include "TextLog.h"

#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>
#include <complex>
	
class EquivLinearCalc
{
    public:
        EquivLinearCalc();
        
        //! Reset the object to the default values
        void reset();

        void setMaxIterations( int maxIterations);	
        int maxIterations() const;

        void setStrainRatio( double strainRatio );
        double strainRatio() const;

        void setErrorTolerance( double errorTolerance );
        double errorTolerance() const;

        void run( TextLog & textLog, Motion * motion, SiteProfile * site);

        SiteProfile * site() const;
        Motion * motion() const; 

        //! Compute the maximum acceleration profile
        const QVector<double> maxAccelProfile() const;

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
         * \param location location of the desired transfer fucntion
         * \param tf the strain transfer function
         */
        void calcStrainTf( const Location & location, QVector<std::complex<double> > & tf ) const;

        //! Strain transfer function of a layer
        const QVector<std::complex<double> > & strainTfAt(int layer) const;

        QMap<QString, QVariant> toMap() const;
        void fromMap(const QMap<QString, QVariant> & map);

    private:
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

        //! Strain transfer function
        QVector<QVector<std::complex<double> > > m_strainTf;
        //@}

        //! Compute the complex shear modulus
        std::complex<double> calcCompShearMod( const double shearMod, const double damping ) const;

        //! Compute the up-going and down-going waves
        void calcWaves();

        /*! Return the combined waves for a given set of conditions
         * \param freqIdx index of the frequency
         * \param location the location in the site profile
         * \param type type of motion
         */
        const std::complex<double> waves( const int freqIdx, const Location & location, const Motion::Type type ) const;
};
#endif
