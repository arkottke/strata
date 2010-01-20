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

#include "NonlinearPropertyLibrary.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

NonlinearPropertyLibrary::NonlinearPropertyLibrary()
{
    m_fileName = "nonLinearCurves.strd";
    // Create the hard coded models
    createModels();

    // Load the user defined models
    load();
}

NonlinearPropertyLibrary::~NonlinearPropertyLibrary()
{
    // Save the user define models
    save();
}

QStringList NonlinearPropertyLibrary::modulusList() const
{
    QStringList list;

    for (int i = 0; i < m_modulus.size(); ++i)
        list << m_modulus.at(i)->name();

    return list;
}

QStringList NonlinearPropertyLibrary::dampingList() const
{
    QStringList list;

    for (int i = 0; i < m_damping.size(); ++i)
        list << m_damping.at(i)->name();

    return list;
}

QList<NonlinearProperty*> & NonlinearPropertyLibrary::modulus()
{
    return m_modulus;
}

QList<NonlinearProperty*> & NonlinearPropertyLibrary::damping()
{
    return m_damping;
}
        
void NonlinearPropertyLibrary::createModels()
{
    // Shear modulus reduction models
    m_modulus << new NonlinearProperty( "Custom",
            NonlinearProperty::ModulusReduction, NonlinearProperty::Temporary);

    m_modulus << new NonlinearProperty("Darendeli & Stokoe",
            NonlinearProperty::ModulusReduction, NonlinearProperty::Computed);

    m_modulus << new NonlinearProperty( "EPRI (93), PI=10", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        0.998 << 0.993 << 0.977 << 0.951 << 0.905 << 0.839 << 0.746 << 0.626 <<
        0.495 << 0.364 << 0.244 << 0.158 << 0.092 << 0.052 << 0.032;

    m_modulus << new NonlinearProperty( "EPRI (93), PI=30", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 1.000 << 0.997 << 0.990 << 0.968 << 0.931 << 0.872 << 0.784 <<
        0.671 << 0.543 << 0.409 << 0.288 << 0.188 << 0.117 << 0.074;

    m_modulus << new NonlinearProperty( "EPRI (93), PI=50", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 1.000 << 1.000 << 1.000 << 0.996 << 0.981 << 0.951 << 0.897 <<
        0.821 << 0.714 << 0.585 << 0.451 << 0.330 << 0.224 << 0.153;

    m_modulus << new NonlinearProperty( "EPRI (93), PI=70", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 1.000 << 1.000 << 1.000 << 1.000 << 1.000 << 0.996 << 0.976 <<
        0.937 << 0.877 << 0.785 << 0.671 << 0.536 << 0.408 << 0.299;

    m_modulus << new NonlinearProperty( "EPRI (93), 0-20 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 0.998 << 0.986 << 0.960 << 0.920 << 0.851 << 0.759 << 0.642 <<
        0.515 << 0.392 << 0.278 << 0.189 << 0.121 << 0.073 << 0.043;

    m_modulus << new NonlinearProperty( "EPRI (93), 20-50 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 0.999 << 0.993 << 0.979 << 0.952 << 0.903 << 0.830 << 0.733 <<
        0.617 << 0.488 << 0.366 << 0.258 << 0.173 << 0.108 << 0.066;

    m_modulus << new NonlinearProperty( "EPRI (93), 50-120 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 0.999 << 0.998 << 0.988 << 0.970 << 0.934 << 0.878 << 0.797 <<
        0.691 << 0.567 << 0.444 << 0.328 << 0.225 << 0.145 << 0.092;

    m_modulus << new NonlinearProperty( "EPRI (93), 120-250 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00;
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 0.999 << 0.999 << 0.994 << 0.979 << 0.954 << 0.909 << 0.840 <<
        0.747 << 0.631 << 0.507 << 0.384 << 0.274 << 0.182 << 0.118;

    m_modulus << new NonlinearProperty( "EPRI (93), 250-500 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00; 
    m_modulus.last()->avg() << 1.000 << 1.000 <<
        1.000 << 0.999 << 0.999 << 0.999 << 0.986 << 0.968 << 0.934 << 0.879 <<
        0.799 << 0.696 << 0.575 << 0.449 << 0.332 << 0.230 << 0.148;

    m_modulus << new NonlinearProperty( "EPRI (93), 500-1000 ft", NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 << 5.623e-04
        << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 << 1.000e-02 <<
        1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 << 1.778e-01 << 3.162e-01
        << 5.623e-01 << 1.000e+00; 
    m_modulus.last()->avg() << 1.000 << 1.000 << 1.000 << 1.000 << 1.000 <<
        1.000 << 0.995 << 0.982 << 0.960 << 0.915 << 0.850 << 0.765 << 0.660 <<
        0.532 << 0.410 << 0.297 << 0.200;

    m_modulus << new NonlinearProperty( "GEI (83), 0-50 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.990 << 0.980 << 0.910 << 0.780 <<
        0.560 << 0.330 << 0.160 << 0.065 << 0.040 << 0.040;

    m_modulus << new NonlinearProperty( "GEI (83), 50-100 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.990 << 0.980 << 0.935 << 0.816 <<
        0.610 << 0.360 << 0.175 << 0.065 << 0.040 << 0.040;

    m_modulus << new NonlinearProperty( "GEI (83), 100-250 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.990 << 0.980 << 0.947 << 0.852 <<
        0.670 << 0.425 << 0.200 << 0.065 << 0.040 << 0.040;

    m_modulus << new NonlinearProperty( "GEI (83), 250-500 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.995 << 0.990 << 0.965 << 0.890 <<
        0.725 << 0.495 << 0.240 << 0.080 << 0.040 << 0.040;

    m_modulus << new NonlinearProperty( "GEI (83), >500 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 0.985 << 0.925 <<
        0.775 << 0.565 << 0.300 << 0.100 << 0.040 << 0.040;

    m_modulus << new NonlinearProperty( "GeoMatrix (1990), 0-50 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.991 << 0.985 << 0.943 << 0.854 <<
        0.688 << 0.454 << 0.259 << 0.100 << 0.052 << 0.052;

    m_modulus << new NonlinearProperty( "GeoMatrix (1990), 50-150 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 0.998 << 0.999 << 0.975 << 0.900 <<
        0.767 << 0.536 << 0.319 << 0.134 << 0.052 << 0.052;

    m_modulus << new NonlinearProperty( "GeoMatrix (1990), >150 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 0.999 << 0.939 <<
        0.826 << 0.620 << 0.381 << 0.162 << 0.052 << 0.052;

    m_modulus << new NonlinearProperty( "Idriss (1990), Clay", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 0.979 << 0.941 <<
        0.839 << 0.656 << 0.429 << 0.238 << 0.238 << 0.238;

    m_modulus << new NonlinearProperty( "Idriss (1990), Sand", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 0.990 << 0.955 << 0.850 <<
        0.628 << 0.370 << 0.176 << 0.080 << 0.080 << 0.080;

    m_modulus << new NonlinearProperty( "Imperial Valley Soils, 0-300 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 1.000 << 0.973 <<
        0.904 << 0.699 << 0.459 << 0.247;

    m_modulus << new NonlinearProperty( "Imperial Valley Soils, >300 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 1.000 << 0.986 <<
        0.952 << 0.767 << 0.541 << 0.329;

    m_modulus << new NonlinearProperty("Iwasaki, 0.25 atm",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 0.98 << 0.93 << 0.85 << 0.72 << 0.49 <<
        0.23 << 0.08 << 0.03;

    m_modulus << new NonlinearProperty("Iwasaki, 1.0 atm",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 0.97 << 0.92 << 0.83 << 0.65 <<
        0.39 << 0.17 << 0.07;
    
    m_modulus << new NonlinearProperty( "Peninsular Range, Cohesionless 0-50 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 0.963 << 0.868 <<
        0.677 << 0.427 << 0.221 << 0.088;

    m_modulus << new NonlinearProperty( "Peninsular Range, Cohesionless 50-500 ft", 
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded );
    m_modulus.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_modulus.last()->avg()  << 1.000 << 1.000 << 1.000 << 0.985 << 0.941 <<
        0.846 << 0.647 << 0.404 << 0.191;
    
    m_modulus << new NonlinearProperty("Seed & Idriss, Sand Mean",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 0.99 << 0.96 << 0.88 << 0.74 << 0.52 <<
        0.29 << 0.15 << 0.06;

    m_modulus << new NonlinearProperty("Seed & Idriss, Sand Upper",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 0.99 << 0.94 << 0.84 << 0.65 <<
        0.36 << 0.19 << 0.08;

    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 0",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 0.96 << 0.88 << 0.70 << 0.47 <<
        0.26 << 0.11 << 0.03;
    
    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 15",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 0.99 << 0.94 << 0.81 << 0.64 <<
        0.41 << 0.22 << 0.10;

    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 30",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 1.00 << 0.98 << 0.90 << 0.75 <<
        0.53 << 0.35 << 0.17;

    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 50",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 <<1.00 << 1.00 << 1.00 << 0.95 << 0.84 <<
        0.67 << 0.47 << 0.25;

    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 100",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 1.00 << 1.00 << 0.98 << 0.92 <<
        0.81 << 0.63 << 0.37;

    m_modulus << new NonlinearProperty("Vucetic & Dobry, PI = 200",
            NonlinearProperty::ModulusReduction, NonlinearProperty::HardCoded);
    m_modulus.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_modulus.last()->avg() << 1.00 << 1.00 << 1.00 << 1.00 << 1.00 << 0.96 <<
        0.89 << 0.75 << 0.53;

    // Damping models
    m_damping << new NonlinearProperty("Custom", NonlinearProperty::Damping,
            NonlinearProperty::Temporary);

    m_damping << new NonlinearProperty("Darendeli & Stokoe",
            NonlinearProperty::Damping, NonlinearProperty::Computed);

    m_damping << new NonlinearProperty( "EPRI (93), PI=10", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.000 << 0.991 << 1.054 << 1.122 << 1.337 <<
        1.639 << 2.040 << 2.890 << 3.981 << 5.585 << 7.733 << 10.388 << 13.339
        << 16.412 << 19.099 << 20.336 << 19.936;

    m_damping << new NonlinearProperty( "EPRI (93), PI=30", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.428 << 1.420 << 1.423 << 1.513 << 1.600 <<
        1.743 << 2.026 << 2.480 << 3.095 << 4.113 << 5.643 << 7.744 << 10.295
        << 13.199 << 16.085 << 18.394 << 18.956;

    m_damping << new NonlinearProperty( "EPRI (93), PI=50", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.928 << 1.920 << 1.911 << 1.973 << 1.965 <<
        2.037 << 2.142 << 2.348 << 2.773 << 3.346 << 4.330 << 5.778 << 7.759 <<
        10.214 << 12.931 << 15.617 << 17.429;

    m_damping << new NonlinearProperty( "EPRI (93), PI=70", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 2.500 << 2.491 << 2.411 << 2.402 << 2.393 <<
        2.411 << 2.446 << 2.545 << 2.662 << 2.937 << 3.365 << 4.119 << 5.279 <<
        6.966 << 9.111 << 11.421 << 13.704;

    m_damping << new NonlinearProperty( "EPRI (93), 0-20 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.429 << 1.476 << 1.536 << 1.602 << 1.879 <<
        2.193 << 2.819 << 3.724 << 5.066 << 6.940 << 9.346 << 12.201 << 15.443
        << 18.703 << 21.704 << 24.541 << 27.217;

    m_damping << new NonlinearProperty( "EPRI (93), 20-50 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.142 << 1.196 << 1.250 << 1.304 << 1.469 <<
        1.670 << 2.074 << 2.658 << 3.622 << 5.066 << 7.069 << 9.569 << 12.500
        << 15.688 << 19.004 << 22.022 << 24.699;

    m_damping << new NonlinearProperty( "EPRI (93), 50-120 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 1.000 << 0.982 << 0.978 << 1.122 << 1.165 <<
        1.371 << 1.610 << 2.046 << 2.830 << 3.901 << 5.493 << 7.662 << 10.381
        << 13.383 << 16.643 << 19.766 << 22.580;

    m_damping << new NonlinearProperty( "EPRI (93), 120-250 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 0.857 << 0.839 << 0.821 << 0.880 << 1.006 <<
        1.125 << 1.341 << 1.629 << 2.198 << 3.131 << 4.460 << 6.281 << 8.686 <<
        11.573 << 14.770 << 17.879 << 20.906;

    m_damping << new NonlinearProperty( "EPRI (93), 250-500 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 0.786 << 0.792 << 0.799 << 0.811 << 0.867 <<
        0.950 << 1.081 << 1.344 << 1.774 << 2.515 << 3.520 << 5.077 << 7.206 <<
        9.853 << 12.839 << 15.972 << 19.072;

    m_damping << new NonlinearProperty( "EPRI (93), 500-1000 ft", NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain() << 1.000e-04 << 1.778e-04 << 3.162e-04 <<
        5.623e-04 << 1.000e-03 << 1.778e-03 << 3.162e-03 << 5.623e-03 <<
        1.000e-02 << 1.778e-02 << 3.162e-02 << 5.623e-02 << 1.000e-01 <<
        1.778e-01 << 3.162e-01 << 5.623e-01 << 1.000e+00;
    m_damping.last()->avg() << 0.492 << 0.554 << 0.607 << 0.589 << 0.643 <<
        0.696 << 0.772 << 0.983 << 1.234 << 1.735 << 2.562 << 3.696 << 5.408 <<
        7.617 << 10.428 << 13.524 << 16.459;
    
    m_damping << new NonlinearProperty( "GEI (83), 0-50 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 1.500 << 1.500 << 1.500 << 1.750 << 3.850 <<
        7.750 << 13.100 << 18.750 << 23.000 << 26.000 << 26.000;

    m_damping << new NonlinearProperty( "GEI (83), 50-100 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 1.500 << 1.500 << 1.500 << 1.600 << 3.150 <<
        6.500 << 11.750 << 17.750 << 22.500 << 25.600 << 25.600;

    m_damping << new NonlinearProperty( "GEI (83), 100-250 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 1.500 << 1.500 << 1.500 << 1.500 << 2.500 <<
        5.000 << 10.000 << 16.500 << 22.000 << 25.400 << 25.400;

    m_damping << new NonlinearProperty( "GEI (83), 250-500 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 1.500 << 1.500 << 1.500 << 1.500 << 2.100 <<
        3.750 << 8.250 << 14.750 << 20.500 << 24.250 << 24.250;
    
    m_damping << new NonlinearProperty( "GEI (83), >500 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 1.500 << 1.500 << 1.500 << 1.500 << 1.750 <<
        2.500 << 6.250 << 13.000 << 19.000 << 23.100 << 23.100;

    
    m_damping << new NonlinearProperty( "GeoMatrix (1990), 0-50 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.210 << 0.210 << 2.050 << 2.500 << 3.730 <<
        6.560 << 11.230 << 15.980 << 21.090 << 24.110 << 24.110;

    m_damping << new NonlinearProperty( "GeoMatrix (1990), 50-150 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.150 << 0.150 << 1.520 << 1.950 << 2.950 <<
        4.730 << 8.570 << 13.070 << 18.590 << 24.110 << 24.110;

    m_damping << new NonlinearProperty( "GeoMatrix (1990), 150-300 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.150 << 0.150 << 1.520 << 1.680 << 2.250 <<
        3.220 << 6.190 << 10.270 << 15.660 << 24.110 << 24.110;

    m_damping << new NonlinearProperty( "GeoMatrix (1990), >300 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.960 << 0.980 << 1.010 << 1.010 << 1.070 <<
        1.560 << 4.150 << 7.840 << 12.080 << 17.840 << 17.840;
    
    m_damping << new NonlinearProperty( "Idriss (1990), Sand", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.240 << 0.440 << 0.800 << 1.460 << 2.800 <<
        5.310 << 9.800 << 15.740 << 21.000 << 21.000 << 21.000;

    m_damping << new NonlinearProperty( "Idriss (1990), Clay", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00 << 3.162e+00 << 1.000e+01;
    m_damping.last()->avg()  << 0.240 << 0.440 << 0.800 << 1.460 << 2.800 <<
        5.310 << 9.800 << 15.740 << 21.000 << 21.000 << 21.000;
    
    m_damping << new NonlinearProperty( "Imperial Valley Soils, 0-300 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_damping.last()->avg()  << 0.950 << 0.950 << 0.950 << 0.950 << 1.220 <<
        2.030 << 4.060 << 7.980 << 12.040;

    m_damping << new NonlinearProperty( "Imperial Valley Soils, >300 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_damping.last()->avg()  << 0.540 << 0.540 << 0.540 << 0.540 << 0.540 <<
        0.950 << 2.710 << 5.950 << 9.870;

    m_damping << new NonlinearProperty( "Peninsular Range, Cohesionless 0-50 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_damping.last()->avg()  << 1.060 << 1.060 << 1.210 << 1.660 << 2.880 <<
        5.600 << 10.590 << 16.650 << 23.000;

    m_damping << new NonlinearProperty( "Peninsular Range, Cohesionless 50-500 ft", 
            NonlinearProperty::Damping, NonlinearProperty::HardCoded );
    m_damping.last()->strain()  << 1.000e-04 << 3.162e-04 << 1.000e-03 <<
        3.162e-03 << 1.000e-02 << 3.162e-02 << 1.000e-01 << 3.162e-01 <<
        1.000e+00;
    m_damping.last()->avg()  << 0.600 << 0.600 << 0.600 << 0.910 << 1.210 <<
        2.580 << 5.450 << 10.590 << 16.650;

    m_damping << new NonlinearProperty("Seed & Idriss, Sand Lower",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 0.50 << 0.60 << 0.80 << 1.40 << 2.80 << 5.30 <<
        10.00 << 15.80 << 21.50;

    m_damping << new NonlinearProperty("Seed & Idriss, Sand Mean",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 0.57 << 0.86 << 1.70 << 3.10 << 5.50 << 9.50 <<
         15.50 << 21.10 << 24.60;

    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 0",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.00 << 3.00 << 5.40 << 9.80 << 15.00 << 20.30 << 24.00;
    
    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 15",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.00 << 2.60 << 4.50 << 7.50 <<
         11.60 << 16.00 << 20.00;

    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 30",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.00 << 2.10 << 3.80 << 5.90 << 8.80 << 12.50 << 16.90; 

    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 50",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.00 << 1.80 << 2.90 << 4.30 << 6.20 << 9.50 << 13.50;

    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 100",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.00 << 1.50 << 2.00 << 2.90 << 4.10 << 6.20 << 9.80;

    m_damping << new NonlinearProperty("Vucetic & Dobry, PI = 200",
            NonlinearProperty::Damping, NonlinearProperty::HardCoded);
    m_damping.last()->strain() << 1.00e-04 << 3.16e-04 << 1.00e-03 << 3.16e-03
        << 1.00e-02 << 3.16e-02 << 1.00e-01 << 3.16e-01 << 1.00e+00;
    m_damping.last()->avg() << 1.00 << 1.00 << 1.10 << 1.30 << 1.60 << 2.10 << 3.00 << 4.80 << 8.10;
}

void NonlinearPropertyLibrary::load()
{
    QFile file(m_fileName);
    // Try to open the file
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "file not found!";
        return;
    }
    
    // Open the data stream
    QDataStream inStream(&file);
    // Read the map from the file 
    QMap<QString,QVariant> map;
    inStream >> map;
    // Load the model from the map
    QList<QVariant> list = map.values("shearMod");
    for (int i = 0; i < list.size(); ++i) {
        m_modulus << new NonlinearProperty(list.at(i).toMap());
    }
    
    list = map.values("damping");
    for (int i = 0; i < list.size(); ++i) {
        m_damping << new NonlinearProperty(list.at(i).toMap());
    }
}

void NonlinearPropertyLibrary::save()
{
    // Count the number of user defined models.  If not models are user defined
    // then there is no need to save.
    int count = 0;

    for (int i = 0; i < m_modulus.size(); ++i)
        if ( m_modulus.at(i)->source() == NonlinearProperty::UserDefined )
            ++ count;

    for (int i = 0; i < m_damping.size(); ++i)
        if ( m_damping.at(i)->source() == NonlinearProperty::UserDefined )
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
        if ( m_modulus.at(i)->source() == NonlinearProperty::UserDefined ){
            map.insertMulti( "shearMod", m_modulus.at(i)->toMap());
        }
    }

    for (int i = 0; i < m_damping.size(); ++i)
        if ( m_damping.at(i)->source() == NonlinearProperty::UserDefined )
            map.insertMulti( "damping", m_damping.at(i)->toMap());

    // Open the data stream
    QDataStream outStream(&file);
    // Create a map of the model and save the map
    outStream << map;
}

