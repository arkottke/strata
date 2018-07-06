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

#include "CrustalAmplification.h"

#include "CrustalModel.h"
#include "Dimension.h"
#include "Serialize.h"

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

CrustalAmplification::CrustalAmplification(QObject *parent) :
    MyAbstractTableModel(parent), _source(Default)
{
    _crustalModel = new CrustalModel;
    connect(_crustalModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(calculate()));

    _interpolator = 0;
    _accelerator =  gsl_interp_accel_alloc();
}

CrustalAmplification::~CrustalAmplification()
{
    gsl_interp_accel_free(_accelerator);
    if (_interpolator)
        gsl_interp_free(_interpolator);

    _crustalModel->deleteLater();
}

QStringList CrustalAmplification::sourceList() {
    return {tr("Default"), tr("Specified"), tr("Calculated")};
}

void CrustalAmplification::setRegion(AbstractRvtMotion::Region region)
{     
    beginResetModel();
    // WUS and CEUS amplification from Campbell (2003)
    if (region == AbstractRvtMotion::CEUS) {
        _freq = {0.01, 0.09, 0.16, 0.51, 0.84, 1.25, 2.26, 3.17, 6.05,
                 16.60, 61.20, 100.00};
        _amp  = {1.00, 1.02, 1.03, 1.05, 1.07, 1.09, 1.11, 1.12, 1.13,
                 1.14, 1.15, 1.15, 1.15, 1.15, 1.15};
    } else if (region == AbstractRvtMotion::WUS) {
        // The final pair (100 Hz, 4.40) in this site amplication is estimated
        // from extrapolation of the data.
        _freq = {0.01, 0.09, 0.16, 0.51, 0.84, 1.25, 2.26, 3.17, 6.05,
                 16.60, 61.20, 100.00};
        _amp  = {1.00, 1.10, 1.18, 1.42, 1.58, 1.74, 2.06, 2.25, 2.58,
                 3.13, 4.00, 4.40};
    }
    endResetModel();
    clearInterp();
}

void CrustalAmplification::setRegion(int region)
{
    setRegion((AbstractRvtMotion::Region)region);
}

void CrustalAmplification::setSource(Source source) {
    if (_source != source) {
        _source = source;
        emit sourceChanged(_source);
    }
}

void CrustalAmplification::setSource(int source) {
    setSource((Source)source);
}

CrustalAmplification::Source CrustalAmplification::source() const {
    return _source;
}

int CrustalAmplification::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return _freq.size();
}

int CrustalAmplification::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant CrustalAmplification::headerData( int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (orientation)
    {
    case Qt::Horizontal:
        switch (section)
        {
        case FreqColumn:
            return tr("Frequency (Hz)");
        case AmpColumn:
            return tr("Amplification");
        }
    case Qt::Vertical:
        return section + 1;
    }

    return QVariant();
}

QVariant CrustalAmplification::data(const QModelIndex & index, int role) const
{
    if (index.parent()!=QModelIndex()) {
        return QVariant();
    }

    if ( role==Qt::DisplayRole || role==Qt::EditRole) {
        switch (index.column())
        {
        case FreqColumn:
            return QString::number(_freq.at(index.row()), 'e', 2);
        case AmpColumn:
            return QString::number(_amp.at(index.row()));
        }
    }

    return QVariant();
}

bool CrustalAmplification::setData( const QModelIndex & index, const QVariant & value, int role)
{
    if(index.parent() != QModelIndex() && role != Qt::EditRole) {
        return false;
    }

    bool b;
    const double d = value.toDouble(&b);

    if (b) {
        switch (index.column()) {
        case FreqColumn:
            _freq[index.row()] = d;
            break;
        case AmpColumn:
            _amp[index.row()] = d;
            break;
        }
        dataChanged(index, index);
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags CrustalAmplification::flags( const QModelIndex & index) const
{
    if (_source == Specified) {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    } else {
        return QAbstractTableModel::flags(index);
    }
}

bool CrustalAmplification::insertRows(int row, int count, const QModelIndex &parent)
{
    emit beginInsertRows(parent, row, row+count-1);

    _freq.insert(row, count, 0);
    _amp.insert(row, count, 0);

    emit endInsertRows();

    // Reset the interpolator
    clearInterp();

    return true;
}

bool CrustalAmplification::removeRows(int row, int count, const QModelIndex &parent)
{
    emit beginRemoveRows(parent, row, row+count-1 );

    _freq.remove(row, count);
    _amp.remove(row, count);

    emit endRemoveRows();

    // Reset the interpolator
    clearInterp();

    return true;
}

CrustalModel* CrustalAmplification::crustalModel()
{
    return _crustalModel;
}

double CrustalAmplification::interpAmpAt(double freq)
{
    if (!_interpolator) {
        // Make sure the interpolator has been intialized
        initInterp();
    }

    if (freq < _freq.first()) {
        return _amp.first();
    } else if (_freq.last() < freq) {
        return _amp.last();
    } else {
        return gsl_interp_eval(_interpolator, _freq.data(), _amp.data(), freq, _accelerator);
    }
}

void CrustalAmplification::calculate()
{
    if (_source == Calculated) {
        beginResetModel();
        _freq = Dimension::logSpace(0.01, 100., 20);
        _amp = _crustalModel->calculate(_freq);
        endResetModel();
    }
}

void CrustalAmplification::initInterp()
{
    _interpolator = gsl_interp_alloc(gsl_interp_linear, _freq.size());
    gsl_interp_init(_interpolator, _freq.data(), _amp.data(), _freq.size());
}

void CrustalAmplification::clearInterp()
{
    if (_interpolator) {
        gsl_interp_free(_interpolator);
        _interpolator = 0;
    }
}

void CrustalAmplification::fromJson(const QJsonObject &json)
{
    int source = json["source"].toInt();
    setSource(source);

    switch (_source) {
    case CrustalAmplification::Specified:
        beginResetModel();
        Serialize::toDoubleVector(json["freq"], _freq);
        Serialize::toDoubleVector(json["amp"], _amp);
        endResetModel();
    case CrustalAmplification::Calculated:
        _crustalModel->fromJson(json["crustalModel"].toObject());
        calculate();
        break;
    default:
        break;
    }
}

QJsonObject CrustalAmplification::toJson() const
{
    QJsonObject json;
    json["source"] = (int)_source;
    switch (_source) {
    case CrustalAmplification::Specified:
        json["freq"] = Serialize::toJsonArray(_freq);
        json["amp"] = Serialize::toJsonArray(_amp);
        break;
    case CrustalAmplification::Calculated:
        json["crustalModel"] = _crustalModel->toJson();
        break;
    default:
        break;
    }
    return json;
}

QDataStream & operator<< (QDataStream & out, const CrustalAmplification* ca)
{
    out << (quint8)1;
    out << (int)ca->_source;

    switch (ca->_source) {
    case CrustalAmplification::Specified:
        out << ca->_freq << ca->_amp;
        break;
    case CrustalAmplification::Calculated:
        out << ca->_crustalModel;
        break;
    default:
        break;
    }
    return out;
}

QDataStream & operator>> (QDataStream & in, CrustalAmplification* ca)
{
    quint8 ver;
    in >> ver;

    int source;
    in >> source;
    ca->setSource(source);

    switch (ca->_source) {
    case CrustalAmplification::Specified:
        ca->beginResetModel();
        in >> ca->_freq >> ca->_amp;
        ca->endResetModel();
        break;
    case CrustalAmplification::Calculated:
        in >> ca->_crustalModel;
        ca->calculate();
        break;
    default:
        break;
    }
    return in;
}
