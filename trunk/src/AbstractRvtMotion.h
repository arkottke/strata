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

#ifndef ABSTRACT_RVT_MOTION_H
#define ABSTRACT_RVT_MOTION_H

#include "AbstractMotion.h"

#include <QAbstractTableModel>
#include <QProgressBar>
#include <QTextStream>
#include <QVector>

#include <qwt_data.h>

#include <gsl/gsl_integration.h>

class AbstractRvtMotion : public AbstractMotion, public QwtData
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractRvtMotion* arm);
    friend QDataStream & operator>> (QDataStream & in, AbstractRvtMotion* arm);

public:
    AbstractRvtMotion(QObject * parent = 0);
    virtual ~AbstractRvtMotion();

    //! Oscillator duration correction
    enum OscillatorCorrection {
        BooreJoyner, //!< Boore and Joyner (1984)
        LiuPezeshk //!< Liu and Pezeshk (1999)
    };

    //!@{ Methods for viewing the Fourier amplitude spectrum of the motion
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    //!@}

    //!@{ Methods for plotting working with the QwtData interface
    virtual QwtData* copy() const;
    virtual size_t size() const;
    virtual double x(size_t i) const;
    virtual double y(size_t i) const;
    //!@}

    //! Compute the maximum reponse of the motion and the applied transfer function
    virtual double max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    virtual double maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >(), bool applyScale = true) const;
    virtual double calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;


    //! Compute the acceleration response spectrum
    QVector<double> computeSa(const QVector<double>& period, double damping,
                              const QVector<std::complex<double> >& tf = QVector<std::complex<double> >());

    virtual const QVector<double> absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    virtual const QVector<double> absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;

    //! A reference to the Fourier amplitude spectrum
    const QVector<double> & fas() const;

    virtual double freqMax() const;

    //! Duration of the motion
    double duration() const;

    //! Suitable name of the motion
    virtual QString name() const;

    //! Create a html document containing the information of the model
    virtual QString toHtml() const = 0;

    //! Load the motion from a TextStream
    virtual bool loadFromTextStream(QTextStream &stream, double scale = 1.);

    void setOscCorrection(OscillatorCorrection oscCorrection);

public slots:
    //! Stop the current calculation
    void stop();

    //! Calculate the response spectrum of the motion
    virtual void calculate();

    //! Set the oscillator correction
    void setOscCorrection(int oscCorrection);
    void setName(const QString &name);

signals:
    void progressRangeChanged(int minimum, int maximum);
    void progressValueChanged(int value);

    void fourierSpectrumChanged();

    void oscCorrectionChanged(double oscChanged);

protected:
    //! Columns for the table
    enum Column {
        FrequencyColumn,
        AmplitudeColumn
    };

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
    double calcMax(const QVector<double> & fas, double durationRms = -1) const;

    /*! Compute the maximum response of an oscillator.
         */
    double calcOscillatorMax(QVector<double> fas, const double period, const double damping) const;

    /*! Compute the moment of a function using the trapzoid rule.
         *
         * \param power the moment power
         * \param fasSqr the squared Fourier amplitude spectrum
         */
    double moment(int power, const QVector<double> & fasSqr) const;

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

    //! Take a comma separated string, split the columns, and return the specified column
    static QString extractColumn(const QString &line, int column=1);

    //! Compute the Fourier amplitudes corresponding to the velocity timeseries
    QVector<double> calcFourierVel() const;

    //! Oscillator corretion method
    OscillatorCorrection m_oscCorrection;

    //! Amplitudes of the Fourier amplitude spectrum corresponding to the acceleration
    QVector<double> m_fourierAcc;

    //! Duration of the ground motion at the rock
    double m_duration;

    //! If the current calculation should continue
    bool m_okToContinue;

    //! Workspace for the peak factor integration
    gsl_integration_workspace* m_workspace;

    //! Name of the motion
    QString m_name;
};

AbstractRvtMotion* loadRvtMotionFromTextFile(const QString &fileName, double scale = 1.);

#endif // ABSTRACT_RVT_MOTION_H
