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

#include <QAbstractTableModel>
#include <QDataStream>
#include <QJsonObject>
#include <QList>
#include <QVector>

#include "gsl/gsl_interp.h"

class SoilType;

//! A class for the shear modulus reduction and damping curves

class NonlinearProperty : public QAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const NonlinearProperty* np);
    friend QDataStream & operator>> (QDataStream & in, NonlinearProperty* np);

public:

    //! Type of model
    enum Type {
        ModulusReduction, //!< Shear-modulus reduction (G/G_max)
        Damping //!< Damping curve
    };

    enum Columns {
        StrainColumn,
        PropertyColumn
    };

    NonlinearProperty(QObject *parent = 0);
    NonlinearProperty(const QString &name, Type type,
                      const QVector<double> &strain, const QVector<double> &property,
                      QObject *parent = 0);
    ~NonlinearProperty();

    Type type() const;
    const QString & name() const;

    //! Linear interpolation of the prop for a given strain
    double interp(const double strain);

    //! Create a html document containing the information of the model
    QString toHtml() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    const QVector<double> & strain() const;
    const QVector<double> & average() const;

    const QVector<double> & varied() const;
    void setVaried(const QVector<double> &varied);

    //! Return a duplicate of the NonlinearProperty
    NonlinearProperty *duplicate() const;

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

protected:    
    //! Initialize the interpolation routine. Called on the first interpolation
    void initialize();

    //! Name of the model
    QString m_name;

    //! Type of curve
    Type m_type;

    //! Strain of the property
    QVector<double> m_strain;

    //! Average value of the property
    QVector<double> m_average;

    //! Varied value of the property
    QVector<double> m_varied;

    //! Accelerator for the interpolation
    gsl_interp_accel *m_acc;

    //! GSL Spline for the interpolation
    gsl_interp *m_interp;
};
#endif // NONLINEAR_PROPERTY_H_
