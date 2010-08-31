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

#include "SoilType.h"

#include "Algorithms.h"
#include "CustomNonlinearProperty.h"
#include "DarendeliNonlinearProperty.h"
#include "Dimension.h"
#include "Serializer.h"
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
    m_untWt = -1;
    m_damping = 5.;

    m_isVaried = true;
    m_saveData = false;

    // Assume custom models
    m_modulusModel = new CustomNonlinearProperty(NonlinearProperty::ModulusReduction, this);
    m_dampingModel = new CustomNonlinearProperty(NonlinearProperty::Damping, this);

    m_meanStress = 2;
    m_pi = 0;
    m_ocr = 1;
    m_freq = 1;
    m_nCycles = 10;
}

double SoilType::untWt() const
{
    return m_untWt;
}

void SoilType::setUntWt(double untWt)
{
    if ( m_untWt != untWt ) {
        m_untWt = untWt;

        emit wasModified();
    }
}

double SoilType::density() const
{
    return m_untWt / Units::instance()->gravity();
}

double SoilType::damping() const
{
    return m_damping;
}

void SoilType::setDamping(double damping)
{
    if ( m_damping != damping ) {
        m_damping = damping;

        emit wasModified();        
    }
}

const QString & SoilType::name() const
{
    return m_name;
}

void SoilType::setName(const QString & name) 
{
    m_name = name;
    emit nameChanged(m_name);
}

const QString & SoilType::notes() const
{
    return m_notes;
}

void SoilType::setNotes(const QString & notes)
{
    if ( m_notes != notes ) {
        m_notes = notes;

        emit wasModified();
    }
}

bool SoilType::isVaried() const
{
    return m_isVaried;
}

void SoilType::setIsVaried(bool isVaried)
{
    if ( m_isVaried != isVaried ) {
        emit wasModified();
    }

    m_isVaried = isVaried;
}

void SoilType::setSaveData(bool saveData)
{
    if ( m_saveData != saveData ) {
        m_saveData = saveData;

        emit wasModified();
    }
}

bool SoilType::saveData() const
{
    return m_saveData;
}

NonlinearProperty* const SoilType::modulusModel()
{
    return m_modulusModel;
}

void SoilType::setModulusModel(NonlinearProperty * model)
{
    if (m_modulusModel)
        m_modulusModel->deleteLater();

    m_modulusModel = model;
    m_modulusModel->setParent(this);

    if (DarendeliNonlinearProperty *dnp = qobject_cast<DarendeliNonlinearProperty*>(m_modulusModel))
        dnp->calculate(this);

    emit modulusModelChanged(m_modulusModel);
    emit wasModified();
}

NonlinearProperty* const SoilType::dampingModel()
{
    return m_dampingModel;
}

void SoilType::setDampingModel(NonlinearProperty * model)
{
    if (m_dampingModel)
        m_dampingModel->deleteLater();

    m_dampingModel = model;
    m_dampingModel->setParent(this);

    if (DarendeliNonlinearProperty *dnp = qobject_cast<DarendeliNonlinearProperty*>(m_dampingModel))
        dnp->calculate(this);

    emit dampingModelChanged(m_dampingModel);
    emit wasModified();
}

bool SoilType::requiresSoilProperties() const
{
    return (qobject_cast<DarendeliNonlinearProperty*>(m_modulusModel)
            || qobject_cast<DarendeliNonlinearProperty*>(m_dampingModel));
}

double SoilType::meanStress() const
{
    return m_meanStress;
}

void SoilType::setMeanStress(double meanStress)
{
    if ( m_meanStress != meanStress ) {
        m_meanStress = meanStress;
        computeDarendeliCurves();
        emit wasModified();
    }

}

double SoilType::pi() const
{
    return m_pi;
}
void SoilType::setPi(double pi)
{
    if (m_pi != pi) {
        m_pi = pi;

        computeDarendeliCurves();
        emit wasModified();
    }
}

double SoilType::ocr() const
{
    return m_ocr;
}

void SoilType::setOcr(double ocr)
{
    if ( m_ocr != ocr ) {
        m_ocr = ocr;
        computeDarendeliCurves();
        emit wasModified();
    }

}

double SoilType::freq() const
{
    return m_freq;
}

void SoilType::setFreq(double freq)
{
    if ( m_freq != freq ) {
    	m_freq = freq;
        computeDarendeliCurves();
        emit wasModified();
    }
}

int SoilType::nCycles() const
{
    return m_nCycles;
}

void SoilType::setNCycles(int nCycles)
{
    if ( m_nCycles != nCycles ) {
        m_nCycles = nCycles;
        computeDarendeliCurves();
        emit wasModified();
    }
}

void SoilType::computeDarendeliCurves()
{
    DarendeliNonlinearProperty *dnp;

    // Calculate the shear modulus reduction
    dnp = qobject_cast<DarendeliNonlinearProperty*>(m_modulusModel);
    if (dnp)
        dnp->calculate(this);

    // Calculate the damping curve
    dnp = qobject_cast<DarendeliNonlinearProperty*>(m_dampingModel);
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
            .arg(m_name)
            .arg(m_notes)
            .arg(m_untWt)
            .arg(Units::instance()->untWt())
            .arg(m_damping)
            .arg(boolToString(m_isVaried));


    // Print the darendeli model parameters
    if (qobject_cast<DarendeliNonlinearProperty*>(m_modulusModel)
         || qobject_cast<DarendeliNonlinearProperty*>(m_dampingModel))
        html += tr(
                "<tr><th>Mean Stress:</th><td>%1 atm</td></tr>"
                "<tr><th>Plasticity index:</th><td>%2</td>"
                "<tr><th>Over-consolidation ratio:</th><td>%3</td></tr>"
                "<tr><th>Excitation frequency:</th><td>%4 Hz</td></tr>"
                "<tr><th>Number of cycles:</th><td>%5</td></tr>"
                )
        .arg(m_meanStress)
        .arg(m_pi)
        .arg(m_ocr)
        .arg(m_freq)
        .arg(m_nCycles);

    html += "</table>";

    // Print the information of the damping and modulus reduction
    // Techincally tables aren't supposed to be used for layout, but fuck em
    html += QString("<table border = \"0\"><tr><td>%1</td><td>%2</td></tr></table>")
            .arg( m_modulusModel->toHtml()). arg(m_dampingModel->toHtml());

    return html;
}

QDataStream & operator<< (QDataStream & out, const SoilType* st)
{
    out << (quint8)1;

    out << st->m_untWt
            << st->m_damping
            << st->m_name
            << st->m_notes
            << st->m_isVaried
            << st->m_saveData
            << st->m_meanStress
            << st->m_pi
            << st->m_ocr
            << st->m_freq
            << st->m_nCycles
            << QString(st->m_modulusModel->metaObject()->className()) << st->m_modulusModel
            << QString(st->m_dampingModel->metaObject()->className()) << st->m_dampingModel;

    return out;
}

QDataStream & operator>> (QDataStream & in, SoilType* st)
{
    quint8 ver;
    in >> ver;

    in >> st->m_untWt
            >> st->m_damping
            >> st->m_name
            >> st->m_notes
            >> st->m_isVaried
            >> st->m_saveData
            >> st->m_meanStress
            >> st->m_pi
            >> st->m_ocr
            >> st->m_freq
            >> st->m_nCycles;

    // Load both nonlinear properties
    for (int i = 0; i < 2; ++i) {
        QString className;
        in >> className;

        NonlinearProperty::Type type = (i == 0) ?
                                       NonlinearProperty::ModulusReduction : NonlinearProperty::Damping;

        NonlinearProperty* np = 0;
        if (className == "CustomNonlinearProperty") {
            np = qobject_cast<NonlinearProperty*>(new CustomNonlinearProperty(type));
        } else if (className == "DarendeliNonlinearProperty") {
            np = qobject_cast<NonlinearProperty*>(new DarendeliNonlinearProperty(type));
        } else if (className == "NonlinearProperty") {
            np = new NonlinearProperty;
        }
        Q_ASSERT(np);
        in >> np;

        if (i == 0) {
            st->setModulusModel(np);
        } else {
            st->setDampingModel(np);
        }
    }

    return in;
}
