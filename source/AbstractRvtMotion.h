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

    friend auto operator<< (QDataStream & out, const AbstractRvtMotion* arm) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractRvtMotion* arm) -> QDataStream &;

public:
    AbstractRvtMotion(QObject *parent = nullptr);
    virtual ~AbstractRvtMotion();

    //! Location of model
    enum Region {
        WUS, //!< Generic Western North America Parameters
        CEUS, //!< Generic Eastern North America Parameters
        Unknown //!< Unknown region
    };

    static auto regionList() -> QStringList;

    auto region() const -> Region;
    auto magnitude() const -> double;
    auto distance() const -> double;

    virtual void setRegion(AbstractRvtMotion::Region region);

    //!@{ Methods for viewing the Fourier amplitude spectrum of the motion
    virtual auto rowCount(const QModelIndex& parent) const -> int;
    virtual auto columnCount(const QModelIndex& parent) const -> int;

    virtual auto data(const QModelIndex& index, int role) const -> QVariant;
    virtual auto headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const -> QVariant;
    //!@}

    //! Compute the maximum response of the motion and the applied transfer function
    auto max(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double;
    auto maxVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double;
    auto maxDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double;
    auto calcMaxStrain(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> double;

    //! Compute the acceleration response spectrum
    auto computeSa(const QVector<double>& period, double damping,
                              const QVector<std::complex<double> >& accelTf = QVector<std::complex<double> >()) -> QVector<double>;

    auto absFourierAcc(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> const QVector<double>;
    auto absFourierVel(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> const QVector<double>;
    auto absFourierDisp(const QVector<std::complex<double> >& tf = QVector<std::complex<double> >()) const -> const QVector<double>;

    //! A reference to the Fourier amplitude spectrum
    auto fourierAcc() const -> const QVector<double> &;

    virtual auto freqMax() const -> double;

    //! Duration of the motion
    auto duration() const -> double;

    //! Suitable name of the motion
    auto name() const -> QString;

    //! Template without the scenario filled in
    auto nameTemplate() const -> const QString&;

    //! Create a html document containing the information of the model
    virtual auto toHtml() const -> QString = 0;

    //! Load the motion from a TextStream
    virtual auto loadFromTextStream(QTextStream &stream, double scale = 1.) -> bool;

    virtual void fromJson(const QJsonObject &json);
    virtual auto toJson() const -> QJsonObject;

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
    auto calcMax(const QVector<double> & fourierAmps) const -> double;

    //! Take a comma separated string, split the columns, and return the specified column
    static auto extractColumn(const QString &line, int column=1) -> QString;

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

auto loadRvtMotionFromTextFile(const QString &fileName, double scale = 1.) -> AbstractRvtMotion*;

#endif // ABSTRACT_RVT_MOTION_H
