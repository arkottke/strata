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

#include "EquivalentLinearCalculator.h"

#include "SoilProfile.h"
#include "SubLayer.h"
#include "TextLog.h"
#include "Units.h"

EquivalentLinearCalculator::EquivalentLinearCalculator(QObject *parent)
    : AbstractIterativeCalculator(parent)
{
    _name = "EQL";
    _strainRatio = 0.65;
}

auto EquivalentLinearCalculator::toHtml() const -> QString
{
    return tr(
            "<li>Equivalent Linear Parameters"
            "<table border=\"0\">"
            "<tr><th>Effective strain ratio:</th><td>%1 Hz</td></tr>"
            "<tr><th>Error tolerance:</th><td>%2</td></tr>"
            "<tr><th>Maximum number of iterations:</th><td>%3</td></tr>"
            "</table>"
            "</li>"
            )
            .arg(_strainRatio)
            .arg(_errorTolerance)
            .arg(_maxIterations);
}

auto EquivalentLinearCalculator::strainRatio() const -> double
{
    return _strainRatio;
}

void EquivalentLinearCalculator::setStrainRatio(double strainRatio)
{
    if (abs(_strainRatio - strainRatio) > 1E-2) {
        _strainRatio = strainRatio;

        emit strainRatioChanged(_strainRatio);
        emit wasModified();
    }
}

auto EquivalentLinearCalculator::updateSubLayer(
        int index,
        const QVector<std::complex<double> > &strainTf) -> bool
{
    const double strainMax = 100 * _motion->calcMaxStrain(strainTf);

    if (strainMax <= 0) {
        return false;
    }

    _site->subLayers()[index].setStrain(_strainRatio * strainMax, strainMax);

    // Compute the complex shear modulus and complex shear-wave velocity
    // for each soil layer -- these change because the damping and shear
    // modulus change.
    _shearMod[index].fill(calcCompShearMod(
            _site->subLayers().at(index).shearMod(),
            _site->subLayers().at(index).damping() / 100.));

    return true;
}

void EquivalentLinearCalculator::estimateInitialStrains()
{
    if ( _textLog->level() > TextLog::Low ) {
        _textLog->append(
                tr("\t\tEstimating strains using PGV and shear velocity."));
    }

    // Estimate the intial strain from the ratio of peak ground velocity of the
    //  motion and the shear-wave velocity of the layer.
    double estimatedStrain = 0;
    for (SubLayer &sl : _site->subLayers()) {
        estimatedStrain = _motion->pgv() / sl.shearVel();
        sl.setInitialStrain(estimatedStrain);
    }

    // Compute the complex shear modulus and complex shear-wave velocity for
    // each soil layer -- initially this is assumed to be frequency independent
    for (int i = 0; i < _nsl; ++i ) {
        _shearMod[i].fill(calcCompShearMod(
                _site->shearMod(i), _site->damping(i) / 100.));
    }
}

void EquivalentLinearCalculator::fromJson(const QJsonObject &json)
{
    AbstractIterativeCalculator::fromJson(json);
    _strainRatio = json["strainRatio"].toDouble();
}

auto EquivalentLinearCalculator::toJson() const -> QJsonObject
{
    QJsonObject json = AbstractIterativeCalculator ::toJson();
    json["strainRatio"] = _strainRatio;
    return json;
}

auto operator<< (QDataStream & out,
                                 const EquivalentLinearCalculator* elc) -> QDataStream &
{
    out << (quint8)1;
    out << qobject_cast<const AbstractIterativeCalculator*>(elc)
            << elc->_strainRatio;

    return out;
}

auto operator>> (QDataStream & in,
                                 EquivalentLinearCalculator* elc) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    in >> qobject_cast<AbstractIterativeCalculator*>(elc);
    in >> elc->_strainRatio;

    return in;
}
