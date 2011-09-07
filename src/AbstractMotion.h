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

#include "MyAbstractTableModel.h"

class ResponseSpectrum;

#include <QString>
#include <QStringList>
#include <QVector>
#include <complex>

class AbstractMotion : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractMotion* m);
    friend QDataStream & operator>> (QDataStream & in, AbstractMotion* m);

public:
    AbstractMotion(QObject * parent = 0);
    virtual ~AbstractMotion() = 0;

    //! Type of motion
    enum Type {
        Outcrop, //!< AbstractMotion has free surface amplification and contains both incoming (A) and reflected (B) waves are identical
        Within, //!< AbstractMotion contains both incoming and reflected waves
        IncomingOnly //!< AbstractMotion contains only the incoming wave
    };

    static QStringList typeList();
    static Type variantToType(QVariant variant, bool* ok);

    bool modified() const;

    Type type() const;
    void setType(Type type);

    bool enabled() const;
    void setEnabled(bool enabled);

    //! A reference to the response spectrum
    ResponseSpectrum* respSpec();

    //! A reference to the frequency
    virtual const QVector<double> & freq() const = 0;

    //! Minimum useable frequency
    double freqMin() const;

    //! Maximum useable frequency
    virtual double freqMax() const = 0;

    int freqCount() const;
    double freqAt(int i) const;
    double angFreqAt(int i) const;

    //! Compute the acceleration response spectrum
    /*!
         * \param period periods of the response spectrum
         * \param damping damping of the response spectrum
         * \param accelTf input acceleration transfer function
         * \return response spectrum
         */
    virtual QVector<double> computeSa(const QVector<double>& period, double damping,
                                      const QVector<std::complex<double> >& accelTf = QVector<std::complex<double> >()) = 0;

    /*! Maximum response of a time series.
     *
     * \param tf transfer function to be applied to the acceleration FAS motion
     * \return maximum value
     */
    virtual double max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const = 0;

    //! Maximum response of the velocity time series.
    virtual double maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >(), bool applyScale = true) const = 0;

    //! Maximum response of the displacement time series.
    virtual double maxDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >(), bool applyScale = true) const = 0;

    //! Calculate the strain for the transfer function and the velocity FAS
    virtual double calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const = 0;

    //! A suitable name for the motion
    virtual QString name() const = 0;

    //! A description of the motion
    QString description() const;

    double pga() const;
    double pgv() const;

    //! Create a html document containing the information of the model
    virtual QString toHtml() const = 0;

    //! The absolute value of the acceleration Fourier amplitude spectrum with the applied transfer function
    virtual const QVector<double> absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const = 0;

    //! The absolute value of the velocity Fourier amplitude spectrum with the applied transfer function
    virtual const QVector<double> absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const = 0;

public slots:
    void setModified(bool modified = true);
    void setType(int type);


    void setDescription(QString s);

signals:
    void wasModified();

    void pgaChanged(double pga);
    void pgvChanged(double pgv);

    void descriptionChanged(QString description);

protected slots:
    void scaleProperties();

protected:    
    /*! Compute the transfer function for a single degree of freedom oscillator.
         *
         * \param period natural periods of the oscillator
         * \param damping damping of the oscillator in percent
         */
    QVector<std::complex<double> > calcSdofTf(const double period, double damping) const;

    //! Set the PGA and signal that it has been changed
    void setPga(double pga);

    //! Set the PGV and signal that it has been changed
    void setPgv(double pgv);

    //! If the motion has been modified and attributes need to be re-computed
    // FIXME still needed?
    bool m_modified;

    //! Type of motion
    Type m_type;

    //! The description of the time series
    QString m_description;

    //! The peak ground acceleration in the time domain in units of g
    double m_pga;

    //! The peak ground velocity in the defined units either cm/sec or in/sec
    double m_pgv;

    //! Controls if the AbstractMotions is to be used
    bool m_enabled;

    //! Response spectrum
    ResponseSpectrum * m_respSpec;

    //! Store if the SDOF transfer function has been computed
    bool m_sdofTfIsComputed;

    //! The single-degree-of-freedom oscillator transfer function
    QVector<QVector<std::complex<double> > > m_sdofTf;
};
#endif
