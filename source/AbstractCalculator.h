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

#ifndef ABSTRACT_CALCULATOR_H
#define ABSTRACT_CALCULATOR_H

#include <QObject>

#include "AbstractMotion.h"

#include <QVector>

#include <gsl/gsl_multifit.h>

#include <complex>

class Location;
class SoilProfile;
class TextLog;

class AbstractCalculator : public QObject
{
    Q_OBJECT

public:
    explicit AbstractCalculator(QObject *parent = nullptr);

    //! Produce an HTML table summarizing the calculator
    virtual auto toHtml() const -> QString;

    auto site() const -> SoilProfile*;
    auto motion() const -> AbstractMotion*;

    //! Set text log to record calculation steps
    void setTextLog(TextLog* const textLog);

    //! Reset the object to the default values
    virtual void reset();

    //! Perform the site response calculation
    virtual auto run(AbstractMotion* motion, SoilProfile* site) -> bool = 0;

    //! If the analysis converged
    virtual auto converged() const -> bool = 0;

    //! Compute the peak ground acceleration at the surface
    auto surfacePGA() const -> double;

    /*! Compute the acceleration transfer functions for a specific layer.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation output location
     * \param outputType type of output
     * \return the transfer function
     */
    auto calcAccelTf(
            const Location & inLocation, const AbstractMotion::Type inputType,
            const Location & outLocation, const AbstractMotion::Type outputType ) const -> const QVector<std::complex<double> >;

    /*! Compute the velocity to strain transfer function.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation location of the desired transfer fucntion
     * \return tf the strain transfer function to be applied to the Fourier amplitude spectrum of the velocity
     */
    auto calcStrainTf(
            const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const -> QVector<std::complex<double> >;

    /*! Compute the velocity to visco-elastic stress transfer function.
     *
     * \param inLocation input location
     * \param inputType type of input
     * \param outLocation location of the desired transfer fucntion
     * \return tf the strain transfer function to be applied to the Fourier amplitude spectrum of the velocity
     */
    auto calcStressTf(
            const Location & inLocation, const AbstractMotion::Type inputType, const Location & outLocation) const -> QVector<std::complex<double> >;
signals:
    void wasModified();

public slots:
    void stop();

protected:
    //! Initialize vectors
    void init(AbstractMotion* motion, SoilProfile* site);

    //! Compute the complex shear modulus
    static auto calcCompShearMod(const double shearMod, const double damping ) -> std::complex<double> ;

    /*! Compute the up-going and down-going waves
     * \return if the calculation is successful
     */
    auto calcWaves() -> bool;

    /*! Return the combined waves for a given set of conditions
     * \param freqIdx index of the frequency
     * \param location the location in the site profile
     * \param type type of motion
     */
    auto waves(const int freqIdx, const Location & location, const AbstractMotion::Type type) const -> const std::complex<double>;

    //! Site profile
    SoilProfile* _site;

    //! Motion that is propagated through the site profile.
    AbstractMotion* _motion;

    //! Number of SubLayers in the site profile
    int _nsl;

    //! Number of frequency points in the motion
    int _nf;

    //! If the calculation should continue
    bool _okToContinue;

    /*! @name Wave propagation parameters
     *
     * The first dimension of the vectors are the number of soil layers, the
     * second is the frequency.
     */
    //@{
    //! Complex shear modulus
    QVector<QVector<std::complex<double> > > _shearMod;

    //! Up-going wave
    QVector<QVector<std::complex<double> > > _waveA;

    //! Down-going wave
    QVector<QVector<std::complex<double> > > _waveB;

    //! Complex wave number
    QVector<QVector<std::complex<double> > > _waveNum;
    //@}

    //! Text log to record calculation steps
    TextLog * _textLog;
};

#endif // ABSTRACT_CALCULATOR_H
