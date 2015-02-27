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
// Copyright 2014 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#ifndef MY_RANDOM_NUM_GENERATOR_H_
#define MY_RANDOM_NUM_GENERATOR_H_

#include <QObject>
#include <QString>
#include <QStringList>

#include <gsl/gsl_rng.h>

#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;

class MyRandomNumGenerator : public QObject
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const MyRandomNumGenerator* myGenerator);
    friend QDataStream & operator>> (QDataStream & in, MyRandomNumGenerator* myGenerator);

public:
    MyRandomNumGenerator(QObject* parent = 0);
    ~MyRandomNumGenerator();
    
    bool seedSpecified() const;

    quint32 seed() const;
    void setSeed(quint32 seed);

    gsl_rng* gsl_pointer();

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

public slots:
    void setSeedSpecified(bool seedSpecified);
    void setSeed(int seed);

    void init();

signals:
    void seedSpecifiedChanged(int seedType);
    void seedChanged(int seed);
    void wasModified();

protected:
    bool m_seedSpecified;
    quint32 m_seed;

    gsl_rng* m_gsl_rng;
};
#endif
