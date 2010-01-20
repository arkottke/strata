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

#ifndef NONLINEAR_PROPERTY_H_
#define NONLINEAR_PROPERTY_H_

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

//! A class for the shear modulus reduction and damping curves


class SoilType;

class NonlinearProperty : public QObject
{
    Q_OBJECT 

    public:
        //! Type of model
        enum Type {
            Undefined, //!< No type of model defined
            ModulusReduction, //!< Shear-modulus reduction (G/G_max)
            Damping, //!< Damping curve
        };

        //! Source of model
        enum Source {
            Temporary, //!< Used for this analysis only
            HardCoded, //!< Property is hardcoded into the program and does not need to be saved
            UserDefined, //!< Property is defined by the user and needs to be saved
            Computed, //!< Model needs to be computed by the soil type
        };
        
        NonlinearProperty(  QString name = "", Type type = Undefined, Source source = Temporary, QObject * parent = 0);
        NonlinearProperty( const QMap<QString, QVariant> & map );


        ~NonlinearProperty();
        Type type() const;
        void setType(Type type);

        QString typeLabel() const;

        const QString & name() const;
        void setName( const QString & name);

        Source source() const;        
        void setSource(Source source);
 
        //! Reset the curve to the average value
        void reset();

        //! Copy the vales from another NonlinearProperty
        void copyValues( NonlinearProperty * other );

        //! Linear interpolation of the prop for a given strain
        double interp(const double strain) const;

        //! Create a map from the object
        QMap<QString, QVariant> toMap() const;

        //! Load the object from a map
        void fromMap(const QMap<QString, QVariant> & map);
        
        //! Create a html document containing the information of the model
        QString toHtml() const;
        
        QList<double> & strain();
        QList<double> & avg();
        QList<double> & prop();

    private:
        //! Name of the model
        QString m_name;

        //! Type of curve
        Type m_type;

        //! Save if the object was defined by the user
        Source m_source;

        //! Strain of the property
        QList<double> m_strain;

        //! Average value of the property
        QList<double> m_avg;

        //! Varied value of the property
        QList<double> m_prop;
};
#endif
