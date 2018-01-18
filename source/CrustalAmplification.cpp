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

#include <QJsonArray>
#include <QJsonValue>

#include <cmath>

CrustalAmplification::CrustalAmplification(QObject *parent) :
        MyAbstractTableModel(parent)
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

QStringList CrustalAmplification::sourceList()
{
    return QStringList()
            << tr("Custom")
            << tr("Western NA")
            << tr("Eastern NA")
            << tr("Calculated");
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
    if (_model == Custom) {
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

CrustalAmplification::Model CrustalAmplification::model() const
{
    return _model;
}

void CrustalAmplification::setModel(Model s)
{
    beginResetModel();
    _model = s;

    switch (_model) {
        // WUS and CEUS amplification from Campbell (2003)
    case WUS:
        // The final pair (100 Hz, 4.40) in this site amplication is estimated
        // from extrapolation of the data.
        _freq.clear();
        _freq  << 0.01
                << 0.09
                << 0.16
                << 0.51
                << 0.84
                << 1.25
                << 2.26
                << 3.17
                << 6.05
                << 16.60
                << 61.20
                << 100.00;

        _amp.clear();
        _amp  << 1.00
                << 1.10
                << 1.18
                << 1.42
                << 1.58
                << 1.74
                << 2.06
                << 2.25
                << 2.58
                << 3.13
                << 4.00
                << 4.40;

        break;
    case CEUS:
        _freq.clear();
        _freq << 0.01
                << 0.10
                << 0.20
                << 0.30
                << 0.50
                << 0.90
                << 1.25
                << 1.80
                << 3.00
                << 5.30
                << 8.00
                << 14.00
                << 30.00
                << 60.00
                << 100.00;

        _amp.clear();
        _amp  << 1.00
                << 1.02
                << 1.03
                << 1.05
                << 1.07
                << 1.09
                << 1.11
                << 1.12
                << 1.13
                << 1.14
                << 1.15
                << 1.15
                << 1.15
                << 1.15
                << 1.15;

        break;
    case Custom:
    case Calculated:
        _freq.clear();
        _amp.clear();
        break;
    }
    Q_ASSERT(_freq.size() == _amp.size());
    endResetModel();

    // Reset the interpolator
    clearInterp();

    emit modelChanged(_model);
    emit needsCrustalModelChanged(_model == Calculated);
    emit readOnlyChanged(_model != Custom);
}

void CrustalAmplification::setModel(int s)
{
    setModel((Model)s);
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
    if (_model == Calculated) {
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
    int model = json["model"].toInt();
    setModel(model);

    switch (_model) {
        case CrustalAmplification::Custom:
        {
            beginResetModel();

            _freq.clear();
            foreach (const QJsonValue &d, json["freq"].toArray())
                _freq << d.toDouble();

            _amp.clear();
            foreach (const QJsonValue &d, json["amp"].toArray())
                _amp << d.toDouble();

            endResetModel();
            break;
        }
        case CrustalAmplification::WUS:
        case CrustalAmplification::CEUS:
            break;
        case CrustalAmplification::Calculated:
        {
            _crustalModel->fromJson(json["crustalModel"].toObject());
            calculate();
            break;
        }
    }
}

QJsonObject CrustalAmplification::toJson() const
{
    QJsonObject json;
    json["model"] = (int) _model;
    switch (_model) {
        case CrustalAmplification::Custom:
        {
            QJsonArray freq;
            foreach(const double &d, _freq)
                freq << QJsonValue(d);
            json["freq"] = freq;

            QJsonArray amp;
            foreach(const double &d, _amp)
                amp << QJsonValue(d);
            json["amp"] = amp;
            break;
        }
        case CrustalAmplification::WUS:
        case CrustalAmplification::CEUS:
            break;
        case CrustalAmplification::Calculated:
            json["crustalModel"] = _crustalModel->toJson();
            break;
    }
    return json;
}

QDataStream & operator<< (QDataStream & out, const CrustalAmplification* ca)
{
    out << (quint8)1;

    out << (int)ca->_model;

    switch (ca->_model) {
    case CrustalAmplification::Custom:
        out << ca->_freq << ca->_amp;
        break;
    case CrustalAmplification::WUS:
    case CrustalAmplification::CEUS:
        break;
    case CrustalAmplification::Calculated:
        out << ca->_crustalModel;
        break;
    }

    return out;
}

QDataStream & operator>> (QDataStream & in, CrustalAmplification* ca)
{
    quint8 ver;
    in >> ver;

    int model;
    in >> model;
    ca->setModel(model);

    switch (ca->_model) {
    case CrustalAmplification::Custom:
        ca->beginResetModel();
        in >> ca->_freq >> ca->_amp;
        ca->endResetModel();
        break;
    case CrustalAmplification::WUS:
    case CrustalAmplification::CEUS:
        break;
    case CrustalAmplification::Calculated:
        in >> ca->_crustalModel;
        ca->calculate();
        break;
    }

    return in;
}
