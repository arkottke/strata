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

#ifndef MY_RANDOM_NUM_GENERATOR_H_
#define MY_RANDOM_NUM_GENERATOR_H_

#include <QDataStream>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QStringList>

#include <gsl/gsl_rng.h>

class MyRandomNumGenerator : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const MyRandomNumGenerator* myGenerator) -> QDataStream &;
    friend auto operator>> (QDataStream & in, MyRandomNumGenerator* myGenerator) -> QDataStream &;

public:
    MyRandomNumGenerator(QObject *parent = nullptr);
    ~MyRandomNumGenerator();
    
    auto seedSpecified() const -> bool;

    auto seed() const -> quint32;
    void setSeed(quint32 seed);

    auto gsl_pointer() -> gsl_rng*;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setSeedSpecified(bool seedSpecified);
    void setSeed(int seed);

    void init();

signals:
    void seedSpecifiedChanged(int seedType);
    void seedChanged(int seed);
    void wasModified();

protected:
    bool _seedSpecified;
    quint32 _seed;

    gsl_rng* _gsl_rng;
};
#endif
