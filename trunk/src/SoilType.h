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

#include "NonLinearProperty.h"
#include "Units.h"

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>
#include <QVariant>

class SoilType : public QObject
{
    Q_OBJECT

	public:
		SoilType( const Units * units = 0, QObject * parent = 0);
        ~SoilType();
        
        double untWt() const;
		void setUntWt(double untWt);

        double density() const;

        double initialDamping() const;
        void setInitialDamping(double initialDamping);
       
		void setName(const QString & name);
		const QString & name() const;

        QString toString() const;

        void setNotes(const QString & notes);
        const QString & notes() const;

		void setIsVaried(bool isVaried);
		bool isVaried() const;

        void setSaveData(bool saveData);
        bool saveData() const;

        NonLinearProperty & normShearMod();
        void setNormShearMod(const NonLinearProperty & normShearMod);

        NonLinearProperty & damping();
        void setDamping(const NonLinearProperty & damping);

		double meanStress() const;
		void setMeanStress(double meanStress);

		double PI() const;
		void setPI(double pi);

		double OCR() const;
		void setOCR(double ocr);

		double freq() const;
		void setFreq(double freq);

		double nCycles() const;
		void setNCycles(double nCycles);

		QMap<QString, QVariant> toMap() const;
		void fromMap( const QMap<QString, QVariant> & map );

        //! Create a html document containing the information of the model
        QString toHtml() const;
        
        //! Compute the shear modulus-reduction and damping curves
		void computeDarendeliCurves();

	private:
        //! Units system 
        const Units * m_units;

        //! Unit weight of the layer
		double m_untWt;

        //! Initial damping of the layer
        double m_initialDamping;
        
        //! Name of the soil layer
		QString m_name;

        //! Notes regarding the soil type
        QString m_notes;

        //! Switch which controls if the soil layer is varied
		bool m_isVaried;

        //! Save the non-linear curve data during the trials
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
		double m_nCycles;
        //@}
        //! Shear modulus reduction
        NonLinearProperty m_normShearMod;

        //! Damping
        NonLinearProperty m_damping;
};
#endif
