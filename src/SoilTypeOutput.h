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

    friend QDataStream & operator<< (QDataStream & out, const SoilTypeOutput* sto);
    friend QDataStream & operator>> (QDataStream & in, SoilTypeOutput* sto);

public:
    explicit SoilTypeOutput(SoilType* soilType, OutputCatalog* catalog);

    SoilType* soilType() const;
    QString name() const;
    NonlinearPropertyOutput* modulus();
    NonlinearPropertyOutput* damping();

    bool enabled() const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:
    void wasModified();

public slots:
    void setEnabled(bool enabled);

protected:
    //! Associated soilType
    SoilType*  m_soilType;

    //! If the is enabled
    bool m_enabled;

    //! Output for the shear modulus reduction
    NonlinearPropertyOutput* m_modulus;

    //! Output for the damping
    NonlinearPropertyOutput* m_damping;
};

#endif // SOILTYPEOUTPUT_H
