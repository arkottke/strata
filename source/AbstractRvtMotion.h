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

#ifndef ABSTRACT_RVT_MOTION_H
#define ABSTRACT_RVT_MOTION_H

#include "AbstractMotion.h"

#include "AbstractPeakCalculator.h"

#include <QAbstractTableModel>
#include <QDataStream>
#include <QJsonObject>
#include <QProgressBar>
#include <QTextStream>
#include <QVector>

#include <gsl/gsl_integration.h>

class AbstractRvtMotion : public AbstractMotion
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractRvtMotion* arm);
    friend QDataStream & operator>> (QDataStream & in, AbstractRvtMotion* arm);

public:
    AbstractRvtMotion(QObject *parent = nullptr);
    virtual ~AbstractRvtMotion();

    //! Location of model
    enum Region {
        WUS, //!< Generic Western North America Parameters
        CEUS, //!< Generic Eastern North America Parameters
        Unknown //!< Unknown region
    };

    static QStringList regionList();

    Region region() const;
    double magnitude() const;
    double distance() const;

    virtual void setRegion(AbstractRvtMotion::Region region);

    //!@{ Methods for viewing the Fourier amplitude spectrum of the motion
    virtual int rowCount(const QModelIndex& parent) const;
    virtual int columnCount(const QModelIndex& parent) const;

    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    //!@}

    //! Compute the maximum response of the motion and the applied transfer function
    double max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    double maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    double maxDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    double calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;

    //! Compute the acceleration response spectrum
    QVector<double> computeSa(const QVector<double>& period, double damping,
                              const QVector<std::complex<double> >& accelTf = QVector<std::complex<double> >());

    const QVector<double> absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    const QVector<double> absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;
    const QVector<double> absFourierDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const;

    //! A reference to the Fourier amplitude spectrum
    const QVector<double> & fourierAcc() const;

    virtual double freqMax() const;

    //! Duration of the motion
    double duration() const;

    //! Suitable name of the motion
    QString name() const;

    //! Template without the scenario filled in
    const QString& nameTemplate() const;

    //! Create a html document containing the information of the model
    virtual QString toHtml() const = 0;

    //! Load the motion from a TextStream
    virtual bool loadFromTextStream(QTextStream &stream, double scale = 1.);

    virtual void fromJson(const QJsonObject &json);
    virtual QJsonObject toJson() const;

public slots:
    //! Stop the current calculation
    void stop();

    //! Calculate the response spectrum of the motion
    virtual void calculate();

    void setRegion(int region);
    void setMagnitude(double magnitude);
    void setDistance(double distance);
    void setName(const QString &name);

signals:
    void progressRangeChanged(int minimum, int maximum);
    void progressValueChanged(int value);

    void fourierSpectrumChanged();

    void oscCorrectionChanged(double oscChanged);

    void regionChanged(int region);
    void magnitudeChanged(double magnitude);
    void distanceChanged(double distance);

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
         * \return the expected maximum value
         */
    double calcMax(const QVector<double> & fourierAmps) const;

    //! Take a comma separated string, split the columns, and return the specified column
    static QString extractColumn(const QString &line, int column=1);

    //! Update the scenario of the peak calculator if needed
    void updatePeakCalculatorScenario();

    //! Amplitudes of the Fourier amplitude spectrum corresponding to the acceleration
    QVector<double> _fourierAcc;

    //! Duration of the ground motion at the rock
    double _duration;

    //! Name of the motion
    QString _name;

    //! Peak calculator
    AbstractPeakCalculator *_peakCalculator;

    Region _region;
    double _magnitude;
    double _distance;

    //! If the current calculation should continue
    bool _okToContinue;
};

AbstractRvtMotion* loadRvtMotionFromTextFile(const QString &fileName, double scale = 1.);

#endif // ABSTRACT_RVT_MOTION_H
