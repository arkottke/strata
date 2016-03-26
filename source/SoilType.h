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

    friend QDataStream & operator<< (QDataStream & out, const SoilType* soilType);
    friend QDataStream & operator>> (QDataStream & in, SoilType* soilType);

public:
    SoilType(QObject * parent = 0);

    double untWt() const;
    void setUntWt(double untWt);

    double density() const;

    double damping() const;
    void setDamping(double damping);

    void setName(const QString & name);
    const QString & name() const;

    void setNotes(const QString & notes);
    const QString & notes() const;

    void setIsVaried(bool isVaried);
    bool isVaried() const;

    void setSaveData(bool saveData);
    bool saveData() const;

    NonlinearProperty* modulusModel();
    void setModulusModel(NonlinearProperty * model);

    NonlinearProperty* dampingModel();
    void setDampingModel(NonlinearProperty * model);

    //! If the soil properties are required
    bool requiresSoilProperties() const;

    double meanStress() const;
    double pi() const;
    double ocr() const;
    double freq() const;
    int nCycles() const;

    //! Create a html document containing the information of the model
    QString toHtml() const;

    //! Compute the shear modulus-reduction and damping curves
    void computeDarendeliCurves();

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

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
    double m_untWt;

    //! Damping of the layer
    double m_damping;

    //! Name of the soil layer
    QString m_name;

    //! Notes regarding the soil type
    QString m_notes;

    //! Switch which controls if the soil layer is varied
    bool m_isVaried;

    //! Save the nonlinear curve data during the trials
    bool m_saveData;

    /*
     * Soil properties used for the Darendeli empirical relationship
     */
    //@{
    //! Mean stress in atmospheres
    double m_meanStress;

    //! Plasticity Index
    double m_pi;

    //! Over-consolidation ratio
    double m_ocr;

    //! Frequency of excitation
    double m_freq;

    //! Number of cycles of excitation
    int m_nCycles;
    //@}
    //! Shear modulus reduction
    NonlinearProperty* m_modulusModel;

    //! Damping
    NonlinearProperty* m_dampingModel;

    NonlinearProperty* deriveModel(NonlinearProperty::Type type, QString className);
};
#endif
