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

#ifndef SITE_RESPONSE_OUTPUT_H_
#define SITE_RESPONSE_OUTPUT_H_

#include "ResponseLocation.h"
#include "RatioLocation.h"
#include "NonLinearPropertyOutput.h"
#include "Dimension.h"
#include "Motion.h"
#include "SubLayer.h"
#include "EquivLinearCalc.h"
#include "SiteResponseModel.h"
#include "Units.h"

#include <QObject>
#include <QList>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>

class SiteResponseOutput : public QObject
{
    Q_OBJECT

    friend class OutputTableModel;

    public:
        SiteResponseOutput( Units * units = 0, QObject * parent = 0);
        ~SiteResponseOutput();

        //! Reset the object to the default values
        void reset();

        QStringList & motionNames();

        const QString & filePrefix() const;
        void setFilePrefix(const QString & prefix);

        bool seriesEnabledAt(int i) const;

        bool motionEnabledAt(int index) const;
        void setMotionEnabledAt(int index, bool enabled);

        bool siteEnabledAt(int index) const;
        void setSiteEnabledAt(int index, bool enabled);

        QString siteNameAt(int index) const;
        QString motionNameAt(int index) const;

        QString fileName() const;
        void setFileName(QString & fileName);

        Dimension & period();
        Dimension & freq();

        const QVector<QVector<double> > & times() const;
        const QVector<double> & depths() const;
        const QVector<QVector<double> > & strains() const;

        QList<ResponseLocation*> & responseLocations();
        QList<RatioLocation*> & ratioLocations();
        QList<Output*> & nlProperties();

        /*! @name Profiles
         */
        //@{
        Output * initialShearVel();
        Output * finalShearVel();
        Output * finalShearMod();
        Output * finalDamping();
        Output * vTotalStress();
        Output * maxError();
        Output * maxShearStress();
        Output * maxShearStrain();
        Output * maxAccel();
        Output * stressRatio();
        //@}

        double damping() const;
        void setDamping(double damping);

        const Units * units() const;
        void setUnits( Units * units );

        int siteCount() const;
        int motionCount() const;
        int totalCount() const;

        //! Determine the general index based on the site and mount index
        int generalIndex( const int siteIndex, const int motionIndex) const;
        
        //! Create list of enabled output
        void createOutputList();

        //! List of enabled output
        QList<Output *> & output();

        //! List of the output names
        const QStringList & outputNames();

        //! Load the model from a file
        bool load(const QString & fileName);

        //! Save the model to a file
        bool save(const QString & fileName);

        /*! Prepare model for saving data.
         * \param method method of analysis performed
         * \param siteCount number of sites
         * \param motions a list of the motions used in the analysis
         * \param soilTypes the soil types used in the current run
         */
        void initialize( SiteResponseModel::Method method, int siteCount, const QList<Motion*> & motions, const QList<SoilType*> & soilTypes);

        /*! Save the layering of the site profile.
         * \param siteProfile the randomized site profile
         */
        void saveProfile(SiteProfile & siteProfile);

        /*! Save the results for a run of the program.
         * \param calc the site response calculator
         */
        void saveResults( EquivLinearCalc & calc );

        /*! Export the data to files
         *
         * \param path location to save the files
         * \param separator separate the columns of data with this symbol
         * \param prefix prefix to append to the start of filenames
         */
        void exportData( const QString & path, const QString & separator = ",", const QString & prefix = "" );

        QMap<QString, QVariant> toMap(bool saveData = false) const;
        void fromMap( QMap<QString, QVariant> map );

    public slots:
        //! Compute the statistics of the output
        void computeStats();

    protected:
        QList<bool> & seriesEnabled();

        //! Clear the data vectors
        void clear();

    signals:
        void responseLocationsChanged();
        void ratioLocationsChanged();
        void nlPropertiesChanged();

        void dataChanged();

        void outputListChanged();

        void enabledChanged();

    private:
        //! Units system
        Units * m_units;

        //! Filename that the binary is saved to
        QString m_fileName;

        //! Filename prefix for the exported text files
        QString m_filePrefix;

        //! Total number of results
        int m_totalCount;

        //! Number of sites 
        int m_siteCount;

        //! Number of motions
        int m_motionCount;

        //! If the output can provide time series
        bool m_hasTime;

        //! Names of input motions
        QStringList m_motionNames;

        //! Each of the series can be enabled/disabled
        QList<bool> m_seriesEnabled;

        /*! @name Reference values
         *
         */
        //@{
        /*! Strain values for the non-linear curves.
         * Each curve has a different strain values.
         */
        QVector<QVector<double> > m_strains;

        /*! Time points for all time related output.
         * The time series values change for each recorded motion, but do not
         * change for the different site realizations.
         */
        QVector<QVector<double> > m_times;

        /*! Period points for all response spectra and response spectra ratio.
         * The period array does not change for motions or realizations of the
         * site profile and soil properties.
         */
        Dimension m_period;
        
        /*! Frequency points for the transfer function.
         * The frequency length()ension does not change for motions or realizations
         * of the site profile and soil properties.
         */
        Dimension m_freq;

        //! Depth values for a given site
        QVector<double> m_depths;

        //@}

        //! The response at a given location
        QList<ResponseLocation*> m_responseLocations;

        //! The ratio of the response at two different locations
        QList<RatioLocation*> m_ratioLocations;

        //! Non-linear soil properties
        QList<Output*> m_nlProperties;

        /*! @name Profiles
         */
        //@{
        /*! Initial shear-wave velocity.
         * Located at the middle of the layer
         */
        Output * m_initialShearVel;

        /*! Final shear-wave velocity.
         * Located at the middle of the layer
         */
        Output * m_finalShearVel;

        /*! Final shear modulus.
         * Located at the middle of the layer
         */
        Output * m_finalShearMod;

        /*! Final damping.
         * Located at the middle of the layer
         */
        Output * m_finalDamping;

        /*! Final vertical total stress.
         * Located at the middle of the layer
         */
        Output * m_vTotalStress;

        /*! Maximum shear-modulus or damping error
         * Located at the middle of the layer
         */
        Output * m_maxError;
        
        /*! Maximum shear stress.
         * Located at the middle of the layer
         */
        Output * m_maxShearStress;

        /*! Maximum shear strain.
         * Located at the middle of the layer
         */
        Output * m_maxShearStrain;

        //! Ratio of maximum shear stress to vertical total stress
        /*!
         * Located at the middle of the layer
         */
        Output * m_stressRatio;
        
        //! Maximum acceleration
        /*!
         * Located at the top of the layer
         */
        Output * m_maxAccel;
        //@}
        
        //! Damping of the response spectra in percent
        double m_damping;

        //! List of all enabled output
        QList<Output *> m_outputList;
        
        //! List of the names of all of the enabled output
        QStringList m_outputNameList;
        
        /*! Create a vector of appropriately spaced depths.
         * The vector uses the following scheme of layer thicknesses:
         *  English units:
         *   0 to  20 ft: 1 ft layers
         *  20 to  60 ft: 2 ft layers
         *  60 to 160 ft: 5 ft layers
         * 160 to 360 ft: 10 ft layers
         * beyond 360 ft: 20 ft layers
         *
         * If the vector has already been created, then it is extended to reach
         * the desired depth.
         *
         * \param maxDepth maximum depth of the vector units of Units->system()
         */
        void computeDepthVector( double maxDepth );
};
#endif
