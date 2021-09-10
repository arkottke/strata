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

#ifndef ABSTRACT_MOTION_H_
#define ABSTRACT_MOTION_H_

#include "MyAbstractTableModel.h"

class ResponseSpectrum;

#include <QDataStream>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <complex>

class AbstractMotion : public MyAbstractTableModel
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractMotion* m) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractMotion* m) -> QDataStream &;

public:
    AbstractMotion(QObject *parent = nullptr);
    virtual ~AbstractMotion() = 0;

    //! Type of motion
    enum Type {
        Outcrop, //!< AbstractMotion has free surface amplification and contains both incoming (A) and reflected (B) waves are identical
        Within, //!< AbstractMotion contains both incoming and reflected waves
        IncomingOnly //!< AbstractMotion contains only the incoming wave
    };

    static auto typeList() -> QStringList;
    static auto variantToType(QVariant variant, bool* ok) -> Type;

    auto modified() const -> bool;

    auto type() const -> Type;
    void setType(Type type);

    auto enabled() const -> bool;
    void setEnabled(bool enabled);

    //! A reference to the response spectrum
    auto respSpec() -> ResponseSpectrum*;

    //! A reference to the frequency
    virtual auto freq() const -> const QVector<double> & = 0;

    //! Minimum useable frequency
    auto freqMin() const -> double;

    //! Maximum useable frequency
    virtual auto freqMax() const -> double = 0;

    auto freqCount() const -> int;
    auto freqAt(int i) const -> double;
    auto angFreqAt(int i) const -> double;

    //! Compute the acceleration response spectrum
    /*!
         * \param period periods of the response spectrum
         * \param damping damping of the response spectrum
         * \param accelTf input acceleration transfer function
         * \return response spectrum
         */
    virtual auto computeSa(const QVector<double>& period, double damping,
                                      const QVector<std::complex<double> >& accelTf = QVector<std::complex<double> >()) -> QVector<double> = 0;

    /*! Maximum response of a time series.
     *
     * \param tf transfer function to be applied to the acceleration FAS motion
     * \return maximum value
     */
    virtual auto max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double = 0;

    //! Maximum response of the velocity time series.
    virtual auto maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double = 0;

    //! Maximum response of the displacement time series.
    virtual auto maxDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double = 0;

    //! Calculate the strain for the transfer function and the velocity FAS
    virtual auto calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double = 0;

    //! A suitable name for the motion
    virtual auto name() const -> QString = 0;

    //! A description of the motion
    auto description() const -> QString;

    auto pga() const -> double;
    auto pgv() const -> double;

    //! Create a html document containing the information of the model
    virtual auto toHtml() const -> QString = 0;

    //! The absolute value of the acceleration Fourier amplitude spectrum with the applied transfer function
    virtual auto absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> const QVector<double> = 0;

    //! The absolute value of the velocity Fourier amplitude spectrum with the applied transfer function
    virtual auto absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> const QVector<double> = 0;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

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
    auto calcSdofTf(const double period, double damping) const -> QVector<std::complex<double> >;

    //! Set the PGA and signal that it has been changed
    void setPga(double pga);

    //! Set the PGV and signal that it has been changed
    void setPgv(double pgv);

    //! If the motion has been modified and attributes need to be re-computed
    // FIXME still needed?
    bool _modified;

    //! Type of motion
    Type _type;

    //! The description of the time series
    QString _description;

    //! The peak ground acceleration in the time domain in units of g
    double _pga;

    //! The peak ground velocity in the defined units either cm/sec or in/sec
    double _pgv;

    //! Controls if the AbstractMotions is to be used
    bool _enabled;

    //! Response spectrum
    ResponseSpectrum * _respSpec;

    //! Store if the SDOF transfer function has been computed
    bool _sdofTfIsComputed;

    //! The single-degree-of-freedom oscillator transfer function
    QVector<QVector<std::complex<double> > > _sdofTf;
};
#endif // ABSTRACT_MOTION_H_
