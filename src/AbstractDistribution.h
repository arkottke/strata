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

#ifndef ABSTRACT_DISTRIBUTION_H
#define ABSTRACT_DISTRIBUTION_H

#include <QObject>

#include <QJsonObject>
#include <QDataStream>
#include <QStringList>

class AbstractDistribution : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const AbstractDistribution* d);
    friend QDataStream & operator>> (QDataStream & in, AbstractDistribution* d);

public:
    explicit AbstractDistribution(QObject *parent = 0);

    enum Type {
        Uniform, //!< Uniform distribution
        Normal, //!< Normal distribution
        LogNormal //!< Log-Normal distribution
    };

    static QStringList typeList();

    Type type() const;
    void setType(Type type);

    //! Set the varied to the average
    void reset();

    double avg() const;
    double stdev() const;
    bool hasMin() const;
    double min() const;
    bool hasMax() const;
    double max() const;

    bool stdevRequired();

    //! Set the varied value after applying limits
    void setVaried(double varied);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

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
    Type m_type;

    //! Average of the distribution
    /*!
     * For uniform distributions this is not used.  For normal
     * distributions this is mean of the distribution.  For log-normal
     * distributions this is the median of the distribution in linear space
     * (or the mean in log space).
     */
    double m_avg;

    //! Varied value
    double m_varied;

    //! Standard deviation of the distribution
    /*!
     * For uniform distributions this is not used.  For normal
     * distributions this is the standard deviation in linear space, but
     * for log-normal distributions this is the standard deviation in log
     * space.
     */
    double m_stdev;

    //! If the distribution has a minimum
    bool m_hasMin;

    //! The minimum value of the distribution
    double m_min;

    //! If the distribution has a maximum
    bool m_hasMax;

    //! The maximum value of the distribution
    double m_max;
};
#endif // ABSTRACT_DISTRIBUTION_H
