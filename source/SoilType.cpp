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

#include "SoilType.h"

#include "Algorithms.h"
#include "CustomNonlinearProperty.h"
#include "DarendeliNonlinearProperty.h"
#include "Dimension.h"
#include "Units.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QList>

#include <cmath>

SoilType::SoilType(QObject *parent)
    : QObject(parent)
{ 
    _untWt = -1;
    _damping = 5.;
    _minDamping = 0.5;

    _isVaried = true;
    _saveData = false;

    // Assume custom models
    _modulusModel = new CustomNonlinearProperty(NonlinearProperty::ModulusReduction, false, this);
    _dampingModel = new CustomNonlinearProperty(NonlinearProperty::Damping, false, this);

    _meanStress = 2;
    _pi = 0;
    _ocr = 1;
    _freq = 1;
    _nCycles = 10;
}

double SoilType::untWt() const
{
    return _untWt;
}

void SoilType::setUntWt(double untWt)
{
    if (fabs(_untWt - untWt) > 1E-4) {
        _untWt = untWt;

        emit wasModified();
    }
}

double SoilType::density() const
{
    return _untWt / Units::instance()->gravity();
}

double SoilType::damping() const
{
    return _damping;
}

void SoilType::setDamping(double damping)
{
    if (fabs(_damping - damping) > 1E-4) {
        _damping = damping;
        emit wasModified();        
    }
}

double SoilType::minDamping() const
{
    return _minDamping;
}

void SoilType::setMinDamping(double minDamping)
{
    if (fabs(_minDamping - minDamping) > 1E-4) {
        _minDamping = minDamping;
        emit wasModified();
    }
}

const QString & SoilType::name() const
{
    return _name;
}

void SoilType::setName(const QString & name) 
{
    _name = name;
    emit nameChanged(_name);
}

const QString & SoilType::notes() const
{
    return _notes;
}

void SoilType::setNotes(const QString & notes)
{
    if ( _notes != notes ) {
        _notes = notes;

        emit wasModified();
    }
}

bool SoilType::isVaried() const
{
    return _isVaried;
}

void SoilType::setIsVaried(bool isVaried)
{
    if ( _isVaried != isVaried ) {
        emit wasModified();
    }

    _isVaried = isVaried;
}

void SoilType::setSaveData(bool saveData)
{
    if ( _saveData != saveData ) {
        _saveData = saveData;

        emit wasModified();
    }
}

bool SoilType::saveData() const
{
    return _saveData;
}

NonlinearProperty* SoilType::modulusModel()
{
    return _modulusModel;
}

void SoilType::setModulusModel(NonlinearProperty * model)
{
    if (_modulusModel)
        _modulusModel->deleteLater();

    _modulusModel = model;
    _modulusModel->setParent(this);

    if (DarendeliNonlinearProperty *dnp = qobject_cast<DarendeliNonlinearProperty*>(_modulusModel))
        dnp->calculate(this);

    emit modulusModelChanged(_modulusModel);
    emit wasModified();
}

NonlinearProperty* SoilType::dampingModel()
{
    return _dampingModel;
}

void SoilType::setDampingModel(NonlinearProperty * model)
{
    if (_dampingModel)
        _dampingModel->deleteLater();

    _dampingModel = model;
    _dampingModel->setParent(this);

    if (DarendeliNonlinearProperty *dnp = qobject_cast<DarendeliNonlinearProperty*>(_dampingModel))
        dnp->calculate(this);

    emit dampingModelChanged(_dampingModel);
    emit wasModified();
}

bool SoilType::requiresSoilProperties() const
{
    return (qobject_cast<DarendeliNonlinearProperty*>(_modulusModel)
            || qobject_cast<DarendeliNonlinearProperty*>(_dampingModel));
}

double SoilType::meanStress() const
{
    return _meanStress;
}

void SoilType::setMeanStress(double meanStress)
{
    if (fabs(_meanStress - meanStress) > 1E-4) {
        _meanStress = meanStress;
        computeDarendeliCurves();
        emit wasModified();
    }

}

double SoilType::pi() const
{
    return _pi;
}
void SoilType::setPi(double pi)
{
    if (fabs(_pi - pi) > 1E-4) {
        _pi = pi;

        computeDarendeliCurves();
        emit wasModified();
    }
}

double SoilType::ocr() const
{
    return _ocr;
}

void SoilType::setOcr(double ocr)
{
    if (fabs(_ocr - ocr) > 1E-4) {
        _ocr = ocr;
        computeDarendeliCurves();
        emit wasModified();
    }

}

double SoilType::freq() const
{
    return _freq;
}

void SoilType::setFreq(double freq)
{
    if (fabs(_freq - freq) > 1E-4) {
    	_freq = freq;
        computeDarendeliCurves();
        emit wasModified();
    }
}

int SoilType::nCycles() const
{
    return _nCycles;
}

void SoilType::setNCycles(int nCycles)
{
    if (_nCycles != nCycles) {
        _nCycles = nCycles;
        computeDarendeliCurves();
        emit wasModified();
    }
}

void SoilType::computeDarendeliCurves()
{
    DarendeliNonlinearProperty *dnp;

    // Calculate the shear modulus reduction
    dnp = qobject_cast<DarendeliNonlinearProperty*>(_modulusModel);
    if (dnp)
        dnp->calculate(this);

    // Calculate the damping curve
    dnp = qobject_cast<DarendeliNonlinearProperty*>(_dampingModel);
    if (dnp)
        dnp->calculate(this);
}

QString SoilType::toHtml() const
{
    // Requires that the HTML header is already established.
    QString html = tr("<a name=\"%1\">%1</a>"
                       "<table border=\"0\">"
                       "<tr><th>Name:</th><td>%1</td></tr>"
                       "<tr><th>Notes:</th><td>%2</td></tr>"
                       "<tr><th>Unit Weight:</th><td>%3 %4</td></tr>"
                       "<tr><th>Initial Damping:</th><td>%5</td></tr>"
                       "<tr><th>Varied:</th><td>%6</td></tr>"
                       )
            .arg(_name)
            .arg(_notes)
            .arg(_untWt)
            .arg(Units::instance()->untWt())
            .arg(_damping)
            .arg(boolToString(_isVaried));


    // Print the darendeli model parameters
    if (qobject_cast<DarendeliNonlinearProperty*>(_modulusModel)
         || qobject_cast<DarendeliNonlinearProperty*>(_dampingModel))
        html += tr(
                "<tr><th>Mean Stress:</th><td>%1 atm</td></tr>"
                "<tr><th>Plasticity index:</th><td>%2</td>"
                "<tr><th>Over-consolidation ratio:</th><td>%3</td></tr>"
                "<tr><th>Excitation frequency:</th><td>%4 Hz</td></tr>"
                "<tr><th>Number of cycles:</th><td>%5</td></tr>"
                )
        .arg(_meanStress)
        .arg(_pi)
        .arg(_ocr)
        .arg(_freq)
        .arg(_nCycles);

    html += "</table>";

    // Print the information of the damping and modulus reduction
    // Techincally tables aren't supposed to be used for layout, but fuck em
    html += QString("<table border = \"0\"><tr><td>%1</td><td>%2</td></tr></table>")
            .arg( _modulusModel->toHtml()). arg(_dampingModel->toHtml());

    return html;
}

void SoilType::fromJson(const QJsonObject &json)
{
    _untWt = json["untWt"].toDouble();
    _damping = json["damping"].toDouble();
    _name = json["name"].toString();
    _notes = json["notes"].toString();
    _isVaried = json["isVaried"].toBool();
    _saveData = json["saveData"].toBool();
    _meanStress = json["meanStress"].toDouble();
    _pi = json["pi"].toDouble();
    _ocr = json["ocr"].toDouble();
    _freq = json["freq"].toDouble();
    _nCycles = json["nCycles"].toInt();

    QString modulusType = json["modulusType"].toString();
    NonlinearProperty *mnp = deriveModel(NonlinearProperty::ModulusReduction, modulusType);
    Q_ASSERT(mnp);
    mnp->fromJson(json["modulusModel"].toObject());
    setModulusModel(mnp);

    QString dampingType = json["dampingType"].toString();
    NonlinearProperty *dnp = deriveModel(NonlinearProperty::Damping, dampingType);
    Q_ASSERT(dnp);
    dnp->fromJson(json["dampingModel"].toObject());
    setDampingModel(dnp);

    if (json.contains("minDamping")) {
        _minDamping = json["minDamping"].toDouble();
    }
}

QJsonObject SoilType::toJson() const
{
    QJsonObject json;
    json["untWt"] = _untWt;
    json["damping"] = _damping;
    json["name"] = _name;
    json["notes"] = _notes;
    json["isVaried"] = _isVaried;
    json["saveData"] = _saveData;
    json["meanStress"] = _meanStress;
    json["pi"] = _pi;
    json["ocr"] = _ocr;
    json["freq"] = _freq;
    json["nCycles"] = _nCycles;

    json["modulusType"] = _modulusModel->metaObject()->className();
    json["modulusModel"] = _modulusModel->toJson();

    json["dampingType"] = _dampingModel->metaObject()->className();
    json["dampingModel"] = _dampingModel->toJson();

    json["minDamping"] = _minDamping;

    return json;
}

QDataStream & operator<< (QDataStream & out, const SoilType* st)
{
    out << static_cast<quint8>(2);

    out << st->_untWt
            << st->_damping
            << st->_name
            << st->_notes
            << st->_isVaried
            << st->_saveData
            << st->_meanStress
            << st->_pi
            << st->_ocr
            << st->_freq
            << st->_nCycles
            << QString(st->_modulusModel->metaObject()->className()) << st->_modulusModel
            << QString(st->_dampingModel->metaObject()->className()) << st->_dampingModel;

    // Added in version 2
    out << st->_minDamping;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilType* st)
{
    quint8 ver;
    in >> ver;

    in >> st->_untWt
            >> st->_damping
            >> st->_name
            >> st->_notes
            >> st->_isVaried
            >> st->_saveData
            >> st->_meanStress
            >> st->_pi
            >> st->_ocr
            >> st->_freq
            >> st->_nCycles;

    // Load both nonlinear properties
    for (int i = 0; i < 2; ++i) {
        QString className;
        in >> className;

        NonlinearProperty::Type type = (i == 0) ? NonlinearProperty::ModulusReduction : NonlinearProperty::Damping;

        NonlinearProperty* np = st->deriveModel(type, className);

        Q_ASSERT(np);
        in >> np;

        if (i == 0) {
            st->setModulusModel(np);
        } else {
            st->setDampingModel(np);
        }
    }


    if (ver > 1) {
        in >> st->_minDamping;
    }

    return in;
}

NonlinearProperty * SoilType::deriveModel(NonlinearProperty::Type type, QString className)
{
    NonlinearProperty *np = nullptr;
    if (className == "CustomNonlinearProperty") {
        np = qobject_cast<NonlinearProperty*>(new CustomNonlinearProperty(type));
    } else if (className == "DarendeliNonlinearProperty") {
        np = qobject_cast<NonlinearProperty*>(new DarendeliNonlinearProperty(type));
    } else if (className == "NonlinearProperty") {
        np = new NonlinearProperty;
    }
    return np;
}
