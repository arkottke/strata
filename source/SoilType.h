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

#ifndef SOIL_TYPE_H_
#define SOIL_TYPE_H_

#include "NonlinearProperty.h"

#include <QAbstractTableModel>
#include <QDataStream>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class SoilType : public QObject
{
    Q_OBJECT

    friend auto operator<< (QDataStream & out, const SoilType* soilType) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SoilType* soilType) -> QDataStream &;

public:
    explicit SoilType(QObject *parent = nullptr);

    auto untWt() const -> double;
    void setUntWt(double untWt);

    auto density() const -> double;

    auto damping() const -> double;
    void setDamping(double damping);

    auto minDamping() const -> double;
    void setMinDamping(double minDamping);

    void setName(const QString & name);
    auto name() const -> const QString &;

    void setNotes(const QString & notes);
    auto notes() const -> const QString &;

    void setIsVaried(bool isVaried);
    auto isVaried() const -> bool;

    void setSaveData(bool saveData);
    auto saveData() const -> bool;

    auto modulusModel() -> NonlinearProperty*;
    void setModulusModel(NonlinearProperty * model);

    auto dampingModel() -> NonlinearProperty*;
    void setDampingModel(NonlinearProperty * model);

    //! If the soil properties are required
    auto requiresSoilProperties() const -> bool;

    auto meanStress() const -> double;
    auto pi() const -> double;
    auto ocr() const -> double;
    auto freq() const -> double;
    auto nCycles() const -> int;

    //! Create a html document containing the information of the model
    auto toHtml() const -> QString;

    //! Compute the shear modulus-reduction and damping curves
    void computeDarendeliCurves();

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

public slots:
    void setPi(double pi);
    void setMeanStress(double meanStress);
    void setOcr(double ocr);
    void setFreq(double freq);
    void setNCycles(int nCycles);
    
signals:
    void nameChanged(const QString &name);
    void dampingModelChanged(NonlinearProperty* damping);
    void modulusModelChanged(NonlinearProperty* modulusReduction);

    void wasModified();

private:
    //! Unit weight of the layer
    double _untWt;

    //! Damping of the layer
    double _damping;

    //! Minimum damping of the layer
    double _minDamping;

    //! Name of the soil layer
    QString _name;

    //! Notes regarding the soil type
    QString _notes;

    //! Switch which controls if the soil layer is varied
    bool _isVaried;

    //! Save the nonlinear curve data during the trials
    bool _saveData;

    /*
     * Soil properties used for the Darendeli empirical relationship
     */
    //@{
    //! Mean stress in atmospheres
    double _meanStress;

    //! Plasticity Index
    double _pi;

    //! Over-consolidation ratio
    double _ocr;

    //! Frequency of excitation
    double _freq;

    //! Number of cycles of excitation
    int _nCycles;
    //@}
    //! Shear modulus reduction
    NonlinearProperty* _modulusModel;

    //! Damping
    NonlinearProperty* _dampingModel;

    auto deriveModel(NonlinearProperty::Type type, QString className) -> NonlinearProperty*;
};
#endif
