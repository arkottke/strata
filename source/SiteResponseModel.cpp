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

#include "SiteResponseModel.h"

#include "AbstractCalculator.h"
#include "Algorithms.h"
#include "EquivalentLinearCalculator.h"
#include "FrequencyDependentCalculator.h"
#include "LinearElasticCalculator.h"
#include "MotionLibrary.h"
#include "MyRandomNumGenerator.h"
#include "NonlinearPropertyRandomizer.h"
#include "OutputCatalog.h"
#include "ProfilesOutputCatalog.h"
#include "ProfileRandomizer.h"
#include "SoilProfile.h"
#include "SoilTypesOutputCatalog.h"
#include "TextLog.h"
#include "Units.h"

#include <QApplication>
#include <QDataStream>
#include <QFile>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaProperty>
#include <QTimer>
#include <QProgressBar>
#include <QTextDocument>

#include <QDebug>

SiteResponseModel::SiteResponseModel(QObject * parent)
    : QThread(parent), _calculator(0)
{
    _modified = false;
    _hasResults = false;
    _method = EquivalentLinear;
    _okToContinue = true;
    _isLoaded = false;

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(setModified()));

    _randNumGen = new MyRandomNumGenerator(this);
    connect(_randNumGen, SIGNAL(wasModified()),
            this, SLOT(setModified()));

    _motionLibrary = new MotionLibrary(this);
    _motionLibrary->setReadOnly(_hasResults);
    connect(_motionLibrary, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            _motionLibrary, SLOT(setReadOnly(bool)));

    _siteProfile = new SoilProfile(this);
    _siteProfile->setReadOnly(_hasResults);
    connect( _siteProfile, SIGNAL(wasModified()),
             this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            _siteProfile, SLOT(setReadOnly(bool)));

    _outputCatalog = new OutputCatalog(this);
    _outputCatalog->setReadOnly(_hasResults);
    connect(_motionLibrary, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            _outputCatalog, SLOT(setReadOnly(bool)));
    connect(_outputCatalog, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this->motionLibrary(), SIGNAL(approachChanged(int)),
            _outputCatalog->profilesCatalog(), SLOT(setApproach(int)));

    // Associate the output soil types catalog with the input soil types catalog
    // Should these have a stronger link? As in be the same object?
    _outputCatalog->soilTypesCatalog()->setSoilTypeCatalog(_siteProfile->soilTypeCatalog());

    setCalculator(new EquivalentLinearCalculator(this));

    _notes = new QTextDocument(this);
    connect(_notes, SIGNAL(contentsChanged()), this, SLOT(setModified()));

    Units::instance()->reset();
}

QStringList SiteResponseModel::methodList()
{
    QStringList list;

    list << tr("Linear Elastic")
         << tr("Equivalent Linear (EQL)");

#ifdef ADVANCED_FEATURES
    list << tr("Frequency Dependent EQL");
#endif

    return list;
}

QString SiteResponseModel::fileName() const
{
    return _fileName;
}

void SiteResponseModel::setFileName(const QString & fileName)
{
    if (_fileName != fileName) {
        _fileName = fileName;

        emit fileNameChanged(_fileName);
        setModified(true);
    }
}

SiteResponseModel::Method SiteResponseModel::method() const
{
    return _method;
}

void SiteResponseModel::setMethod(Method method)
{
    if ( _method != method ) {
        _method = method;

        switch (_method) {
        case LinearElastic:
            setCalculator(new LinearElasticCalculator(this));
            break;
        case EquivalentLinear:
            setCalculator(new EquivalentLinearCalculator(this));
            break;
        case FrequencyDependent:
            setCalculator(new FrequencyDependentCalculator(this));
            break;
        }

        emit methodChanged(_method);
        emit dampingRequiredChanged(dampingRequired());
        emit nonlinearPropertiesRequiredChanged(nonlinearPropertiesRequired());
        setModified(true);
    }
}

void SiteResponseModel::setMethod(int method)
{
    setMethod((Method)method);
}

bool SiteResponseModel::dampingRequired() const
{
    return _method == LinearElastic;
}

bool SiteResponseModel::nonlinearPropertiesRequired() const
{
    return (_method == EquivalentLinear
            || _method == FrequencyDependent);
}

bool SiteResponseModel::modified() const
{
    return _modified;
}

void SiteResponseModel::setModified(bool modified)
{    
    _modified = modified;
    emit modifiedChanged(modified);

    //     FIXME
    //    if (_modified != modified && _isLoaded) {
    //        // FIXME When opening a previously saved document, the QTextDocument
    //        // containing the notes signals that it has been modified.  This happens
    //        // before the modified flag is reset.
    //    }
}

QTextDocument * SiteResponseModel::notes() const
{
    return _notes;
}

void SiteResponseModel::stop()
{
    _okToContinue = false;
    _calculator->stop();
}

void SiteResponseModel::clearResults()
{
    _outputCatalog->clear();
    setHasResults(false);
}

void SiteResponseModel::setIsLoaded(bool isLoaded)
{
    _isLoaded = isLoaded;
}

SoilProfile* SiteResponseModel::siteProfile()
{
    return _siteProfile;
}

MotionLibrary* SiteResponseModel::motionLibrary()
{
    return _motionLibrary;
}

AbstractCalculator* SiteResponseModel::calculator()
{
    return _calculator;
}

OutputCatalog* SiteResponseModel::outputCatalog()
{
    return _outputCatalog;
}

MyRandomNumGenerator* SiteResponseModel::randNumGen()
{
    return _randNumGen;
}

void SiteResponseModel::setCalculator(AbstractCalculator *calculator)
{
    if (_calculator != calculator) {
        if (_calculator)
            _calculator->deleteLater();

        _calculator = calculator;
        _calculator->setTextLog(_outputCatalog->log());

        connect(_calculator, SIGNAL(wasModified()),
                this, SLOT(setModified()));

        emit calculatorChanged(_calculator);
    }
}

bool SiteResponseModel::loadBinary(const QString &fileName)
{
    setModified(false);
    _isLoaded = false;

    // Save the file name
    _fileName = fileName;

    QFile file(fileName);
    // If the file can't be opened halt
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Unable to open file: %s", qPrintable(fileName));
        return false;

    }
    // Open the data stream
    QDataStream inStream(&file);
    inStream.setVersion(QDataStream::Qt_4_0);

    // Read and check the header
    quint32 magic;
    inStream >> magic;

    if (magic != 0xA1B2) {
        qCritical() << "Bad file format!";
        return false;
    }

    // Read the data
    inStream >> this;

    setModified(false);
    // Wait until all events have been processed and then reset the modified flag
    //QTimer::singleShot(10, this, SLOT(setIsLoaded()));

    return true;
}

bool SiteResponseModel::loadJson(const QString & fileName)
{
    setModified(false);
    _isLoaded = false;

    // Save the file name
    _fileName = fileName;

    QFile file(_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Unable to open file: %s", qPrintable(fileName));
        return false;
    }

    QByteArray savedData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(savedData);

    QJsonObject json = jsonDoc.object();

    //
    _notes->setHtml(json["notes"].toString());
    Units::instance()->setSystem(json["system"].toInt());

    _randNumGen->fromJson(json["randNumGen"].toObject());
    _siteProfile->fromJson(json["siteProfile"].toObject());
    _motionLibrary->fromJson(json["motionLibrary"].toObject());
    _outputCatalog->fromJson(json["outputCatalog"].toObject());

    setMethod(json["method"].toInt());
    const QJsonObject cjo = json["calculator"].toObject();
    switch (_method)
    {
    case SiteResponseModel::EquivalentLinear:
        qobject_cast<EquivalentLinearCalculator*>(_calculator)->fromJson(cjo);
        break;
    case SiteResponseModel::FrequencyDependent:
        qobject_cast<FrequencyDependentCalculator*>(_calculator)->fromJson(cjo);
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    setHasResults(json["hasResults"].toBool());

    _outputCatalog->initialize(
                _siteProfile->isVaried() ?  _siteProfile->profileCount() : 1, _motionLibrary);

    if (_hasResults) {
        _outputCatalog->finalize();
    }

    setModified(false);
    return true;
}

bool SiteResponseModel::saveBinary()
{
    QFile file(_fileName);
    // If the file can't be opened halt
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Error opening file: %s" << qPrintable(_fileName);
        return false;
    }
    // Open the data stream
    QDataStream outStream(&file);
    outStream.setVersion(QDataStream::Qt_4_0);

    // Write a header with a "magic number" and a version
    outStream << (quint32)0xA1B2;

    outStream << this;

    setModified(false);
    return true;
}

bool SiteResponseModel::saveJson()
{
    QJsonObject json;

    json["notes"] = _notes->toPlainText();
    json["method"] = (int) _method;
    json["hasResults"] = _hasResults;
    json["system"] = (int) Units::instance()->system();

    json["randNumGen"] = _randNumGen->toJson();
    json["siteProfile"] = _siteProfile->toJson();
    json["motionLibrary"] = _motionLibrary->toJson();
    json["outputCatalog"] = _outputCatalog->toJson();

    switch (_method) {
    case SiteResponseModel::EquivalentLinear:
        json["calculator"] = qobject_cast<EquivalentLinearCalculator*>(_calculator)->toJson();
        break;
    case SiteResponseModel::FrequencyDependent:
        json["calculator"] = qobject_cast<FrequencyDependentCalculator*>(_calculator)->toJson();
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    QFile file(_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonDocument saveDoc(json);
    file.write(saveDoc.toJson(QJsonDocument::Indented));

    setModified(false);
    return true;
}

bool SiteResponseModel::hasResults() const
{
    return _hasResults;
}

void SiteResponseModel::setHasResults(bool hasResults)
{
    if (_hasResults != hasResults) {
        _hasResults = hasResults;

        emit hasResultsChanged(hasResults);
    }
}

void SiteResponseModel::run()
{    
    _okToContinue = true;
    setHasResults(false);

    // Check the input -- FIMXE add more thorough checks
    if (_siteProfile->rowCount() < 2) {
        qCritical() << "Need at least one soil layer!";
        _okToContinue = false;
    }

    if (!_motionLibrary->rowCount()) {
        qCritical() << "Need at least one input motion!";
        _okToContinue = false;
    }

    if (!_okToContinue)
        return;

    _outputCatalog->clear();
    _outputCatalog->log()->append(tr("<b>Starting Strata Calculation</b>"));

    // Determine the number of sites to be used in the computation
    const int siteCount = _siteProfile->isVaried() ? _siteProfile->profileCount() : 1;

    // Initialize the random number generator
    _randNumGen->init();

    // Initialize the output
    _outputCatalog->initialize(siteCount, _motionLibrary);

    // Setup the progress bar with the number of steps
    const int motionCount = _motionLibrary->motionCount();
    const int totalCount = motionCount * siteCount;
    emit progressRangeChanged(0, totalCount);
    emit progressChanged(0);

    _outputCatalog->log()->append(tr("%1 Trial(s) (%2 Site(s) and %3 Motion(s) )")
                                   .arg(totalCount)
                                   .arg(siteCount)
                                   .arg(motionCount));

    int count = 0;
    for (int i = 0; i < siteCount; ++i) {
        // Break if not okay to continue
        if ( !_okToContinue )
            break;

        _outputCatalog->log()->append((
                    QString(tr("[%1 of %2] Generating site and soil properties")).arg(i+1).arg(siteCount)));

        // Create the sublayers -- this randomizes the properties
        _siteProfile->createSubLayers(_outputCatalog->log());

        // FIXME -- check the site profile to ensure that the waves can be
        // computed for the intial coniditions
        int motionCountOffset = 0;
        for (int j = 0; j < _motionLibrary->rowCount(); ++j ) {
            if (!_motionLibrary->motionAt(j)->enabled()) {
                // Skip the disabled motion
                ++motionCountOffset;
                continue;
            }

            if (!_okToContinue)
                // Break if not okay to continue
                break;

            // Output status
            _outputCatalog->log()->append(QString(tr("\t[%1 of %2] Computing site response for motion: %3"))
                                           .arg(j - motionCountOffset + 1)
                                           .arg(motionCount)
                                           .arg(_motionLibrary->motionAt(j)->name()));

            // Compute the site response
            if (!_calculator->run(_motionLibrary->motionAt(j), _siteProfile) && _okToContinue) {
                _outputCatalog->log()->append(tr("\tWave propagation error -- removing site."));
                // Error in the calculation -- need to remove the site
                _outputCatalog->removeLastSite();
                // Reset site count
                --i;
                break;
            }

            if (!_okToContinue)
                // Break if not okay to continue
                break;

            // Generate the output
            _outputCatalog->saveResults(j - motionCountOffset, _calculator);

            // Increment the progress bar
            ++count;
            emit progressChanged(count);

            // Reset the sublayers
            _siteProfile->resetSubLayers();
        }
    }

    if (_okToContinue) {
        // Compute the statistics of the output
        _outputCatalog->log()->append(tr("Computing statistics."));
        _outputCatalog->finalize();
        setHasResults(true);
    } else {
        _outputCatalog->log()->append(tr("<b>Canceled by the user.</b>"));
        setHasResults(false);
    }
}

QString SiteResponseModel::toHtml()
{
    QString html;

    // Define the html header
    html += "<html><head>"
            "<title>Strata Input</title>"
            "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />"
            "<style type=\"text/css\">"
            "ol {"
            "list-style-type: upper-roman;"
            "font-size: 	medium;"
            "font-weight:   bold;"
            "}"
            "ol ol {"
            "list-style-type: upper-alpha;"
            "font-size: 	medium;"
            "font-weight:   bold;"
            "}"
            "ol ol ol {"
            "list-style-type: decimal;"
            "font-size: 	medium;"
            "font-weight:   bold;"
            "}"
            "ol ol ol ol {"
            "list-style-type: lower-alpha;"
            "font-size: 	medium;"
            "font-weight:   bold;"
            "}"
            "strong {"
            "font-size:     small;"
            "font-weight:   bold;"
            "}"
            "th {"
            "font-size:     small;"
            "font-weight:   bold;"
            "padding: 2px 4px 2px 4px;"
            "align: right;"
            "}"
            "td {"
            "font-size: 	small;"
            "font-weight:   normal;"
            "padding: 2px 4px 2px 4px;"
            "}"
            "table {"
            "border-style:  solid;"
            "border-collapse:  collapse;"
            "}"
            "</style>"
            "</head>";

    // Project
    html += tr("<h1>%1</h1>"
               "<ol>"
               "<li>General Settings"
               "<ol>"
               "<li>Project"
               "<table border=\"0\">"
               "<tr><th>Title:</th><td>%1</td></tr>"
               "<tr><th>Notes:</th><td>%2</td></tr>"
               "<tr><th>File preffix:</th><td>%3</td></tr>"
               "<tr><th>Units System:</th><td>%4</td></tr>"
               "</table>"
               "</li>"
               )
            .arg(_outputCatalog->title())
            .arg(_notes->toHtml())
            .arg(_outputCatalog->filePrefix())
            .arg(Units::instance()->systemList().at(Units::instance()->system()));


    // Type of Analysis
    html += tr(
                "<li>Type of Analysis"
                "<table border=\"0\">"
                "<tr><th>Analysis Method:</th><td>%1</td></tr>"
                "<tr><th>Approach:</th><td>%2</td></tr>"
                "<tr><th>Properties Varied:</th><td>%3</td></tr>"
                "</table>"
                "</li>"
                )
            .arg(methodList().at(_method))
            .arg(MotionLibrary::approachList().at(_motionLibrary->approach()))
            .arg(boolToString(_siteProfile->isVaried()));

    // Site Variation
    if ( _siteProfile->isVaried() )
        html += tr(
                    "<li>Site Property Variation"
                    "<table border=\"0\">"
                    "<tr><th>Number of realizations:</th><td>%1</td></tr>"
                    "<tr><th>Vary the nonlinear soil properties:</th><td>%2</td></tr>"
                    "<tr><th>Vary the site profile:</th><td>%3</td></tr>"
                    "</table>"
                    "</li>"
                    )
                .arg(_siteProfile->profileCount())
                .arg(boolToString(_siteProfile->nonlinearPropertyRandomizer()->enabled()))
                .arg(boolToString(_siteProfile->profileRandomizer()->enabled()));

    // Layer Discretization
    html += tr(
                "<li>Layer Discretization"
                "<table border=\"0\">"
                "<tr><th>Maximum frequency:</th><td>%1 Hz</td></tr>"
                "<tr><th>Wavelength fraction:</th><td>%2</td></tr>"
                "</table>"
                "</li>"
                )
            .arg(_siteProfile->maxFreq())
            .arg(_siteProfile->waveFraction());

    // Calculator parameters
    html += _calculator->toHtml();

    html += "</ol>";

    // Site profile
    html += "<li>" + _siteProfile->toHtml() + "</li>";

    // Motions
    html += tr("<li>Motion(s)");

    QString loc;
    if (_siteProfile->inputDepth() < 0)
        loc = "Bedrock";
    else
        loc = QString("%1 %2").arg(_siteProfile->inputDepth()).arg(Units::instance()->length());

    html += tr("<table border=\"0\"><tr><th>Input Location:</th><td>%1</td></tr></table>")
            .arg(loc);

    html += _motionLibrary->toHtml() + "</li>";

    // Close the html file
    html += "</ol></html>";

    return html;
}

QDataStream & operator<< (QDataStream & out, const SiteResponseModel* srm)
{
    out << (quint8)2;

    out << Units::instance()
        << srm->_notes->toPlainText()
        << srm->_method
        << srm->_siteProfile
        << srm->_motionLibrary
        << srm->_outputCatalog
        << srm->_randNumGen
        << srm->_hasResults;

    switch (srm->_method) {
    case SiteResponseModel::EquivalentLinear:
        out << qobject_cast<EquivalentLinearCalculator*>(srm->_calculator);
        break;
    case SiteResponseModel::FrequencyDependent:
        out << qobject_cast<FrequencyDependentCalculator*>(srm->_calculator);
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, SiteResponseModel* srm)
{
    quint8 ver;
    in >> ver;

    QString notes;
    int method;
    bool hasResults;

    in >> Units::instance()
       >> notes
       >> method
       >> srm->_siteProfile
       >> srm->_motionLibrary // TODO Need to update motion count
       >> srm->_outputCatalog;
    
    if (ver > 1) {
        in >> srm->_randNumGen;
    }

    in >> hasResults;

    srm->_notes->setPlainText(notes);
    srm->setMethod(method);
    srm->_outputCatalog->initialize(
                srm->_siteProfile->isVaried() ?  srm->_siteProfile->profileCount() : 1,
                srm->_motionLibrary);

    if (hasResults)
        srm->_outputCatalog->finalize();

    switch (srm->_method) {
    case SiteResponseModel::EquivalentLinear:
        in >> qobject_cast<EquivalentLinearCalculator*>(srm->_calculator);
        break;
    case SiteResponseModel::FrequencyDependent:
        in >> qobject_cast<FrequencyDependentCalculator*>(srm->_calculator);
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    // Need to update the other objects that the model has data and should not be editted.
    srm->setHasResults(hasResults);

    return in;
}
