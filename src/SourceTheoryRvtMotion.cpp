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
// Copyright 2010 Albert Kottke
//
////////////////////////////////////////////////////////////////////////////////

#include "SourceTheoryRvtMotion.h"

#include "CrustalAmplification.h"
#include "Dimension.h"

#include <QDebug>

SourceTheoryRvtMotion::SourceTheoryRvtMotion(QObject * parent)
    : AbstractRvtMotion(parent)
{
    m_freq = new Dimension(this);
    m_freq->setMin(0.05);
    m_freq->setMax(50);
    m_freq->setSize(1024);
    m_freq->setSpacing(Dimension::Log);

    m_fourierAcc = QVector<double>(freqCount(), 0.);

    m_crustalAmp = new CrustalAmplification;

    setDistance(20.);
    setDepth(8.);
    setMomentMag(6.5);
    setModel(WUS);

    m_name = tr("Source Theory (M= $mag, R= $dist km)");
}

SourceTheoryRvtMotion::~SourceTheoryRvtMotion()
{
    m_crustalAmp->deleteLater();
}

QStringList SourceTheoryRvtMotion::sourceList()
{
    return QStringList() << tr("Custom") << tr("Western NA") << tr("Eastern NA");
}

const QVector<double> & SourceTheoryRvtMotion::freq() const
{
    return m_freq->data();
}

Dimension*  SourceTheoryRvtMotion::freqDimension()
{
    return m_freq;
}

QString SourceTheoryRvtMotion::nameTemplate() const
{
    return m_name;
}

QString SourceTheoryRvtMotion::name() const
{
    QString s = m_name;

    return s
            .replace("$mag",
                     QString::number(m_momentMag, 'f', 1),
                     Qt::CaseInsensitive)
            .replace("$dist",
                     QString::number(m_distance, 'f', 1),
                     Qt::CaseInsensitive);
}

QString SourceTheoryRvtMotion::toHtml() const
{
    //FIXME

    return QString();
}

bool SourceTheoryRvtMotion::loadFromTextStream(QTextStream &stream)
{
    // FIXME

    return false;
}

SourceTheoryRvtMotion::Model SourceTheoryRvtMotion::model() const
{
    return m_model;
}

void SourceTheoryRvtMotion::setModel(Model s)
{
    m_model = s;
    if (m_model != Custom)
        m_crustalAmp->setModel(s);

    switch (m_model) {
    case WUS:
        setStressDrop(100);
        setPathAttenCoeff(180);
        setPathAttenPower(0.45);
        setShearVelocity(3.5);
        setDensity(2.8);
        setSiteAtten(0.04);
        break;
    case CEUS:
        setStressDrop(150);
        setPathAttenCoeff(680);
        setPathAttenPower(0.36);
        setShearVelocity(3.6);
        setDensity(2.8);
        setSiteAtten(0.006);
        break;
    case Custom:
        // Do nothing
        break;
    }
    // Geometric attenuation may have changed
    calcGeoAtten();

    emit isCustomizeable(m_model == Custom);
}

void SourceTheoryRvtMotion::setModel(int s)
{
    setModel((Model)s);
}

//QString SourceTheoryRvtMotion::toHtml() const
//{
//    QString html = QString(tr(
//                "<tr><th>Location:</th><td>%1</td></tr>"
//                "<tr><th>Moment Magnitude:</th><td>%2</td></tr>"
//                "<tr><th>Distance:</th><td>%3 km</td></tr>"
//                "<tr><th>Depth:</th><td>%4 km </td></tr>"
//                "<tr><th>Stress Drop:</th><td>%5 bars</td></tr>"
//                "<tr><th>Geometric Attenuation:</th><td>%6</td></tr>"
//                "<tr><th>Path Atten Coeff.:</th><td>%7</td></tr>"
//                "<tr><th>Path Atten Power.:</th><td>%8</td></tr>"
//                "<tr><th>Shear-wave velocity:</th><td>%9 km/s</td></tr>"
//                "<tr><th>Density:</th><td>%10 gm/cm%11</td></tr>"
//                "<tr><th>Site Attenuation (kappa):</th><td>%12</td></tr>"
//                "<tr><th>Generic Crustal Amplication:</th><td>%13</td></tr>"
//                "</table>"
//               ))
//        .arg(locationList().at(m_location))
//        .arg(m_momentMag)
//        .arg(m_distance)
//        .arg(m_depth)
//        .arg(m_stressDrop)
//        .arg(m_geoAtten)
//        .arg(m_pathAttenCoeff)
//        .arg(m_pathAttenPower)
//        .arg(m_shearVelocity)
//        .arg(m_density).arg(QChar(0x00B3))
//        .arg(m_CrustalAtten)
//        .arg(m_siteSpecificCrustalAmp ? tr("yes") : tr("no"));
//
//    html += "<table><tr>";
//
//    if (m_siteSpecificCrustalAmp) {
//        // Add the velocity profile
//        html += "<td><h4>Velocity Profile</h4><table border = \"1\">";
//        html += QString("<tr><th>Thickness (km)</th><th>Shear Velocity (km/s)</th><th>Density (gm/cm%1)</th></tr>").arg(QChar(0x00B3));
//
//        for (int i = 0; i < m_crustThickness.size(); ++i) {
//            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
//                .arg(m_crustThickness.at(i))
//                .arg(m_crustVelocity.at(i))
//                .arg(m_crustDensity.at(i));
//        }
//
//        html += "</table></td>";
//
//        // Add the crustal amplification
//        // FIXME
////        if (m_crustAmpNeedsUpdate) {
////            calcCrustalAmp();
////        }
//    }
//
//    html += "<td><h4>Crustal Amplification</h4><table border = \"1\">";
//    html += "<tr><th>Frequency (Hz)</th><th>Amplification</th></tr>";
//
//    for (int i = 0; i < m_freq.size(); ++i) {
//        html += QString("<tr><td>%1</td><td>%2</td></tr>")
//            .arg(freqAt(i))
//            .arg(m_crustAmp.at(i));
//    }
//    html += "</table></td></tr></table>";
//
//    return html;
//}

double SourceTheoryRvtMotion::momentMag() const
{
    return m_momentMag;
}

void SourceTheoryRvtMotion::setMomentMag(double momentMag)
{    
    if (m_momentMag != momentMag) {
        m_momentMag = momentMag;

        // Compute seismic moment based on the moment magnitude
        m_seismicMoment = pow(10, 1.5 * (m_momentMag + 10.7));

        emit momentMagChanged(m_momentMag);
        calcCornerFreq();
    }
}

double SourceTheoryRvtMotion::distance() const
{
    return m_distance;
}

void SourceTheoryRvtMotion::setDistance(double distance)
{
    m_distance = distance;

    calcHypoDistance();
}

double SourceTheoryRvtMotion::depth() const
{
    return m_depth;
}

void SourceTheoryRvtMotion::setDepth(double depth)
{
    m_depth = depth;

    calcHypoDistance();
}

double SourceTheoryRvtMotion::stressDrop() const
{
    return m_stressDrop;
}

void SourceTheoryRvtMotion::setStressDrop(double stressDrop)
{
    if (m_stressDrop != stressDrop) {
        m_stressDrop = stressDrop;

        emit stressDropChanged(stressDrop);
        calcCornerFreq();
    }
}

double SourceTheoryRvtMotion::pathDurCoeff() const
{
    return m_pathDurCoeff;
}

void SourceTheoryRvtMotion::setPathDurCoeff(double pathDurCoeff)
{
    if (m_pathDurCoeff != pathDurCoeff) {
        m_pathDurCoeff = pathDurCoeff;

        emit pathDurCoeffChanged(pathDurCoeff);
        calcDuration();
    }
}

double SourceTheoryRvtMotion::geoAtten() const
{
    return m_geoAtten;
}

void SourceTheoryRvtMotion::setGeoAtten(double geoAtten)
{
    if (m_geoAtten != geoAtten) {
        m_geoAtten = geoAtten;

        emit geoAttenChanged(geoAtten);
    }
}

double SourceTheoryRvtMotion::pathAttenCoeff() const
{
    return m_pathAttenCoeff;
}

void SourceTheoryRvtMotion::setPathAttenCoeff(double pathAttenCoeff)
{
    if (m_pathAttenCoeff != pathAttenCoeff) {
        m_pathAttenCoeff = pathAttenCoeff;

        emit pathAttenCoeffChanged(pathAttenCoeff);
    }
}

double SourceTheoryRvtMotion::pathAttenPower() const
{
    return m_pathAttenPower;
}

void SourceTheoryRvtMotion::setPathAttenPower(double pathAttenPower)
{
    if (m_pathAttenPower != pathAttenPower) {
        m_pathAttenPower = pathAttenPower;

        emit pathAttenPowerChanged(pathAttenPower);
    }
}

double SourceTheoryRvtMotion::shearVelocity() const
{
    return m_shearVelocity;
}

void SourceTheoryRvtMotion::setShearVelocity(double shearVelocity)
{
    if (m_shearVelocity != shearVelocity) {
        m_shearVelocity = shearVelocity;

        emit shearVelocityChanged(shearVelocity);
        calcCornerFreq();
    }
}

double SourceTheoryRvtMotion::density() const
{
    return m_density;
}

void SourceTheoryRvtMotion::setDensity(double density)
{
    if (m_density != density) {
        m_density = density;

        emit densityChanged(density);
    }
}

double SourceTheoryRvtMotion::siteAtten() const
{
    return m_siteAtten;
}

double SourceTheoryRvtMotion::duration() const
{
    return m_duration;
}

void SourceTheoryRvtMotion::setSiteAtten(double siteAtten)
{
    if (m_siteAtten != siteAtten) {
        m_siteAtten = siteAtten;

        emit siteAttenChanged(siteAtten);
    }
}

CrustalAmplification* SourceTheoryRvtMotion::crustalAmp()
{
    return m_crustalAmp;
}

void SourceTheoryRvtMotion::calcHypoDistance()
{
    if (m_depth > 0 && m_distance > 0) {
        m_hypoDistance = sqrt(m_depth * m_depth + m_distance * m_distance);

        calcDuration();
        calcGeoAtten();
    }
}

void SourceTheoryRvtMotion::calcCornerFreq()
{
    if (m_shearVelocity > 0 && m_stressDrop > 0 && m_seismicMoment > 0) {
        m_cornerFreq = 4.9e6 * m_shearVelocity * pow(m_stressDrop/m_seismicMoment, 1./3.);

        calcDuration();
    }
}

void SourceTheoryRvtMotion::calcDuration()
{
    if (m_cornerFreq > 0) {
        // Compute source component
        const double sourceDur = 1 / m_cornerFreq;

        // Offset distance
        double offsetDist = 0;
        // Duration at offset
        double offsetDur = 0;

        switch (m_model)
        {
        case Custom:
            // Do nothing
            break;
            // Values proposed by Campbell 2003
        case WUS:
            m_pathDurCoeff = 0.05;
            break;
        case CEUS:
            // The duration is assumed to be piece-wise linear with the equal
            // values at the intersections.
            if (m_hypoDistance <= 10.) {
                m_pathDurCoeff = 0;

                offsetDur = 0;
                offsetDist = 0;
            } else if (m_hypoDistance <= 70.) {
                // slope of 0.16 during this segment
                m_pathDurCoeff = 0.16;

                offsetDur = 0;
                offsetDist = 10.;
            } else if (m_hypoDistance <= 130.) {
                m_pathDurCoeff = -0.03;

                offsetDur = 0.16 * (70.-10.);
                offsetDist = 70.;
            } else {
                m_pathDurCoeff = 0.04;

                offsetDur = 0.16 * (70.-10.) - 0.03 * (130.-70.);
                offsetDist = 130.;
            }
            break;
        }
        emit pathDurCoeffChanged(m_pathDurCoeff);

        // Compute the path duration component
        double pathDur = offsetDur + m_pathDurCoeff * (m_hypoDistance - offsetDist);

        m_duration = sourceDur + pathDur;

        emit durationChanged(m_duration);
    }
}

void SourceTheoryRvtMotion::calcGeoAtten()
{
    if (m_hypoDistance > 0) {
        // Determine the geometric attenuation based on a piecewise linear
        // calculation
        switch (m_model) {
        case Custom:
            // Do nothing
            break;
        case WUS:
            if (m_hypoDistance < 40.)
                m_geoAtten = 1. / m_hypoDistance;
            else
                m_geoAtten = 1./40. * sqrt(40./m_hypoDistance);
            break;
        case CEUS:
            if (m_hypoDistance < 70.)
                m_geoAtten = 1. / m_hypoDistance;
            else if (m_hypoDistance < 130.)
                m_geoAtten = 1. / 70.;
            else
                m_geoAtten = 1./70. * sqrt(130./m_hypoDistance);
            break;
        }

        emit geoAttenChanged(m_geoAtten);
    }
}


void SourceTheoryRvtMotion::calculate()
{
    // Conversion factor to convert from dyne-cm into gravity-sec
    // const double conv = 1e-20 / 981;
    const double conv = 1e-18 / 981.;

    // Constant term for the model component
    const double C = (0.55 * 2) / (M_SQRT2 * 4 * M_PI * m_density * pow(m_shearVelocity, 3));

    for (int i = 0; i < m_fourierAcc.size(); ++i) {
        // Model component
        const double sourceComp =  C * m_seismicMoment / (1 + pow(freqAt(i)/m_cornerFreq, 2));

        // Path component
        const double pathAtten = m_pathAttenCoeff * pow(freqAt(i), m_pathAttenPower);
        const double pathComp = m_geoAtten *
                                exp((-M_PI * freqAt(i) * m_hypoDistance) / (pathAtten * m_shearVelocity));

        // Site component
        const double siteAmp = m_crustalAmp->interpAmpAt(freqAt(i));
        const double siteDim = exp(-M_PI * m_siteAtten * freqAt(i));
        const double siteComp = siteAmp * siteDim;

        // Combine the three components and convert from displacement to
        // acceleleration
        m_fourierAcc[i] = conv * pow(M_2_PI * freqAt(i), 2) * sourceComp * pathComp * siteComp;
    }

    dataChanged(index(0, AmplitudeColumn), index(m_fourierAcc.size(), AmplitudeColumn));

    AbstractRvtMotion::calculate();
}


QDataStream & operator<< (QDataStream & out, const SourceTheoryRvtMotion* strm)
{
    out << (quint8)1;

    out << qobject_cast<const AbstractRvtMotion*>(strm);

    // Properties of SourceTheoryRvtMotion
    out << (int)strm->m_model
            << strm->m_momentMag
            << strm->m_distance
            << strm->m_depth
            << strm->m_freq;

    if (strm->m_model == SourceTheoryRvtMotion::Custom) {
        out
               << strm->m_stressDrop
               << strm->m_geoAtten
               << strm->m_pathDurCoeff
               << strm->m_pathAttenCoeff
               << strm->m_pathAttenPower
               << strm->m_shearVelocity
               << strm->m_density
               << strm->m_siteAtten
               << strm->m_crustalAmp;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, SourceTheoryRvtMotion* strm)
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractRvtMotion*>(strm);

    // Properties of SourceTheoryRvtMotion
    int model;
    double momentMag;
    double distance;
    double depth;

    in >> model
            >> momentMag
            >> distance
            >> depth
            >> strm->m_freq;

    strm->setMomentMag(momentMag);
    strm->setDistance(distance);
    strm->setDepth(depth);
    strm->setModel(model);

    if (strm->m_model == SourceTheoryRvtMotion::Custom) {
        double stressDrop;
        double geoAtten;
        double pathDurCoeff;
        double pathAttenCoeff;
        double pathAttenPower;
        double shearVelocity;
        double density;
        double siteAtten;

        in >> stressDrop
                >> geoAtten
                >> pathDurCoeff
                >> pathAttenCoeff
                >> pathAttenPower
                >> shearVelocity
                >> density
                >> siteAtten
                >> strm->m_crustalAmp;

        // Use set methods to calculate dependent parameters
        strm->setStressDrop(stressDrop);
        strm->setGeoAtten(geoAtten);
        strm->setPathDurCoeff(pathDurCoeff);
        strm->setPathAttenCoeff(pathAttenCoeff);
        strm->setPathAttenPower(pathAttenPower);
        strm->setShearVelocity(shearVelocity);
        strm->setDensity(density);
        strm->setSiteAtten(siteAtten);
    }

    // Compute the FAS
    strm->calculate();

    return in;

}
