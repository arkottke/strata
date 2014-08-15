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

#include "CrustalAmplification.h"

#include "CrustalModel.h"
#include "Dimension.h"

#include <cmath>

CrustalAmplification::CrustalAmplification(QObject *parent) :
        MyAbstractTableModel(parent)
{
    m_crustalModel = new CrustalModel;
    connect(m_crustalModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(calculate()));

    m_interpolator = 0;
    m_accelerator =  gsl_interp_accel_alloc();
}

CrustalAmplification::~CrustalAmplification()
{
    gsl_interp_accel_free(m_accelerator);
    if (m_interpolator)
        gsl_interp_free(m_interpolator);

    m_crustalModel->deleteLater();
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

    return m_freq.size();
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
            return QString::number(m_freq.at(index.row()), 'e', 2);
        case AmpColumn:
            return QString::number(m_amp.at(index.row()));
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
            m_freq[index.row()] = d;
            break;
        case AmpColumn:
            m_amp[index.row()] = d;
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
    if (m_model == Custom) {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    } else {
        return QAbstractTableModel::flags(index);
    }
}

bool CrustalAmplification::insertRows(int row, int count, const QModelIndex &parent)
{
    emit beginInsertRows(parent, row, row+count-1);

    m_freq.insert(row, count, 0);
    m_amp.insert(row, count, 0);

    emit endInsertRows();

    // Reset the interpolator
    clearInterp();

    return true;
}

bool CrustalAmplification::removeRows(int row, int count, const QModelIndex &parent)
{
    emit beginRemoveRows(parent, row, row+count-1 );

    m_freq.remove(row, count);
    m_amp.remove(row, count);

    emit endRemoveRows();

    // Reset the interpolator
    clearInterp();

    return true;
}

CrustalAmplification::Model CrustalAmplification::model() const
{
    return m_model;
}

void CrustalAmplification::setModel(Model s)
{
    beginResetModel();
    m_model = s;

    switch (m_model) {
        // WUS and CEUS amplification from Campbell (2003)
    case WUS:
        // The final pair (100 Hz, 4.40) in this site amplication is estimated
        // from extrapolation of the data.
        m_freq.clear();
        m_freq  << 0.01
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

        m_amp.clear();
        m_amp  << 1.00
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
        m_freq.clear();
        m_freq << 0.01
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

        m_amp.clear();
        m_amp  << 1.00
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
        m_freq.clear();
        m_amp.clear();
        break;
    }
    Q_ASSERT(m_freq.size() == m_amp.size());
    endResetModel();

    // Reset the interpolator
    clearInterp();

    emit modelChanged(m_model);
    emit needsCrustalModelChanged(m_model == Calculated);
    emit readOnlyChanged(m_model != Custom);
}

void CrustalAmplification::setModel(int s)
{
    setModel((Model)s);
}

CrustalModel* CrustalAmplification::crustalModel()
{
    return m_crustalModel;
}

double CrustalAmplification::interpAmpAt(double freq)
{
    if (!m_interpolator) {
        // Make sure the interpolator has been intialized
        initInterp();
    }

    if (freq < m_freq.first()) {
        return m_amp.first();
    } else if (m_freq.last() < freq) {
        return m_amp.last();
    } else {
        return gsl_interp_eval(m_interpolator, m_freq.data(), m_amp.data(), freq, m_accelerator);
    }
}

void CrustalAmplification::calculate()
{
    if (m_model == Calculated) {
        beginResetModel();

        m_freq = Dimension::logSpace(0.01, 100., 20);
        m_amp = m_crustalModel->calculate(m_freq);
        endResetModel();
    }
}

void CrustalAmplification::initInterp()
{
    m_interpolator = gsl_interp_alloc(gsl_interp_linear, m_freq.size());
    gsl_interp_init(m_interpolator, m_freq.data(), m_amp.data(), m_freq.size());
}

void CrustalAmplification::clearInterp()
{
    if (m_interpolator) {
        gsl_interp_free(m_interpolator);
        m_interpolator = 0;
    }
}

QDataStream & operator<< (QDataStream & out, const CrustalAmplification* ca)
{
    out << (quint8)1;

    out << (int)ca->m_model;

    switch (ca->m_model) {
    case CrustalAmplification::Custom:
        out << ca->m_freq << ca->m_amp;
        break;
    case CrustalAmplification::WUS:
    case CrustalAmplification::CEUS:
        break;
    case CrustalAmplification::Calculated:
        out << ca->m_crustalModel;
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

    switch (ca->m_model) {
    case CrustalAmplification::Custom:
        ca->beginResetModel();
        in >> ca->m_freq >> ca->m_amp;
        ca->endResetModel();
        break;
    case CrustalAmplification::WUS:
    case CrustalAmplification::CEUS:
        break;
    case CrustalAmplification::Calculated:
        in >> ca->m_crustalModel;
        ca->calculate();
        break;
    }

    return in;
}
