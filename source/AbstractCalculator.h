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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ABSTRACT_CALCULATOR_H
#define ABSTRACT_CALCULATOR_H

#include <QObject>

#include "AbstractMotion.h"

#include <QVector>

#include <gsl_multifit.h>

#include <complex>

class Location;
class SoilProfile;
class TextLog;

class AbstractCalculator : public QObject
{
    Q_OBJECT

public:
    explicit AbstractCalculator(QObject *parent = 0);

    //! Produce an HTML table summarizing the calculator
    virtual QString toHtml() const;

    SoilProfile* site() const;
    AbstractMotion* motion() const;

    //! Set text log to record calculation steps
    void setTextLog(TextLog* const textLog);

    //! Reset the object to the default values
    virtual void reset();

    //! Perform the site response calculation
    virtual bool run(AbstractMotion* motion, SoilProfile* site) = 0;

    //! Compute the peak ground acceleration at the surface
    double surfacePGA() const;

    /*! Compute the acceleration transfer functions for a specific layer.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation output location
     * \param outputType type of output
     * \return the transfer function
     */
    const QVector<std::complex<double> > calcAccelTf(
            const Location & inLocation, const AbstractMotion::Type inputType,
            const Location & outLocation, const AbstractMotion::Type outputType ) const;

    /*! Compute the velocity to strain transfer function.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation location of the desired transfer fucntion
     * \return tf the strain transfer function to be applied to the Fourier amplitude spectrum of the velocity
     */
    QVector<std::complex<double> > calcStrainTf(
            const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const;

    /*! Compute the velocity to visco-elastic stress transfer function.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation location of the desired transfer fucntion
     * \return tf the strain transfer function to be applied to the Fourier amplitude spectrum of the velocity
     */
    QVector<std::complex<double> > calcStressTf(
            const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const;
signals:
    void wasModified();

public slots:
    void stop();

protected:
    //! Initialize vectors
    void init(AbstractMotion* motion, SoilProfile* site);

    //! Compute the complex shear modulus
    std::complex<double> calcCompShearMod(const double shearMod, const double damping ) const;

    /*! Compute the up-going and down-going waves
     * \return if the calculation is successful
     */
    bool calcWaves();

    /*! Return the combined waves for a given set of conditions
     * \param freqIdx index of the frequency
     * \param location the location in the site profile
     * \param type type of motion
     */
    const std::complex<double> waves(const int freqIdx, const Location & location, const AbstractMotion::Type type) const;

    //! Site profile
    SoilProfile* m_site;

    //! Motion that is propagated through the site profile.
    AbstractMotion* m_motion;

    //! Number of SubLayers in the site profile
    int m_nsl;

    //! Number of frequency points in the motion
    int m_nf;

    //! If the calculation should continue
    bool m_okToContinue;

    /*! @name Wave propagation parameters
     *
     * The first dimension of the vectors are the number of soil layers, the
     * second is the frequency.
     */
    //@{
    //! Complex shear modulus
    QVector<QVector<std::complex<double> > > m_shearMod;

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

#endif // ABSTRACT_CALCULATOR_H
