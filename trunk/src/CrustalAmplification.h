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

#ifndef CRUSTAL_AMPLIFICATION_H
#define CRUSTAL_AMPLIFICATION_H

#include "MyAbstractTableModel.h"

#include <QString>
#include <QStringList>
#include <QVector>

#include <gsl/gsl_interp.h>

class CrustalModel;

class CrustalAmplification : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const CrustalAmplification* ca);
    friend QDataStream & operator>> (QDataStream & in, CrustalAmplification* ca);

public:
    CrustalAmplification(QObject *parent=0);
    ~CrustalAmplification();

    enum Model {
        Custom, //!< Custom location
        WUS, //!< Generic Western North America Parameters
        CEUS, //!< Generic Eastern North America Parameters
        Calculated //!< Calculate from Crustal Model
    };

    enum Columns {
        FreqColumn,
        AmpColumn
    };

    static QStringList sourceList();

    //!@{ Methods for QAbstractTableModel
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual Qt::ItemFlags flags( const QModelIndex & index) const;

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!@}

    Model model() const;
    void setModel(Model s);

    CrustalModel* crustalModel();

    double interpAmpAt(double freq);

signals:
    void modelChanged(int i);
    void readOnlyChanged(bool b);
    void needsCrustalModelChanged(bool b);

public slots:
    void setModel(int s);

private slots:
    void calculate();

private:
    //! Initialize the interpolator
    void initInterp();

    //! Clear the interpolator
    void clearInterp();

    //! Frequency
    QVector<double> m_freq;

    //! Amplification
    QVector<double> m_amp;

    //! Region of crustal model
    Model m_model;

    //! Specific crustal model
    CrustalModel *m_crustalModel;

    //! GSL interpolator
    gsl_interp *m_interpolator;

    //! Accelerator for the interpolation
    gsl_interp_accel *m_accelerator;
};

#endif // CRUSTAL_AMPLIFICATION_H
