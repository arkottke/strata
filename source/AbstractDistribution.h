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

#ifndef ABSTRACT_DISTRIBUTION_H
#define ABSTRACT_DISTRIBUTION_H

#include <QObject>

#include <QJsonObject>
#include <QDataStream>
#include <QStringList>

class AbstractDistribution : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const AbstractDistribution* d) -> QDataStream &;
    friend auto operator>> (QDataStream & in, AbstractDistribution* d) -> QDataStream &;

public:
    explicit AbstractDistribution(QObject *parent = nullptr);

    enum Type {
        Uniform, //!< Uniform distribution
        Normal, //!< Normal distribution
        LogNormal //!< Log-Normal distribution
    };

    static auto typeList() -> QStringList;

    auto type() const -> Type;
    void setType(Type type);

    //! Set the varied to the average
    void reset();

    auto avg() const -> double;
    auto stdev() const -> double;
    auto hasMin() const -> bool;
    auto min() const -> double;
    auto hasMax() const -> bool;
    auto max() const -> double;

    auto stdevRequired() -> bool;

    //! Set the varied value after applying limits
    void setVaried(double varied);

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setType(int type);
    void setAvg(double avg);
    void setStdev(double stdev);
    void setHasMin(bool hasMin);
    void setMin(double min);
    void setHasMax(bool hasMax);
    void setMax(double max);

signals:
    void typeChanged(int type);
    void avgChanged(double avg);
    void stdevChanged(double stdev);
    void hasMinChanged(bool hasMin);
    void minChanged(double min);
    void hasMaxChanged(bool hasMax);
    void maxChanged(double max);

    bool stdevRequiredChanged(bool stdevRequired);

    void wasModified();

    void requiresLimits(bool b);
protected:

    //! Type of distribution
    Type _type;

    //! Average of the distribution
    /*!
     * For uniform distributions this is not used.  For normal
     * distributions this is mean of the distribution.  For log-normal
     * distributions this is the median of the distribution in linear space
     * (or the mean in log space).
     */
    double _avg;

    //! Varied value
    double _varied;

    //! Standard deviation of the distribution
    /*!
     * For uniform distributions this is not used.  For normal
     * distributions this is the standard deviation in linear space, but
     * for log-normal distributions this is the standard deviation in log
     * space.
     */
    double _stdev;

    //! If the distribution has a minimum
    bool _hasMin;

    //! The minimum value of the distribution
    double _min;

    //! If the distribution has a maximum
    bool _hasMax;

    //! The maximum value of the distribution
    double _max;
};
#endif // ABSTRACT_DISTRIBUTION_H
