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

#include "NonlinearProperty.h"

#include "Dimension.h"
#include "SoilType.h"
#include "Serialize.h"

#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

NonlinearProperty::NonlinearProperty(QObject *parent)
    : QAbstractTableModel(parent)
{
    _interp = nullptr;
    _acc = gsl_interp_accel_alloc();
}

NonlinearProperty::NonlinearProperty(
        const QString &name, Type type, const QVector<double> &strain,
        const QVector<double> &property, QObject *parent)
            : QAbstractTableModel(parent), _name(name), _type(type), _strain(strain),
                                         _average(property), _varied(property)
{
    _interp = nullptr;
    _acc = gsl_interp_accel_alloc();
}

NonlinearProperty::~NonlinearProperty()
{
    gsl_interp_accel_free(_acc);

    if (_interp) {
        gsl_interp_free(_interp);
        _interp = nullptr;
    }
}

auto NonlinearProperty::type() const -> NonlinearProperty::Type
{
    return _type;
}

auto NonlinearProperty::name() const -> const QString &
{
    return _name;
}

auto NonlinearProperty::interp(const double strain) -> double
{
    double d;
    if (strain < _strain.first()) {
        d = _varied.first();
    } else if (strain > _strain.last()) {
        d = _varied.last();
    } else if (_strain.size() == 1) {
        // Interpolater won't be intialized
        d = _varied.first();
    } else {
        if (!_interp)
            initialize();
        d = gsl_interp_eval(_interp, _strain.data(), _varied.data(), strain, _acc);
    }

    return d;
}

auto NonlinearProperty::toHtml() const -> QString
{  
    QString html = tr(
            "<h4>%1</h4>"
            "<table border = \"0\">"
            "<tr><th>Type:</th><td>%1</td></tr>"
            "<tr><th>Name:</th><td>%2</td></tr>"
            "</table>")
            .arg(_type == ModulusReduction ?
                 tr("Shear Modulus Reduction") : tr("Damping Ratio"))
            .arg(_name);


    html += "<table border=\"1\"><tr>";

    // Header
    for (int i = 0; i < columnCount(); ++i)
        html += "<th>" + headerData(i, Qt::Horizontal).toString() + "</th>";

    html += "</tr>";

    // Data
    for (int r = 0; r < rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < columnCount(); ++c)
            html += "<td>" + data(index(r, c)).toString() + "</td>";

        html += "</tr>";
    }
    html += "</table>";

    return html;
}

auto NonlinearProperty::rowCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent);

    return _strain.size();
}

auto NonlinearProperty::columnCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent);

    return 2;
}

auto NonlinearProperty::data(const QModelIndex &index, int role) const -> QVariant
{
    if (index.parent()!=QModelIndex())
        return QVariant();

    if (role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column()) {
        case StrainColumn:
            return QString::number(_strain.at(index.row()), 'e', 2);
        case PropertyColumn:
            return QString::number(_average.at(index.row()), 'f', 3);
        }
    } else if (role == Qt::UserRole) {
        switch (index.column()) {
        case StrainColumn:
            return _strain.at(index.row());
        case PropertyColumn:
            return _average.at(index.row());
        }
    }

    return QVariant();
}

auto NonlinearProperty::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (role != Qt::DisplayRole)
        return QVariant();

    switch (orientation) {
    case Qt::Horizontal:
        switch (section) {
        case StrainColumn:
            return tr("Strain (%)");
        case PropertyColumn:
            switch (_type) {
            case ModulusReduction:
                return tr("G/Gmax");
            case Damping:
                return tr("Damping (%)");
            }
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}


auto NonlinearProperty::strain() const -> const QVector<double> &
{
    return _strain;
}

auto NonlinearProperty::average() const -> const QVector<double> &
{
    return _average;
}

auto NonlinearProperty::varied() const -> const QVector<double> &
{    
    return _varied;
}

void NonlinearProperty::setVaried(const QVector<double> &varied)
{
    _varied = varied;

    // Need to ensure that the strains are constantly increasing. This is required for the GSL interpolation
    for (int i = 0; i < _strain.size(); ++i) {
        int j = i + 1;

        while (j < _strain.size()) {
            if (_strain.at(i) >= _strain.at(j)) {
                _strain.remove(j);
                _varied.remove(j);
            } else {
                ++j;
            }
        }
    }

    // Compute log strains
    _lnStrain.clear();
    for (double s : _strain) {
        _lnStrain << log(s);
    }

    // Free the interpolator and reset the pointer to zero
    if (_interp) {
        gsl_interp_free(_interp);
        _interp = nullptr;
    }
}

void NonlinearProperty::initialize()
{
    if (_strain.size() > 2) {
        if (_interp)
            gsl_interp_free(_interp);
        _interp = gsl_interp_alloc(gsl_interp_cspline, _strain.size());
        gsl_interp_init(_interp, _strain.data(), _varied.data(), _strain.size());
        gsl_interp_accel_reset(_acc);
    }
}

auto NonlinearProperty::duplicate() const -> NonlinearProperty *
{
    return new NonlinearProperty(_name, _type, _strain, _average);
}

void NonlinearProperty::fromJson(const QJsonObject &json)
{
    beginResetModel();

    _name = json["name"].toString();
    _type = (NonlinearProperty::Type) json["type"].toInt();

    Serialize::toDoubleVector(json["strain"], _strain);
    Serialize::toDoubleVector(json["average"], _average);

    setVaried(_average);
    endResetModel();
}

auto NonlinearProperty::toJson() const -> QJsonObject
{
    QJsonObject json;
    json["name"] = _name;
    json["type"] = (int) _type;
    json["strain"] = Serialize::toJsonArray(_strain);
    json["average"] = Serialize::toJsonArray(_average);

    return json;
}

auto operator<< (QDataStream & out, const NonlinearProperty* np) -> QDataStream &
{
    out << (quint8)1;

    out << np->_name
            << (int)np->_type
            << np->_strain
            << np->_average;

    return out;
}

auto operator>> (QDataStream & in, NonlinearProperty* np) -> QDataStream &
{
    quint8 ver;
    in >> ver;

    np->beginResetModel();

    int type;

    in >> np->_name
            >> type
            >> np->_strain
            >> np->_average;

    np->_type = (NonlinearProperty::Type)type;
    np->setVaried(np->average());
    np->endResetModel();
    return in;
}
