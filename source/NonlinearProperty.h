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

#ifndef NONLINEAR_PROPERTY_H_
#define NONLINEAR_PROPERTY_H_

#include <QAbstractTableModel>
#include <QDataStream>
#include <QJsonObject>
#include <QList>
#include <QVector>

#include <gsl/gsl_interp.h>

class SoilType;

//! A class for the shear modulus reduction and damping curves

class NonlinearProperty : public QAbstractTableModel
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const NonlinearProperty* np) -> QDataStream &;
    friend auto operator>> (QDataStream & in, NonlinearProperty* np) -> QDataStream &;

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

    NonlinearProperty(QObject *parent = nullptr);
    NonlinearProperty(const QString &name, Type type,
                      const QVector<double> &strain, const QVector<double> &property,
                      QObject *parent = nullptr);
    ~NonlinearProperty();

    auto type() const -> Type;
    auto name() const -> const QString &;

    //! Linear interpolation of the prop for a given strain
    auto interp(const double strain) -> double;

    //! Create a html document containing the information of the model
    auto toHtml() const -> QString;

    auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int;
    auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant;
    auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant;

    auto strain() const -> const QVector<double> &;
    auto average() const -> const QVector<double> &;

    auto varied() const -> const QVector<double> &;
    void setVaried(const QVector<double> &varied);

    //! Return a duplicate of the NonlinearProperty
    auto duplicate() const -> NonlinearProperty *;

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

protected:    
    //! Initialize the interpolation routine. Called on the first interpolation
    void initialize();

    //! Name of the model
    QString _name;

    //! Type of curve
    Type _type;

    //! Strain of the property
    QVector<double> _strain;

    //! Log of the strain
    QVector<double> _lnStrain;

    //! Average value of the property
    QVector<double> _average;

    //! Varied value of the property
    QVector<double> _varied;

    //! Accelerator for the interpolation
    gsl_interp_accel *_acc;

    //! GSL Spline for the interpolation
    gsl_interp *_interp;
};
#endif // NONLINEAR_PROPERTY_H_
