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
    : QThread(parent), m_calculator(0)
{
    m_modified = false;
    m_hasResults = false;
    m_method = EquivalentLinear;
    m_okToContinue = true;
    m_isLoaded = false;

    connect(Units::instance(), SIGNAL(systemChanged(int)),
            this, SLOT(setModified()));

    m_randNumGen = new MyRandomNumGenerator(this);
    connect(m_randNumGen, SIGNAL(wasModified()),
            this, SLOT(setModified()));

    m_motionLibrary = new MotionLibrary(this);
    m_motionLibrary->setReadOnly(m_hasResults);
    connect(m_motionLibrary, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            m_motionLibrary, SLOT(setReadOnly(bool)));

    m_siteProfile = new SoilProfile(this);
    m_siteProfile->setReadOnly(m_hasResults);
    connect( m_siteProfile, SIGNAL(wasModified()),
             this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            m_siteProfile, SLOT(setReadOnly(bool)));

    m_outputCatalog = new OutputCatalog(this);
    m_outputCatalog->setReadOnly(m_hasResults);
    connect(m_motionLibrary, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this, SIGNAL(hasResultsChanged(bool)),
            m_outputCatalog, SLOT(setReadOnly(bool)));
    connect(m_outputCatalog, SIGNAL(wasModified()),
            this, SLOT(setModified()));
    connect(this->motionLibrary(), SIGNAL(approachChanged(int)),
            m_outputCatalog->profilesCatalog(), SLOT(setApproach(int)));

    // Associate the output soil types catalog with the input soil types catalog
    // Should these have a stronger link? As in be the same object?
    m_outputCatalog->soilTypesCatalog()->setSoilTypeCatalog(m_siteProfile->soilTypeCatalog());

    setCalculator(new EquivalentLinearCalculator(this));

    m_notes = new QTextDocument(this);
    connect(m_notes, SIGNAL(contentsChanged()), this, SLOT(setModified()));

    Units::instance()->reset();
}

QStringList SiteResponseModel::methodList()
{
    QStringList list;

    list << tr("Linear Elastic")
         << tr("Equivalent Linear (EQL)");

#ifdef ADVANCED_OPTIONS
    list << tr("Frequency Dependent EQL");
#endif

    return list;
}

QString SiteResponseModel::fileName() const
{
    return m_fileName;
}

void SiteResponseModel::setFileName(const QString & fileName)
{
    if (m_fileName != fileName) {
        m_fileName = fileName;

        emit fileNameChanged(m_fileName);
        setModified(true);
    }
}

SiteResponseModel::Method SiteResponseModel::method() const
{
    return m_method;
}

void SiteResponseModel::setMethod(Method method)
{
    if ( m_method != method ) {
        m_method = method;

        switch (m_method) {
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

        emit methodChanged(m_method);
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
    return m_method == LinearElastic;
}

bool SiteResponseModel::nonlinearPropertiesRequired() const
{
    return (m_method == EquivalentLinear
            || m_method == FrequencyDependent);
}

bool SiteResponseModel::modified() const
{
    return m_modified;
}

void SiteResponseModel::setModified(bool modified)
{    
    m_modified = modified;
    emit modifiedChanged(modified);

    //     FIXME
    //    if (m_modified != modified && m_isLoaded) {
    //        // FIXME When opening a previously saved document, the QTextDocument
    //        // containing the notes signals that it has been modified.  This happens
    //        // before the modified flag is reset.
    //    }
}

QTextDocument * SiteResponseModel::notes() const
{
    return m_notes;
}

void SiteResponseModel::stop()
{
    m_okToContinue = false;
    m_calculator->stop();
}

void SiteResponseModel::clearResults()
{
    m_outputCatalog->clear();
    setHasResults(false);
}

void SiteResponseModel::setIsLoaded(bool isLoaded)
{
    m_isLoaded = isLoaded;
}

SoilProfile* SiteResponseModel::siteProfile()
{
    return m_siteProfile;
}

MotionLibrary* SiteResponseModel::motionLibrary()
{
    return m_motionLibrary;
}

AbstractCalculator* SiteResponseModel::calculator()
{
    return m_calculator;
}

OutputCatalog* SiteResponseModel::outputCatalog()
{
    return m_outputCatalog;
}

MyRandomNumGenerator* SiteResponseModel::randNumGen()
{
    return m_randNumGen;
}

void SiteResponseModel::setCalculator(AbstractCalculator *calculator)
{
    if (m_calculator != calculator) {
        if (m_calculator)
            m_calculator->deleteLater();

        m_calculator = calculator;
        m_calculator->setTextLog(m_outputCatalog->log());

        connect(m_calculator, SIGNAL(wasModified()),
                this, SLOT(setModified()));

        emit calculatorChanged(m_calculator);
    }
}

bool SiteResponseModel::loadBinary(const QString &fileName)
{
    setModified(false);
    m_isLoaded = false;

    // Save the file name
    m_fileName = fileName;

    QFile file(fileName);
    // If the file can't be opened halt
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Error opening file: %s", qPrintable(fileName));
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
    m_isLoaded = false;

    // Save the file name
    m_fileName = fileName;

    QFile file(m_fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray savedData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(savedData);

    QJsonObject json = jsonDoc.object();

    //
    m_notes->setHtml(json["notes"].toString());
    Units::instance()->setSystem(json["system"].toInt());

    m_randNumGen->fromJson(json["randNumGen"].toObject());
    m_siteProfile->fromJson(json["siteProfile"].toObject());
    m_motionLibrary->fromJson(json["motionLibrary"].toObject());
    m_outputCatalog->fromJson(json["outputCatalog"].toObject());

    setMethod(json["method"].toInt());
    const QJsonObject cjo = json["calculator"].toObject();
    switch (m_method)
    {
    case SiteResponseModel::EquivalentLinear:
        qobject_cast<EquivalentLinearCalculator*>(m_calculator)->fromJson(cjo);
        break;
    case SiteResponseModel::FrequencyDependent:
        qobject_cast<FrequencyDependentCalculator*>(m_calculator)->fromJson(cjo);
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    setHasResults(json["hasResults"].toBool());

    m_outputCatalog->initialize(
                m_siteProfile->isVaried() ?  m_siteProfile->profileCount() : 1, m_motionLibrary);

    if (m_hasResults) {
        m_outputCatalog->finalize();
    }

    setModified(false);
    return true;
}

bool SiteResponseModel::saveBinary()
{
    QFile file(m_fileName);
    // If the file can't be opened halt
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Error opening file: %s" << qPrintable(m_fileName);
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

    json["notes"] = m_notes->toPlainText();
    json["method"] = (int) m_method;
    json["hasResults"] = m_hasResults;
    json["system"] = (int) Units::instance()->system();

    json["randNumGen"] = m_randNumGen->toJson();
    json["siteProfile"] = m_siteProfile->toJson();
    json["motionLibrary"] = m_motionLibrary->toJson();
    json["outputCatalog"] = m_outputCatalog->toJson();

    switch (m_method) {
    case SiteResponseModel::EquivalentLinear:
        json["calculator"] = qobject_cast<EquivalentLinearCalculator*>(m_calculator)->toJson();
        break;
    case SiteResponseModel::FrequencyDependent:
        json["calculator"] = qobject_cast<FrequencyDependentCalculator*>(m_calculator)->toJson();
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    QFile file(m_fileName);
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
    return m_hasResults;
}

void SiteResponseModel::setHasResults(bool hasResults)
{
    if (m_hasResults != hasResults) {
        m_hasResults = hasResults;

        emit hasResultsChanged(hasResults);
    }
}

void SiteResponseModel::run()
{    
    m_okToContinue = true;
    setHasResults(false);

    // Check the input -- FIMXE add more thorough checks
    if (m_siteProfile->rowCount() < 2) {
        qCritical() << "Need at least one soil layer!";
        m_okToContinue = false;
    }

    if (!m_motionLibrary->rowCount()) {
        qCritical() << "Need at least one input motion!";
        m_okToContinue = false;
    }

    if (!m_okToContinue)
        return;

    m_outputCatalog->clear();
    m_outputCatalog->log()->append(tr("<b>Starting Strata Calculation</b>"));

    // Determine the number of sites to be used in the computation
    const int siteCount = m_siteProfile->isVaried() ? m_siteProfile->profileCount() : 1;

    // Initialize the random number generator
    m_randNumGen->init();

    // Initialize the output
    m_outputCatalog->initialize(siteCount, m_motionLibrary);

    // Setup the progress bar with the number of steps
    const int motionCount = m_motionLibrary->motionCount();
    const int totalCount = motionCount * siteCount;
    emit progressRangeChanged(0, totalCount);
    emit progressChanged(0);

    m_outputCatalog->log()->append(tr("%1 Trials (%2 Sites and %3 Motions )")
                                   .arg(totalCount)
                                   .arg(siteCount)
                                   .arg(motionCount));

    int count = 0;
    for (int i = 0; i < siteCount; ++i) {
        // Break if not okay to continue
        if ( !m_okToContinue )
            break;

        m_outputCatalog->log()->append((
                    QString(tr("[%1 of %2] Generating site and soil properties")).arg(i+1).arg(siteCount)));

        // Create the sublayers -- this randomizes the properties
        m_siteProfile->createSubLayers(m_outputCatalog->log());

        // FIXME -- check the site profile to ensure that the waves can be
        // computed for the intial coniditions
        int motionCountOffset = 0;
        for (int j = 0; j < m_motionLibrary->rowCount(); ++j ) {
            if (!m_motionLibrary->motionAt(j)->enabled()) {
                // Skip the disabled motion
                ++motionCountOffset;
                continue;
            }

            if (!m_okToContinue)
                // Break if not okay to continue
                break;

            // Output status
            m_outputCatalog->log()->append(QString(tr("\t[%1 of %2] Computing site response for motion: %3"))
                                           .arg(j - motionCountOffset + 1)
                                           .arg(motionCount)
                                           .arg(m_motionLibrary->motionAt(j)->name()));

            // Compute the site response
            if (!m_calculator->run(m_motionLibrary->motionAt(j), m_siteProfile) && m_okToContinue) {
                m_outputCatalog->log()->append(tr("\tWave propagation error -- removing site."));
                // Error in the calculation -- need to remove the site
                m_outputCatalog->removeLastSite();
                // Reset site count
                --i;
                break;
            }

            if (!m_okToContinue)
                // Break if not okay to continue
                break;

            // Generate the output
            m_outputCatalog->saveResults(j - motionCountOffset, m_calculator);

            // Increment the progress bar
            ++count;
            emit progressChanged(count);

            // Reset the sublayers
            m_siteProfile->resetSubLayers();
        }
    }

    if (m_okToContinue) {
        // Compute the statistics of the output
        m_outputCatalog->log()->append(tr("Computing statistics."));
        m_outputCatalog->finalize();
        setHasResults(true);
    } else {
        m_outputCatalog->log()->append(tr("<b>Canceled by the user.</b>"));
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
            .arg(m_outputCatalog->title())
            .arg(m_notes->toHtml())
            .arg(m_outputCatalog->filePrefix())
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
            .arg(methodList().at(m_method))
            .arg(MotionLibrary::approachList().at(m_motionLibrary->approach()))
            .arg(boolToString(m_siteProfile->isVaried()));

    // Site Variation
    if ( m_siteProfile->isVaried() )
        html += tr(
                    "<li>Site Property Variation"
                    "<table border=\"0\">"
                    "<tr><th>Number of realizations:</th><td>%1</td></tr>"
                    "<tr><th>Vary the nonlinear soil properties:</th><td>%2</td></tr>"
                    "<tr><th>Vary the site profile:</th><td>%3</td></tr>"
                    "</table>"
                    "</li>"
                    )
                .arg(m_siteProfile->profileCount())
                .arg(boolToString(m_siteProfile->nonlinearPropertyRandomizer()->enabled()))
                .arg(boolToString(m_siteProfile->profileRandomizer()->enabled()));

    // Layer Discretization
    html += tr(
                "<li>Layer Discretization"
                "<table border=\"0\">"
                "<tr><th>Maximum frequency:</th><td>%1 Hz</td></tr>"
                "<tr><th>Wavelength fraction:</th><td>%2</td></tr>"
                "</table>"
                "</li>"
                )
            .arg(m_siteProfile->maxFreq())
            .arg(m_siteProfile->waveFraction());

    // Calculator parameters
    html += m_calculator->toHtml();

    html += "</ol>";

    // Site profile
    html += "<li>" + m_siteProfile->toHtml() + "</li>";

    // Motions
    html += tr("<li>Motion(s)");

    QString loc;
    if (m_siteProfile->inputDepth() < 0)
        loc = "Bedrock";
    else
        loc = QString("%1 %2").arg(m_siteProfile->inputDepth()).arg(Units::instance()->length());

    html += tr("<table border=\"0\"><tr><th>Input Location:</th><td>%1</td></tr></table>")
            .arg(loc);

    html += m_motionLibrary->toHtml() + "</li>";

    // Close the html file
    html += "</ol></html>";

    return html;
}

QDataStream & operator<< (QDataStream & out, const SiteResponseModel* srm)
{
    out << (quint8)2;

    out << Units::instance()
        << srm->m_notes->toPlainText()
        << srm->m_method
        << srm->m_siteProfile
        << srm->m_motionLibrary
        << srm->m_outputCatalog
        << srm->m_randNumGen
        << srm->m_hasResults;

    switch (srm->m_method) {
    case SiteResponseModel::EquivalentLinear:
        out << qobject_cast<EquivalentLinearCalculator*>(srm->m_calculator);
        break;
    case SiteResponseModel::FrequencyDependent:
        out << qobject_cast<FrequencyDependentCalculator*>(srm->m_calculator);
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
       >> srm->m_siteProfile
       >> srm->m_motionLibrary // TODO Need to update motion count
       >> srm->m_outputCatalog;
    
    if (ver > 1) {
        in >> srm->m_randNumGen;
    }

    in >> hasResults;

    srm->m_notes->setPlainText(notes);
    srm->setMethod(method);
    srm->m_outputCatalog->initialize(
                srm->m_siteProfile->isVaried() ?  srm->m_siteProfile->profileCount() : 1,
                srm->m_motionLibrary);

    if (hasResults)
        srm->m_outputCatalog->finalize();

    switch (srm->m_method) {
    case SiteResponseModel::EquivalentLinear:
        in >> qobject_cast<EquivalentLinearCalculator*>(srm->m_calculator);
        break;
    case SiteResponseModel::FrequencyDependent:
        in >> qobject_cast<FrequencyDependentCalculator*>(srm->m_calculator);
        break;
    case SiteResponseModel::LinearElastic:
    default:
        break;
    }

    // Need to update the other objects that the model has data and should not be editted.
    srm->setHasResults(hasResults);

    return in;
}
