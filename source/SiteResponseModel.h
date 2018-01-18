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

#ifndef SITE_RESPONSE_MODEL_H_
#define SITE_RESPONSE_MODEL_H_

#include <QDataStream>
#include <QJsonObject>
#include <QThread>

class SoilProfile;
class SiteResponseOutput;
class AbstractCalculator;
class MotionLibrary;
class OutputCatalog;
class MyRandomNumGenerator;

class QProgressBar;
class QDataStream;
class QTextDocument;

class SiteResponseModel : public QThread
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const SiteResponseModel* srm);
    friend QDataStream & operator>> (QDataStream & in, SiteResponseModel* srm);

public:
    explicit SiteResponseModel( QObject * parent = 0);

    //! Method used to compute the site response
    enum Method {
        LinearElastic,  //!< Linear elastic
        EquivalentLinear, //!< Equivalent-linear (SHAKE type analysis)
        FrequencyDependent //!< Equivalent-linear with frequency dependent properties
    };

    static QStringList methodList();

    QString fileName() const;
    QTextDocument* notes() const;

    Method method() const;
    void setMethod(Method method);

    bool saveAbstractMotionData() const;

    SoilProfile* siteProfile();
    MotionLibrary* motionLibrary();
    AbstractCalculator* calculator();
    OutputCatalog* outputCatalog();
    MyRandomNumGenerator* randNumGen();

    bool modified() const;

    bool dampingRequired() const;
    bool nonlinearPropertiesRequired() const;

    //! Load the model from a binary file
    bool loadBinary(const QString & fileName);
    //! Load the model from a JSON file
    bool loadJson(const QString & fileName);

    //! Save the model to a file
    bool saveBinary();
    //! Save the model in a JSON readable format
    bool saveJson();

    //! If the model has results from an analysis
    bool hasResults() const;

    //! Create a html document containing the information of the model
    QString toHtml();

public slots:       
    void setFileName(const QString &fileName);
    void setMethod(int method);
    void setModified(bool modified = true);

    //! Stop the calculation
    void stop();

    //! Clear the results and allow the input to be modified
    void clearResults();

signals:
    void fileNameChanged(const QString &fileName);
    void methodChanged(int method);
    void modifiedChanged(bool modified);

    void calculatorChanged(AbstractCalculator* calculator);

    void progressChanged( int current );
    void progressRangeChanged( int minimum, int maximum);

    void dampingRequiredChanged(bool dampingRequired);
    void nonlinearPropertiesRequiredChanged(bool nonlinearPropertiesRequired);

    void hasResultsChanged(bool hasResults);

protected slots:
    void setIsLoaded(bool isLoaded = true);

    void setHasResults(bool hasResults);

private:
    //! Set the calculator -- called by setMethod()
    void setCalculator(AbstractCalculator* calculator);

    //! Run the calculation
    void run();

    //! If the model was modified since the last save
    bool _modified;

    //! Filename that the binary is saved to
    QString _fileName;

    //! Notes on the project for sanity of the user
    QTextDocument* _notes;

    //! Method of analysis
    Method _method;

    //! Contains all information about the site
    SoilProfile* _siteProfile;

    //! The object responsible for computing the site response
    AbstractCalculator* _calculator;

    //! Random number generator
    MyRandomNumGenerator* _randNumGen;

    //! Okay to continue calculation.
    bool _okToContinue;

    //! A list of motions
    MotionLibrary* _motionLibrary;

    //! Catalog of the output
    OutputCatalog* _outputCatalog;

    //! If the model is loaded from a file
    bool _isLoaded;

    //! If the model has results from an analysis
    bool _hasResults;
};
#endif
