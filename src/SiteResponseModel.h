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

#ifndef SITE_RESPONSE_MODEL_H_
#define SITE_RESPONSE_MODEL_H_

#include "SiteProfile.h"
#include "EquivLinearCalc.h"
#include "RecordedMotion.h"
#include "RvtMotion.h"
#include "NonlinearPropertyLibrary.h"

#include <QList>
#include <QMap>
#include <QProgressBar>
#include <QString>
#include <QTextDocument>
#include <QThread>
#include <QVariant>

class SiteResponseOutput; // Forward declaration of SiteResponseOutput

class SiteResponseModel : public QThread
{
	Q_OBJECT

	public:
		SiteResponseModel( QObject * parent = 0);
        ~SiteResponseModel();

        //! Method used to compute the site response
		enum Method { 
            RecordedMotions,  //!< Recorded motions
            RandomVibrationTheory //!< Random Vibration Theory
        };

        static QStringList methodList();

        //! Reset the object to the default values
        void reset();
        
        QString fileName() const;
		QString title() const;
		QTextDocument * notes() const;
		QString filePrefix() const;

		Method method() const;
        void setMethod(Method method);
        
        bool saveMotionData() const;
        bool okToContinue() const;
        bool wasSucessful() const;

        //! Set that it is okay to continue computing
        void setOkToContinue(bool okToContinue);

		SiteProfile * siteProfile();
		EquivLinearCalc * calculator();
        QList<RecordedMotion*> & recordedMotions();
        RvtMotion * rvtMotion();

        NonlinearPropertyLibrary * nlPropertyLibrary();

        SiteResponseOutput * output();

        //! Set read-only
        bool readOnly() const;

        bool modified() const;

        //! Load the model from a file
        bool load(const QString & fileName);

        //! Save the model to a file
        bool save(const QString & fileName);

		QMap<QString, QVariant> toMap() const;
		void fromMap(const QMap<QString, QVariant> & map);

        //! Create a html document containing the information of the model
        QString toHtml();

    public slots:
		void setMethod(int method);
        void setSaveMotionData(bool saveData);

        //! Set the model is read only
        void setReadOnly(bool readOnly);

        void stop();
        
        //! Set if the model was modified
        void setModified(bool modified = true);

    protected slots:
        void setIsLoaded(bool isLoaded = true);

    signals:
        void recordedMotionsChanged();
        void rvtMotionChanged();

        void modifiedChanged(bool modified);
        
        void progressChanged( int current );
        void progressRangeChanged( int minimum, int maximum);

        void readOnlyChanged(bool readOnly);

        void isWorkingChanged(bool isWorking);
	private:
        //! Run the calculation
        void run();

        //! If the model was modified since the last save
        bool m_modified;

        //! If the model is read only
        bool m_readOnly;

        //! The fileName that the model was saved too or loaded from
        QString m_fileName;

        //! Notes on the project for sanity of the user
		QTextDocument * m_notes;
        
        //! Type of analysis
		Method m_method;
        
        //! Contains all information about the site
		SiteProfile * m_siteProfile;
        
        //! The object responsible for computing the site response
		EquivLinearCalc * m_calculator;

        //! Okay to continue calculation.
        bool m_okToContinue;
        
        //! If the calculation was successful.
        bool m_wasSuccessful;
        
        //! A list of recorded motions
        /*! Both the recorded and random vibration theory motions are stored in
         * the SiteResponseModel as it allows the same site to be easily
         * analysis using the two different methods.
         */
        QList<RecordedMotion*> m_recordedMotions;
        
        //! Save the motion data in the input file
        bool m_saveMotionData;
        
        //! The motion used for the random vibration theory analysis
        RvtMotion * m_rvtMotion;

        //! Ouput from the analysis
        SiteResponseOutput * m_output;

        //! Previously defined nonlinear curves
        NonlinearPropertyLibrary * m_nlPropertyLibrary;

        //! If the information a file is being loaded and should not influence the modified flag.
        bool m_isLoaded;
};
#endif
