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

    friend auto operator<< (QDataStream & out, const SiteResponseModel* srm) -> QDataStream &;
    friend auto operator>> (QDataStream & in, SiteResponseModel* srm) -> QDataStream &;

public:
    explicit SiteResponseModel(QObject *parent = nullptr);

    //! Method used to compute the site response
    enum Method {
        LinearElastic,  //!< Linear elastic
        EquivalentLinear, //!< Equivalent-linear (SHAKE type analysis)
        FrequencyDependent //!< Equivalent-linear with frequency dependent properties
    };

    static auto methodList() -> QStringList;

    auto fileName() const -> QString;
    auto notes() const -> QTextDocument*;

    auto method() const -> Method;
    void setMethod(Method method);

    auto saveAbstractMotionData() const -> bool;

    auto siteProfile() -> SoilProfile*;
    auto motionLibrary() -> MotionLibrary*;
    auto calculator() -> AbstractCalculator*;
    auto outputCatalog() -> OutputCatalog*;
    auto randNumGen() -> MyRandomNumGenerator*;

    auto modified() const -> bool;

    auto dampingRequired() const -> bool;
    auto nonlinearPropertiesRequired() const -> bool;

    //! Load the model from a binary file
    auto loadBinary(const QString & fileName) -> bool;
    //! Load the model from a JSON file
    auto loadJson(const QString & fileName) -> bool;

    //! Save the model to a file
    auto saveBinary() -> bool;
    //! Save the model in a JSON readable format
    auto saveJson() -> bool;

    //! If the model has results from an analysis
    auto hasResults() const -> bool;

    //! Create a html document containing the information of the model
    auto toHtml() -> QString;

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
