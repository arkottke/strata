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

#ifndef COMPATIBLE_RVT_MOTION_H
#define COMPATIBLE_RVT_MOTION_H

#include "AbstractRvtMotion.h"

#include <QDataStream>

class Dimension;

/*! RvtAbstractMotion defined by a response spectrum.
  */

class CompatibleRvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const CompatibleRvtMotion* crm);
    friend QDataStream & operator>> (QDataStream & in, CompatibleRvtMotion* crm);

public:
    CompatibleRvtMotion(QObject* parent = 0);
    virtual ~CompatibleRvtMotion();

    virtual const QVector<double> & freq() const;
    Dimension* freqDimension();

    //! Target response spectrum
    ResponseSpectrum * targetRespSpec();

    //! Limit the FAS to be seismologically plausible
    bool limitFas() const;

    //! Create a html document containing the information of the model
    virtual QString toHtml() const;

    //! Load the motion from a TextStream
    virtual bool loadFromTextStream(QTextStream &stream, double scale = 1.);

public slots:
    void setDuration(double duration);
    void setLimitFas(bool limitFas);

    /*! Compute the FAS from a response spectrum.
     * The targetPeriod, targetSa, and targetDamping must be set ahead of calling the inversion
     * \return returns true if the inversion provided a valid result (determined by rmse value)
     */
    virtual void calculate();

private:
    /*! Compute the FAS based on the Vanmarcke 1976 inversion technique.
     * \return Fourier amplitude spectrum spaced at the same periods as the target response spectrum
     */
    QVector<double> vanmarckeInversion() const;

    //! If the FAS should be corrected to better fit theory
    bool m_limitFas;

    //! Target response spectrum
    ResponseSpectrum * m_targetRespSpec;

    //! Frequency dimension
    Dimension* m_freq;
};

#endif // COMPATIBLE_RVT_MOTION_H
