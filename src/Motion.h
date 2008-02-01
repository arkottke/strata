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

#ifndef MOTION_H_
#define MOTION_H_

#include "ResponseSpectrum.h"

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QVector>
#include <complex>

class Motion : public QObject
{
  Q_OBJECT

  public:
    Motion(QObject * parent = 0);
    virtual ~Motion();

    //! Type of motion
    enum Type { 
      Outcrop, //!< Motion recorded at the surface of the earth
      Within //!< Motion recorded at depth
    };

    //! The type of duration
    enum DurationType {
      Rock, //!< Original duration is used
      Soil, //!< Soil duration is used
      Strain //!< Strain duration is used
    };

    static QStringList typeList();

    bool modified() const;

    Type type() const;
    void setType(Type type);

    //! A reference to the response spectrum
    ResponseSpectrum * respSpec();

    //! A reference to the frequency
    QVector<double> & freq();

    //! Minimum useable frequency
    const double freqMin() const;

    //! Maximum useable frequency
    const double freqMax() const;

    int freqCount() const;
    double freqAt(int i) const;
    double angFreqAt(int i) const;

    //! Compute the acceleration response spectrum
    /*!
     * \param durationType type of duration to use in the calculation
     * \param period periods of the response spectrum
     * \param damping damping of the response spectrum
     * \param accelTf input acceleration transfer function
     * \return response spectrum
     */
    virtual QVector<double> computeSa( DurationType durationType, const QVector<double> & period, double damping,
        const QVector<std::complex<double> > & accelTf = QVector<std::complex<double> >() ) = 0;

    //! Maximum response of a time series
    /*!
     * \param durationType type of duration to use in the calculation
     * \param tf transfer function to be applied to the motion
     * \return maximum value
     */
    virtual double max( DurationType durationType, const QVector<std::complex<double> > & tf = QVector<std::complex<double> >()) const = 0;

    //! A suitable indentifier of the motion
    virtual QString toString() const = 0;

    //! If the motion can provide a time series
    virtual bool hasTime() const = 0;

    virtual QMap<QString, QVariant> toMap(bool saveData = false) const = 0;
    virtual void fromMap( const QMap<QString, QVariant> & map) = 0;

    //! Create a html document containing the information of the model
    virtual QString toHtml() const = 0;

  public slots:
    void setModified(bool modified = true);


  protected:
    //! If the motion has been modified and attributes need to be re-computed
    bool m_modified;

    //! Type of motion
    Type m_type;

    //! Frequency of the Fourier amplitude spectrum
    QVector<double> m_freq;

    //! Response spectrum
    ResponseSpectrum * m_respSpec;

    //! Store if the SDOF transfer function has been computed
    bool m_sdofTfIsComputed;

    //! The single-degree-of-freedom oscillator transfer function
    QVector<QVector<std::complex<double> > > m_sdofTf;

    //! Compute the transfer function for a single degree of freedom oscillator
    void computeSdofTf( const QVector<double> & period, double damping );		
};
#endif