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

#include "Algorithms.h"
#include "Dimension.h"
#include "Serializer.h"
#include "SoilType.h"
#include "Units.h"

#include <cmath>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QList>

SoilType::SoilType( QObject * parent)
    : QObject(parent)
{ 
    m_untWt = -1;
    m_initialDamping = 5.;

	m_isVaried = true;
    m_saveData = false;

    m_normShearMod = new NonlinearProperty;
    m_normShearMod->setParent(this);
    m_normShearMod->setType(NonlinearProperty::ModulusReduction);
   
    m_damping = new NonlinearProperty;
    m_damping->setParent(this);
    m_damping->setType(NonlinearProperty::Damping);
	
    m_meanStress = 2;
	m_pi = 0;
	m_ocr = 1;
	m_freq = 1;
	m_nCycles = 10;
}

SoilType::~SoilType()
{
    delete m_normShearMod;
    delete m_damping;
}

double SoilType::untWt() const
{
	return m_untWt;
}

void SoilType::setUntWt(double untWt)
{
    if ( m_untWt != untWt ) {
        emit wasModified();
    }

	m_untWt = untWt;
}

double SoilType::density() const
{
	return m_untWt / Units::instance()->gravity();
}
        
double SoilType::initialDamping() const
{
    return m_initialDamping;
}

void SoilType::setInitialDamping(double initialDamping)
{
    if ( m_initialDamping != initialDamping ) {
        emit wasModified();
    }

    m_initialDamping = initialDamping;
}

const QString & SoilType::name() const
{
	return m_name;
}

void SoilType::setName(const QString & name) 
{
	m_name = name;
}

const QString & SoilType::notes() const
{
    return m_notes;
}

void SoilType::setNotes(const QString & notes)
{
    if ( m_notes != notes ) {
        emit wasModified();
    }

    m_notes = notes;
}

QString SoilType::toString() const
{
    return QString("%1").arg(m_name);
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
        emit wasModified();
    }

    m_saveData = saveData;
}

bool SoilType::saveData() const
{
    return m_saveData;
}

NonlinearProperty * SoilType::normShearMod()
{
    return m_normShearMod;
}

void SoilType::setNormShearMod(NonlinearProperty * normShearMod)
{
    m_normShearMod->copyValues(normShearMod);

    if (m_normShearMod->source() == NonlinearProperty::Computed) {
        computeDarendeliCurves();
    }

    emit shearModModelChanged();
    emit wasModified();
}

NonlinearProperty * SoilType::damping()
{
    return m_damping;
}

void SoilType::setDamping(NonlinearProperty * damping)
{
    m_damping->copyValues(damping);
    
    if (m_damping->source() == NonlinearProperty::Computed) {
        computeDarendeliCurves();
    }

    emit dampingModelChanged();
    emit wasModified();
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

double SoilType::PI() const
{
	return m_pi;
}
void SoilType::setPI(double pi)
{
    if ( m_pi != pi ) {
	    m_pi = pi;
        computeDarendeliCurves();
        emit wasModified();
    }
}

double SoilType::OCR() const
{
	return m_ocr;
}

void SoilType::setOCR(double ocr)
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

void SoilType::reset()
{
    m_normShearMod->reset();
    m_damping->reset();
    emit wasModified();
}

void SoilType::computeDarendeliCurves()
{
    // Create the strain vector
    QList<double> strain = Dimension::logSpace(pow(10.,-4), pow(10.,0.5), 19).toList();
    QList<double> shearMod;
    QList<double> damping;

	// Compute the reference strain based on the PI, OCR, and mean stress
	double refStrain = (0.0352 + 0.0010 * m_pi * pow(m_ocr,0.3246)) * pow(m_meanStress, 0.3483);

    // Curvature coefficient of the hyperbolic strain model
    const double curv =  0.9190;

	for (int i = 0; i < strain.size(); ++i )
    {
		// Normalized shear modulus
		shearMod << 1 / ( 1 + pow(strain.at(i) / refStrain, 0.9190));
	    
        // Minimum damping based on soil properties -- FIXME log(freq) or log10(freq)
		const double minDamping = (0.8005 + 0.0129 * m_pi * pow(m_ocr, -0.1069)) 
            * pow(m_meanStress, -0.2889) * (1 + 0.2919 * log(m_freq));
        
        // Masing damping based on shear-modulus reduction
        const double masingDamping_a1 = (100./M_PI) * 
            ( 4 * ( strain.at(i) - refStrain * log((strain.at(i) + refStrain )/ refStrain ) ) 
		    / ( pow(strain.at(i),2.) / ( strain.at(i) + refStrain ) ) - 2. );
		
        // Correction between perfect hyperbolic strain model and modified model.
		const double c1 = -1.1143 * curv * curv + 1.8618 * curv + 0.2523;
		const double c2 =  0.0805 * curv * curv - 0.0710 * curv - 0.0095;
		const double c3 = -0.0005 * curv * curv + 0.0002 * curv + 0.0003;
		
		//double masingD = c1 * masingD_a1 + c2 * pow( masingD_a1, 2. ) + c3 * pow( masingD_a1, 3. );
        const double masingDamping = c1 * masingDamping_a1 + c2 * pow(masingDamping_a1,2.) + c3 * pow(masingDamping_a1,3.);
		
        // Masing correction factor
		const double b = 0.6329 - 0.00566 * log(m_nCycles);

		// Compute the damping in percent
		damping << ( minDamping + masingDamping * b * pow(shearMod.at(i), 0.1));
	}

    if (m_normShearMod->source() == NonlinearProperty::Computed) {
        // Copy the values over to the shear modulus reduction
        m_normShearMod->strain() = strain;
        m_normShearMod->avg() = shearMod;
    }
    emit shearModModelChanged();

    if (m_damping->source() == NonlinearProperty::Computed) {
        // Copy the values over to the damping
        m_damping->strain() = strain;
        m_damping->avg()   = damping;
    }
    emit dampingModelChanged();
}

QMap<QString, QVariant> SoilType::toMap() const
{
	QMap<QString, QVariant> map;

	map.insert("untWt", m_untWt);
	map.insert("initialDamping", m_initialDamping);
	map.insert("name", m_name);
	map.insert("notes", m_notes);
	map.insert("isVaried", m_isVaried);
	map.insert("saveData", m_saveData);
	map.insert("meanStress", m_meanStress);
	map.insert("pi", m_pi);
	map.insert("ocr", m_ocr);
	map.insert("freq", m_freq);
	map.insert("nCycles", m_nCycles);

    map.insert("normShearMod", m_normShearMod->toMap());
    map.insert("damping", m_damping->toMap());

	return map;
}

void SoilType::fromMap( const QMap<QString, QVariant> & map )
{
    m_untWt = map.value("untWt").toDouble();
    m_initialDamping = map.value("initialDamping").toDouble();
	m_name = map.value("name").toString();
	m_notes = map.value("notes").toString();
	m_isVaried = map.value("isVaried").toBool();
	m_saveData = map.value("saveData").toBool();
	m_meanStress = map.value("meanStress").toDouble();
	m_pi = map.value("pi").toDouble();
	m_ocr = map.value("ocr").toDouble();
	m_freq = map.value("freq").toDouble();
	m_nCycles = map.value("nCycles").toInt();

    m_normShearMod->fromMap( map.value("normShearMod").toMap() );
    m_damping->fromMap( map.value("damping").toMap() );
}

QString SoilType::toHtml() const
{
    // Requires that the HTML header is already established.
    QString html;

    html += QString(tr("<li><a name=\"%1\">%1<a>"
                "<table border=\"0\">"
                "<tr><td><strong>Name:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Notes:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Unit Weight:</strong></td><td>%3 %4</td></tr>"
                "<tr><td><strong>Initial Damping:</strong></td><td>%5</td></tr>"
                "<tr><td><strong>Varied:</strong></td><td>%6</td></tr>"
                ))
        .arg(m_name)
        .arg(m_notes)
        .arg(m_untWt)
        .arg(Units::instance()->untWt())
        .arg(m_initialDamping)
        .arg(boolToString(m_isVaried));

    // Print the darendeli model parameters
    if ( m_normShearMod->source() == NonlinearProperty::Computed
            || m_damping->source() == NonlinearProperty::Computed )
        html += QString(tr(
               "<tr><td><strong>Mean Stress:</strong></td><td>%1 atm</td></tr>"
               "<tr><td><strong>Plasticity index:</strong></td><td>%2</td>"
               "<tr><td><strong>Over-consolidation ratio:</strong></td><td>%3</td></tr>"
               "<tr><td><strong>Excitation frequency:</strong></td><td>%4 Hz</td></tr>"
               "<tr><td><strong>Number of cycles:</strong></td><td>%5</td></tr>"
               ))
            .arg(m_meanStress)
            .arg(m_pi)
            .arg(m_ocr)
            .arg(m_freq)
            .arg(m_nCycles);

    html += "</table>";

    // Print the information of the damping and modulus reduction
    // Techincally tables aren't supposed to be used for layout, but fuck em
    html += QString("<table border = \"0\"><tr><td>%1</td><td>%2</td></tr></table>")
        .arg( m_normShearMod->toHtml() ). arg( m_damping->toHtml() );

    html += "</li>";

    return html;
}
