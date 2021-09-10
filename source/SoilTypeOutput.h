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

#ifndef SOIL_TYPE_OUTPUT_H
#define SOIL_TYPE_OUTPUT_H

#include <QDataStream>
#include <QJsonObject>
#include <QObject>

class SoilType;
class NonlinearPropertyOutput;
class OutputCatalog;

class SoilTypeOutput : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const SoilTypeOutput* sto) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SoilTypeOutput* sto) -> QDataStream &;

public:
    explicit SoilTypeOutput(SoilType* soilType, OutputCatalog* catalog);

    auto soilType() const -> SoilType*;
    auto name() const -> QString;
    auto modulus() -> NonlinearPropertyOutput*;
    auto damping() -> NonlinearPropertyOutput*;

    auto enabled() const -> bool;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

signals:
    void wasModified();

public slots:
    void setEnabled(bool enabled);

protected:
    //! Associated soilType
    SoilType*  _soilType;

    //! If the is enabled
    bool _enabled;

    //! Output for the shear modulus reduction
    NonlinearPropertyOutput* _modulus;

    //! Output for the damping
    NonlinearPropertyOutput* _damping;
};

#endif // SOILTYPEOUTPUT_H
