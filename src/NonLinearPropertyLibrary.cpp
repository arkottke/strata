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

#include "NonLinearPropertyLibrary.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

NonLinearPropertyLibrary::NonLinearPropertyLibrary()
{
    m_fileName = "nonLinearCurves.strd";
    // Create the hard coded models
    createModels();

    // Load the user defined models
    load();
}

NonLinearPropertyLibrary::~NonLinearPropertyLibrary()
{
    // Save the user define models
    save();
}

QStringList NonLinearPropertyLibrary::modulusList() const
{
    QStringList list;

    for (int i = 0; i < m_modulus.size(); ++i)
        list << m_modulus.at(i).name();

    return list;
}

QStringList NonLinearPropertyLibrary::dampingList() const
{
    QStringList list;

    for (int i = 0; i < m_damping.size(); ++i)
        list << m_damping.at(i).name();

    return list;
}

QList<NonLinearProperty> & NonLinearPropertyLibrary::modulus()
{
    return m_modulus;
}

QList<NonLinearProperty> & NonLinearPropertyLibrary::damping()
{
    return m_damping;
}
        
void NonLinearPropertyLibrary::createModels()
{
    // Shear modulus reduction models
    m_modulus << NonLinearProperty( "Custom",
            NonLinearProperty::ModulusReduction, NonLinearProperty::Temporary);

    m_modulus << NonLinearProperty("Darendeli & Stokoe",
            NonLinearProperty::ModulusReduction, NonLinearProperty::Computed);

    m_modulus << NonLinearProperty("Seed & Idriss - Mean",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 0.99 << 0.96 << 0.88 << 0.74 << 0.52 <<
        0.29 << 0.15 << 0.06;

    m_modulus << NonLinearProperty("Seed & Idriss - Upper",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 0.99 << 0.94 << 0.84 << 0.65 <<
        0.36 << 0.19 << 0.08;

    m_modulus << NonLinearProperty("Iwasaki -- 0.25 atm",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 0.98 << 0.93 << 0.85 << 0.72 << 0.49 <<
        0.23 << 0.08 << 0.03;

    m_modulus << NonLinearProperty("Iwasaki -- 1.0 atm",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 0.97 << 0.92 << 0.83 << 0.65 <<
        0.39 << 0.17 << 0.07;

    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 0",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 0.96 << 0.88 << 0.70 << 0.47 <<
        0.26 << 0.11 << 0.03;
    
    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 15",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 0.99 << 0.94 << 0.81 << 0.64 <<
        0.41 << 0.22 << 0.10;

    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 30",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 1.00 << 0.98 << 0.90 << 0.75 <<
        0.53 << 0.35 << 0.17;

    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 50",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 <<1.00 << 1.00 << 1.00 << 0.95 << 0.84 <<
        0.67 << 0.47 << 0.25;

    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 100",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 1.00 << 1.00 << 0.98 << 0.92 <<
        0.81 << 0.63 << 0.37;

    m_modulus << NonLinearProperty("Vucetic & Dobry -- PI = 200",
            NonLinearProperty::ModulusReduction, NonLinearProperty::HardCoded);
    m_modulus.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last().avg() << 1.00 << 1.00 << 1.00 << 1.00 << 1.00 << 0.96 <<
        0.89 << 0.75 << 0.53;

    // Damping models
    m_damping << NonLinearProperty("Custom", NonLinearProperty::Damping,
            NonLinearProperty::Temporary);

    m_damping << NonLinearProperty("Darendeli & Stokoe",
            NonLinearProperty::Damping, NonLinearProperty::Computed);

    m_damping << NonLinearProperty("Seed & Idriss - Lower",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 0.50 << 0.60 << 0.80 << 1.40 << 2.80 << 5.30 << 10.00 << 15.80 << 21.50;

    m_damping << NonLinearProperty("Seed & Idriss - Mean",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
     m_damping.last().avg() << 0.57 << 0.86 << 1.70 << 3.10 << 5.50 << 9.50 << 15.50 << 21.10 << 24.60;

    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 0",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 1.00 << 1.00 << 1.00 << 3.00 << 5.40 << 9.80 << 15.00 << 20.30 << 24.00;
    
    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 15",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
     m_damping.last().avg() << 1.00 << 1.00 << 1.00 << 2.60 << 4.50 << 7.50 << 11.60 << 16.00 << 20.00;

    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 30",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 1.00 << 1.00 << 1.00 << 2.10 << 3.80 << 5.90 << 8.80 << 12.50 << 16.90; 

    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 50",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 1.00 << 1.00 << 1.00 << 1.80 << 2.90 << 4.30 << 6.20 << 9.50 << 13.50;

    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 100",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 1.00 << 1.00 << 1.00 << 1.50 << 2.00 << 2.90 << 4.10 << 6.20 << 9.80;

    m_damping << NonLinearProperty("Vucetic & Dobry -- PI = 200",
            NonLinearProperty::Damping, NonLinearProperty::HardCoded);
    m_damping.last().strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last().avg() << 1.00 << 1.00 << 1.10 << 1.30 << 1.60 << 2.10 << 3.00 << 4.80 << 8.10;
}

void NonLinearPropertyLibrary::load()
{
    QFile file(m_fileName);
    // Try to open the file
    if (!file.open(QIODevice::ReadOnly))
        return;
    
    // Open the data stream
    QDataStream inStream(&file);
    // Read the map from the file 
    QMap<QString,QVariant> map;
    inStream >> map;
    // Load the model from the map
    QList<QVariant> list = map.values("shearMod");
    for (int i = 0; i < list.size(); ++i) {
        m_modulus << NonLinearProperty();
        m_modulus.last().fromMap(list.at(i).toMap());
    }
    
    list = map.values("damping");
    for (int i = 0; i < list.size(); ++i) {
        m_damping << NonLinearProperty();
        m_damping.last().fromMap(list.at(i).toMap());
    }
}

void NonLinearPropertyLibrary::save()
{
    // Count the number of user defined models.  If not models are user defined
    // then there is no need to save.
    int count = 0;

    for (int i = 0; i < m_modulus.size(); ++i)
        if ( m_modulus.at(i).source() == NonLinearProperty::UserDefined )
            ++ count;

    for (int i = 0; i < m_damping.size(); ++i)
        if ( m_damping.at(i).source() == NonLinearProperty::UserDefined )
            ++ count;

    QFile file(m_fileName);
    // If the file can't be opened return
    if (!file.open(QIODevice::WriteOnly))
        return;
    
    if ( count == 0 ) {
        // If there are userdefined models delete the file
        file.remove();
        return;
    }
    
    // Create a map of the user defined models
    QMap<QString,QVariant> map;
    
    for (int i = 0; i < m_modulus.size(); ++i){
        if ( m_modulus.at(i).source() == NonLinearProperty::UserDefined ){
            map.insertMulti( "shearMod", m_modulus.at(i).toMap());
        }
    }

    for (int i = 0; i < m_damping.size(); ++i)
        if ( m_damping.at(i).source() == NonLinearProperty::UserDefined )
            map.insertMulti( "damping", m_damping.at(i).toMap());

    // Open the data stream
    QDataStream outStream(&file);
    // Create a map of the model and save the map
    outStream << map;
}

