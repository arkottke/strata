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
#include "SiteResponseModel.h"
#include "SiteResponseOutput.h"
#include "Units.h"

#include <QFile>
#include <QDataStream>
#include <QApplication>
#include <QTimer>

#include <QDebug>

SiteResponseModel::SiteResponseModel(QObject * parent)
    : QThread(parent)
{
    connect(Units::instance(), SIGNAL(systemChanged()), SLOT(setModified()));

    m_output = new SiteResponseOutput;
    connect(m_output, SIGNAL(wasModified()), SLOT(setModified()));
    
    m_calculator = new EquivLinearCalc;
    m_calculator->setTextLog(m_output->textLog());
    connect(m_calculator, SIGNAL(wasModified()), SLOT(setModified()));

    m_siteProfile = new SiteProfile;
    connect( m_siteProfile, SIGNAL(wasModified()), SLOT(setModified()));

    m_notes = new QTextDocument;
    connect( m_notes, SIGNAL(contentsChanged()), SLOT(setModified()));

    m_rvtMotion = new RvtMotion;
    connect(m_rvtMotion, SIGNAL(wasModified()), SLOT(setModified()));

    m_nlPropertyLibrary = new NonlinearPropertyLibrary;

    // Load the default values
    reset();
}

SiteResponseModel::~SiteResponseModel()
{
    delete m_output;
    delete m_rvtMotion;
    delete m_nlPropertyLibrary;
    delete m_notes;
    delete m_calculator;
    delete m_output;
}
        
QStringList SiteResponseModel::methodList()
{
    QStringList list;
    list << tr("Recorded Motions") << tr("Random Vibration Theory");

    return list;
}

void SiteResponseModel::reset()
{
    m_isLoaded = false;
    m_modified = false;
    m_notes->clear();
    m_fileName = "";
    m_saveMotionData = true;
    m_okToContinue = true;

    Units::instance()->reset();

	m_method = RecordedMotions;

    m_siteProfile->reset();

    m_calculator->reset();
  
    for ( int i = 0; i < m_recordedMotions.size(); ++i )
        delete m_recordedMotions.takeFirst();
    
    emit recordedMotionsChanged();

    m_rvtMotion->reset();
    m_output->reset();

    // Wait until all events have been processed and then reset the modified flag
    QTimer::singleShot(10, this, SLOT(setIsLoaded()));
}

bool SiteResponseModel::readOnly() const
{
    return m_readOnly;
}

void SiteResponseModel::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

QString SiteResponseModel::fileName() const
{
    return m_fileName;
}

QTextDocument * SiteResponseModel::notes() const
{
	return m_notes;
}

SiteResponseModel::Method SiteResponseModel::method() const
{
	return m_method;
}

void SiteResponseModel::setMethod(Method method)
{
    if ( m_method != method ) {
        setModified(true);
    }

    m_method = method;
}

void SiteResponseModel::setMethod(int method)
{
    setMethod((Method)method);
}
        
bool SiteResponseModel::saveMotionData() const
{
    return m_saveMotionData;
}
        
void SiteResponseModel::setSaveMotionData(bool saveData)
{
    m_saveMotionData = saveData;
    setModified(true);
}

void SiteResponseModel::stop()
{
    m_okToContinue = false;
    m_calculator->stop();
}

bool SiteResponseModel::modified() const
{
    return m_modified;
}

void SiteResponseModel::setModified( bool modified)
{
    // FIXME When opening a previously saved document, the QTextDocument
    // containing the notes signals that it has been modified.  This happens
    // before the modified flag is reset.
    if (m_isLoaded) {
        m_modified = modified;
        emit modifiedChanged(modified);
    }
}

void SiteResponseModel::setIsLoaded(bool isLoaded)
{
    m_isLoaded = isLoaded;
}

bool SiteResponseModel::okToContinue() const
{
    return m_okToContinue;
}

bool SiteResponseModel::wasSucessful() const
{
    return m_wasSuccessful;
}

void SiteResponseModel::setOkToContinue(bool okToContinue)
{
    m_okToContinue = okToContinue;
}

SiteProfile * SiteResponseModel::siteProfile()
{
	return m_siteProfile;
}

EquivLinearCalc * SiteResponseModel::calculator()
{
	return m_calculator;
}

QList<RecordedMotion*> & SiteResponseModel::recordedMotions()
{
    return m_recordedMotions;
}

RvtMotion * SiteResponseModel::rvtMotion()
{
    return m_rvtMotion;
}

NonlinearPropertyLibrary * SiteResponseModel::nlPropertyLibrary()
{
    return m_nlPropertyLibrary;
}

SiteResponseOutput * SiteResponseModel::output()
{
    return m_output;
}

bool SiteResponseModel::load(const QString & fileName)
{
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
    // Read the map from the file 
    QMap<QString,QVariant> map;
    inStream >> map;

    setModified(false);
    m_isLoaded = false;
    // Load the model from the map
    fromMap(map); 

    // Wait until all events have been processed and then reset the modified flag
    QTimer::singleShot(10, this, SLOT(setIsLoaded()));
   
    return true;
}

bool SiteResponseModel::save(const QString & fileName)
{
    // Save the file name
    m_fileName = fileName;

    QFile file(fileName);
    // If the file can't be opened halt 
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical("Error opening file: %s", qPrintable(fileName));
        return false;
    }
    // Open the data stream
    QDataStream outStream(&file);
    // Create a map of the model and save the map
    outStream << toMap();

    m_modified = false;
    return true;
}
        
void SiteResponseModel::run()
{
    emit isWorkingChanged(true);

    m_okToContinue = true;
    m_output->textLog()->clear();
    m_output->textLog()->append(tr("<b>Starting Strata Calculation</b>"));

    // Determine the number of sites to be used in the computation
    int siteCount;
    if (m_siteProfile->isSiteVaried()) {
        siteCount =  m_siteProfile->profileCount();
    } else {
        siteCount = 1;
    }

    // Create a list of the motions used in the analysis
    QList<Motion*> motions;
    
    if (m_method == RecordedMotions )
        for (int i = 0; i < m_recordedMotions.size(); ++i) {
            if ( m_recordedMotions.at(i)->isEnabled() )
                motions << m_recordedMotions[i];
        }
    else if ( m_method == RandomVibrationTheory ){
        // Invert the motion
        if ( m_rvtMotion->source() == RvtMotion::DefinedResponseSpectrum ) {
            m_output->textLog()->append(tr("Calculation the Fourier Amplitude Spectrum from the Response spectrum"));
            m_rvtMotion->invert();
        } else if ( m_rvtMotion->source() == RvtMotion::CalculatedFourierSpectrum ) {
            m_output->textLog()->append(tr("Calculation the Fourier Amplitude Spectrum from the Point Source Model"));
            m_rvtMotion->calcPointSource();
        }
        motions << m_rvtMotion;
    }

    // Initialize the output
    m_output->initialize( m_method, siteCount, motions, m_siteProfile->soilTypes());

    // Setup the progress bar with the number of steps
    int totalCount = motions.size() * siteCount;
    emit progressRangeChanged( 0, totalCount );
    emit progressChanged(0);

    if (m_okToContinue) {
        m_output->textLog()->append(QString(tr("%1 Trials (%2 Sites and %3 Motions )"))
            .arg(totalCount)
            .arg(siteCount)
            .arg(motions.size()));
    }

    int count = 0;
    for (int i = 0; i < siteCount; ++i) {
        // Break if not okay to continue
        if ( !m_okToContinue )
            break;

        m_output->textLog()->append((QString(tr("[%1 of %2] Generating site and soil properties")).arg(i+1).arg(siteCount)));
        
        // Create the sublayers -- this randomizes the properties
        m_siteProfile->createSubLayers(m_output->textLog());

        // Save the profile to the output
        m_output->saveProfile(m_siteProfile);

        // FIXME -- check the site profile to ensure that the waves can be computed for the intial coniditions

        for (int j = 0; j < motions.size(); ++j ) {
            // Break if not okay to continue
            if ( !m_okToContinue ) {
                break;
            }

            // Output status
            m_output->textLog()->append(QString(tr("\t[%1 of %2] Computing site response for motion: %3"))
                .arg(j+1)
                .arg(motions.size())
                .arg(motions.at(j)->toString()));

            // Compute the site response
            if (!m_calculator->run( motions[j], m_siteProfile) && m_okToContinue) {
                m_output->textLog()->append(tr("\tWave propagation error -- removing site."));
                // Error in the calculation -- need to remove the site
                m_output->removeLastSite();
                // Reset site count
                --i;
                break;
            }

            // Generate the output
            m_output->saveResults(m_calculator);

            // Increment the progress bar
            ++count;
            emit progressChanged(count);

            // Reset the sublayers
            m_siteProfile->resetSubLayers();
        }
    }

    if (m_okToContinue) {
        // Compute the statistics of the output
        m_output->computeStats();
        m_wasSuccessful = true;
    } else {
        m_output->textLog()->append(tr("<b>Canceled by the user.</b>"));
        m_wasSuccessful = false;
        m_output->clear();
    }
    
    emit isWorkingChanged(false);
}

QMap<QString, QVariant> SiteResponseModel::toMap() const
{
	QMap<QString, QVariant> map;

    disconnect( m_notes, 0, this, 0 );
	map.insert("notes", m_notes->toHtml());
    connect( m_notes, SIGNAL(contentsChanged()), SLOT(setModified()));


	map.insert("units", Units::instance()->toMap());
	map.insert("method", m_method);
    map.insert("saveMotionData", m_saveMotionData);

	map.insert("siteProfile",m_siteProfile->toMap());
	map.insert("calculator",m_calculator->toMap());
    
    map.insert("rvtMotion", m_rvtMotion->toMap());

	for ( int i = 0; i < m_recordedMotions.size(); ++i)
		map.insertMulti("recordedMotion", (m_recordedMotions.at(i)->toMap(m_saveMotionData)));

    // Output settings
    map.insert("output", m_output->toMap());

	return map;
}

void SiteResponseModel::fromMap(const QMap<QString, QVariant> & map )
{
	m_notes->setHtml(map.value("notes").toString());
	Units::instance()->fromMap(map.value("units").toMap());
	m_method = (Method)map.value("method").toInt();
    m_saveMotionData = map.value("saveMotionData").toBool();
    
	m_siteProfile->fromMap(map.value("siteProfile").toMap());
	m_calculator->fromMap(map.value("calculator").toMap());
   
    m_rvtMotion->fromMap(map.value("rvtMotion").toMap());
    emit rvtMotionChanged();

    // Recorded motions
    m_recordedMotions.clear();
    QList<QVariant> recordedMotions = map.values("recordedMotion");
	for (int i = 0; i < recordedMotions.size(); ++i) {
        // Values are stored last first
		m_recordedMotions.prepend( new RecordedMotion );
		m_recordedMotions.first()->fromMap(recordedMotions.at(i).toMap());
	}
    emit recordedMotionsChanged();

    // Output settings
    m_output->fromMap(map.value("output").toMap());
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
    html += QString(tr("<h1>%1</h1>"
                "<ol>"
                "<li>General Settings"
                "<ol>"
                "<li>Project"
                "<table border=\"0\">"
                "<tr><td><strong>Title:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Notes:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>File preffix:</strong></td><td>%3</td></tr>"
                "<tr><td><strong>Units System:</strong></td><td>%4</td></tr>"
                "</table>"
                "</li>"
                ))
        .arg(m_output->title())
        .arg(m_notes->toHtml())
        .arg(m_output->filePrefix())
        .arg(Units::instance()->systemList().at(Units::instance()->system()));

  
    // Type of Analysis
    html += QString(tr(
                "<li>Type of Analysis"
                "<table border=\"0\">"
                "<tr><td><strong>Analysis Method:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Properties Varied:</strong></td><td>%2</td></tr>"
                "</table>"
                "</li>"
                ))
        .arg(methodList().at(m_method))
        .arg(boolToString(m_siteProfile->isSiteVaried()));

    // Site Variation
    if ( m_siteProfile->isSiteVaried() )
        html += QString(tr(
                "<li>Site Property Variation"
                "<table border=\"0\">"
                "<tr><td><strong>Number of realizations:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Vary the nonlinear soil properties:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Vary the site profile:</strong></td><td>%3</td></tr>"
                "</table>"
                "</li>"
                ))
            .arg(m_siteProfile->profileCount())
            .arg(boolToString(m_siteProfile->nonLinearPropertyVariation()->enabled()))
            .arg(boolToString(m_siteProfile->profileVariation()->enabled()));

    // Layer Discretization
    html += QString(tr(
                "<li>Layer Discretization"
                "<table border=\"0\">"
                "<tr><td><strong>Maximum frequency:</strong></td><td>%1 Hz</td></tr>"
                "<tr><td><strong>Wavelength fraction:</strong></td><td>%2</td></tr>"
                "</table>"
                "</li>"
                ))
        .arg(m_siteProfile->maxFreq())
        .arg(m_siteProfile->waveFraction());
   
    // Equivalent Linear Parameters
    html += QString(tr(
                "<li>Equivalent Linear Parameters"
                "<table border=\"0\">"
                "<tr><td><strong>Effective strain ratio:</strong></td><td>%1 Hz</td></tr>"
                "<tr><td><strong>Error tolerance:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Maximum number of iterations:</strong></td><td>%3</td></tr>"
                "</table>"
                "</li>"
                ))
        .arg(m_calculator->strainRatio())
        .arg(m_calculator->errorTolerance())
        .arg(m_calculator->maxIterations());
    
    html += "</ol>";

    // Site profile
    html += m_siteProfile->toHtml();

    // Motions
    html += tr("<li>Motion(s)");

    QString loc;
    if ( m_siteProfile->inputDepth() < 0 )
        loc = "Bedrock";
    else
        loc = QString("%1 %2").arg(m_siteProfile->inputDepth()).arg(Units::instance()->length());

    html += QString(tr("<table border=\"0\"><tr><td><strong>Input Location:</strong></td><td>%1</td></tr></table>"))
        .arg(loc);

    if ( m_method == RecordedMotions ) {
        html += tr(
                "<table border=\"1\">"
                "<tr>"
                "<th>Filename</th>"
                "<th>Description</th>"
                "<th>Type</th>"
                "<th>Scale Factor</th>"
                "<th>PGA (g)</th>"
                "</tr>");

        for ( int i = 0; i < m_recordedMotions.size(); ++i )
            html += m_recordedMotions.at(i)->toHtml();

        html += "</table>";

    } else if ( m_method == RandomVibrationTheory ) {
        html += m_rvtMotion->toHtml();
    }
    html += "</li>";
    // Close the html file
    html += "</ol></html>";

    return html;
}
