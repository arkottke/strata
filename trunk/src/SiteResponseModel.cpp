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
#include "SiteResponseOutput.h"
#include "Algorithms.h"
#include <QFile>
#include <QDataStream>
#include <QApplication>
#include <QDebug>

SiteResponseModel::SiteResponseModel(QObject * parent)
    : QObject(parent)
{
    m_units = new Units;
    m_siteProfile.setUnits(m_units);
    m_output = new SiteResponseOutput(m_units);
    m_rvtMotion = new RvtMotion;

    m_nlPropertyLibrary = new NonLinearPropertyLibrary;

    // Load the default values
    reset();
}

SiteResponseModel::~SiteResponseModel()
{
    delete m_units;
    delete m_output;
    delete m_rvtMotion;
    delete m_nlPropertyLibrary;
}
        
QStringList SiteResponseModel::methodList()
{
    QStringList list;
    list << tr("Recorded Motions") << tr("Random Vibration Theory");

    return list;
}

void SiteResponseModel::reset()
{
    m_title = "";
    m_notes = "";
    m_filePrefix = "";
    m_fileName = "";
    m_saveMotionData = true;
    m_okToContinue = true;

    m_units->reset();

	m_method = RecordedMotions;

    m_siteProfile.reset();

    m_calculator.reset();
  
    for ( int i = 0; i < m_recordedMotions.size(); ++i )
        delete m_recordedMotions.takeFirst();
    
    emit recordedMotionsChanged();

    m_rvtMotion->reset();

    m_output->reset();

    m_textLog.reset();
}

QString SiteResponseModel::fileName() const
{
    return m_fileName;
}

QString SiteResponseModel::title() const
{
	return m_title;
}

void SiteResponseModel::setTitle(const QString & title)
{
	m_title = title;
}

QString SiteResponseModel::notes() const
{
	return m_notes;
}

void SiteResponseModel::setNotes(const QString & notes)
{
	m_notes = notes;
}

SiteResponseModel::Method SiteResponseModel::method() const
{
	return m_method;
}

void SiteResponseModel::setMethod(Method method)
{
	m_method = method;
}
        
bool SiteResponseModel::saveMotionData() const
{
    return m_saveMotionData;
}
        
void SiteResponseModel::setSaveMotionData(bool saveData)
{
    m_saveMotionData = saveData;
}

bool SiteResponseModel::okToContinue() const
{
    return m_okToContinue;
}

void SiteResponseModel::setOkToContinue(bool okToContinue)
{
    m_okToContinue = okToContinue;
}

Units * SiteResponseModel::units()
{
    return m_units;
}

TextLog & SiteResponseModel::textLog()
{
    return m_textLog;
}

SiteProfile & SiteResponseModel::siteProfile()
{
	return m_siteProfile;
}

EquivLinearCalc & SiteResponseModel::calculator()
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

NonLinearPropertyLibrary * SiteResponseModel::nlPropertyLibrary()
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
    // Load the model from the map
    fromMap(map); 

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

    return true;
}
        
void SiteResponseModel::compute( QProgressBar * progressBar)
{
    m_textLog.clear();
    m_textLog << tr("<b>Starting Strata Calculation</b>");

    // Determine the number of sites to be used in the computation
    int siteCount;
    if (m_siteProfile.isSiteVaried())
        siteCount =  m_siteProfile.profileCount();
    else
        siteCount = 1;

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
            m_textLog << QString(tr("Computing the Fourier Amplitude Spectrum from the Response spectrum"));
            m_rvtMotion->invert( &m_okToContinue, progressBar);
        }

        motions << m_rvtMotion;
    }

    // Initialize the output
    m_output->initialize( m_method, siteCount, motions, m_siteProfile.soilTypes());

    // Setup the progress bar with the number of steps
    int totalCount = motions.size() * siteCount;
    progressBar->setMaximum(totalCount);
    progressBar->setMinimum(0);
    progressBar->setValue(0);

    if (m_okToContinue) 
        m_textLog << QString(tr("%1 Trials (%2 Sites and %3 Motions )"))
            .arg(totalCount)
            .arg(siteCount)
            .arg(motions.size());

    int count = 0;
    for (int i = 0; i < siteCount; ++i)
    {
        // Break if not okay to continue
        if ( !m_okToContinue )
            break;

        m_textLog << QString(tr("[%1 of %2] Generating site and soil properties")).arg(i+1).arg(siteCount);
        
        // Create the sublayers -- this randomizes the properties
        m_siteProfile.createSubLayers(m_textLog);

        // Save the profile to the output
        m_output->saveProfile(m_siteProfile);

        for (int j = 0; j < motions.size(); ++j ) {
            // Break if not okay to continue
            if ( !m_okToContinue )
                break;

            // Output status
            m_textLog << QString(tr("\t[%1 of %2] Computing site response for motion: %3"))
                .arg(j+1)
                .arg(motions.size())
                .arg(motions.at(j)->toString());

            // Compute the site response
            m_calculator.run( m_textLog, motions[j], &m_siteProfile);
            // Generate the output
            m_output->saveResults(m_calculator);

            // Increment the progress bar
            ++count;

            progressBar->setValue(count);

            // Reset the sublayers
            m_siteProfile.resetSubLayers();
        }
    }

    if (m_okToContinue)
        // Compute the statistics of the output
        m_output->computeStats();
    else 
        m_textLog << tr("<b>Canceled by the user.</b>");
}

QMap<QString, QVariant> SiteResponseModel::toMap() const
{
	QMap<QString, QVariant> map;

	map.insert("title", m_title);
	map.insert("notes", m_notes);
	map.insert("units", m_units->toMap());
	map.insert("textLog", m_textLog.toMap());
	map.insert("method", m_method);
    map.insert("saveMotionData", m_saveMotionData);

	map.insert("siteProfile",m_siteProfile.toMap());
	map.insert("calculator",m_calculator.toMap());
    
    map.insert("rvtMotion", m_rvtMotion->toMap());

	for ( int i = 0; i < m_recordedMotions.size(); ++i)
		map.insertMulti("recordedMotion", (m_recordedMotions.at(i)->toMap(m_saveMotionData)));

    // Output settings
    map.insert("output", m_output->toMap(false));

	return map;
}

void SiteResponseModel::fromMap(const QMap<QString, QVariant> & map )
{
	m_title = map.value("title").toString();
	m_notes = map.value("notes").toString();
	m_units->fromMap(map.value("units").toMap());
    m_textLog.fromMap(map.value("textLog").toMap());
	m_method = (Method)map.value("method").toInt();
    m_saveMotionData = map.value("saveMotionData").toBool();
    
	m_siteProfile.fromMap(map.value("siteProfile").toMap());
	m_calculator.fromMap(map.value("calculator").toMap());
   
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
        .arg(m_title)
        .arg(m_notes)
        .arg(m_filePrefix)
        .arg(m_units->systemList().at(m_units->system()));

  
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
        .arg(boolToString(m_siteProfile.isSiteVaried()));

    // Site Variation
    if ( m_siteProfile.isSiteVaried() )
        html += QString(tr(
                "<li>Site Property Variation"
                "<table border=\"0\">"
                "<tr><td><strong>Number of realizations:</strong></td><td>%1</td></tr>"
                "<tr><td><strong>Vary the non-linear soil properties:</strong></td><td>%2</td></tr>"
                "<tr><td><strong>Vary the site profile:</strong></td><td>%3</td></tr>"
                "</table>"
                "</li>"
                ))
            .arg(m_siteProfile.profileCount())
            .arg(boolToString(m_siteProfile.nonLinearPropertyVariation().enabled()))
            .arg(boolToString(m_siteProfile.profileVariation().enabled()));

    // Layer Discretization
    html += QString(tr(
                "<li>Layer Discretization"
                "<table border=\"0\">"
                "<tr><td><strong>Maximum frequency:</strong></td><td>%1 Hz</td></tr>"
                "<tr><td><strong>Wavelength fraction:</strong></td><td>%2</td></tr>"
                "</table>"
                "</li>"
                ))
        .arg(m_siteProfile.maxFreq())
        .arg(m_siteProfile.waveFraction());
   
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
        .arg(m_calculator.strainRatio())
        .arg(m_calculator.errorTolerance())
        .arg(m_calculator.maxIterations());
    
    html += "</ol>";

    // Site profile
    html += m_siteProfile.toHtml();

    // Motions
    html += tr("<li>Motion(s)");

    QString loc;
    if ( m_siteProfile.inputDepth() < 0 )
        loc = "Bedrock";
    else
        loc = QString("%1 %2").arg(m_siteProfile.inputDepth()).arg(m_units->length());

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
