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

#ifndef NON_LINEAR_PROPERTY_VARIATION_H_
#define NON_LINEAR_PROPERTY_VARIATION_H_

#include "SoilType.h"
#include "RockLayer.h"

#include <gsl/gsl_rng.h>
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QScriptEngine>

class NonLinearPropertyVariation : public QObject
{
    Q_OBJECT

    public:
        NonLinearPropertyVariation(QObject * parent = 0 );

        //! Model for the standard deviation
        enum Model {
            Darendeli, //!< Defined by Stokoe and Darendeli
            Custom //!< User defined standard deviation
        };

        static QStringList modelList();
        
        //! Reset the object to the default values
        void reset();

        bool enabled() const;
        void setEnabled(bool enabled);

        Model model() const;
        void setModel(Model model);

        bool bedrockIsEnabled() const;
        void setBedrockIsEnabled(bool enabled);

        double correl() const;
        void setCorrel(double correl);

        QString shearModStdev() const;
        void setShearModStdev(QString shearModStdev);

        double shearModMin() const;
        void setShearModMin(double min);

        double shearModMax() const;
        void setShearModMax(double max);
        
        QString dampingStdev() const;
        void setDampingStdev(QString dampingStdev);

        double dampingMin() const;
        void setDampingMin(double min);
        
        void setRandomNumberGenerator(gsl_rng * rng);

        QMap<QString, QVariant> toMap() const;
        void fromMap( const QMap<QString, QVariant> & map );

        //! Vary the properties of a given layer
        void vary(SoilType & soilType);

        //! Vary the damping of the bedrock
        void vary(RockLayer & bedrock);

        //! Check if the QString is a valid function
        bool isValid( const QString & function ) const;

    private:
        //! If the variation is enabled
        bool m_enabled;

        //! Model for the standard deviation
        Model m_model;
        
        //! If the damping of the bedrock is enabled
        bool m_bedrockIsEnabled;

        //! Correlation between shear modulus and damping
        double m_correl;

        //! A function that describes the standard deviation of the normalized shear modulus
        QString m_shearModStdev;

        //! Minimum normalized shear modulus
        double m_shearModMin;
        
        //! Maximum normalized shear modulus
        double m_shearModMax;

        //! A function that describes the standard deviation of the damping
        QString m_dampingStdev;
        
        //! Minimum damping value (percent)
        double m_dampingMin;
    
        //! Script engine used to evaluate the custom standard deviation definitions
        QScriptEngine m_engine;
       
        //! Random number generator
        gsl_rng * m_rng;

        //! Compute the standard deviation of the damping
        double computeDampingStdev( double damping, double strain = 0.0001 );
};
#endif
