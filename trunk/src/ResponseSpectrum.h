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

#ifndef RESPONSE_SPECTRUM_H_
#define RESPONSE_SPECTRUM_H_

#include "MyAbstractTableModel.h"

#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>

class ResponseSpectrum : public MyAbstractTableModel
{
    Q_OBJECT

    public:
        ResponseSpectrum(bool readOnly = false, QObject * parent = 0);
        
        //! Reset the object to the default values
        void reset();

        bool modified() const;

        double damping() const;

        const QVector<double> & period() const;
        void setPeriod(const QVector<double> & period);

        const QVector<double> & sa() const;
        void setSa(const QVector<double> & sa);
        
        //! Convert the ResponseSpectrum to a QMap for saving
        QMap<QString, QVariant> toMap() const;
        
        //! Load the ResponseSpectrum from a QMap
        void fromMap( const QMap<QString, QVariant> & map);

        int rowCount ( const QModelIndex &parent = QModelIndex() ) const;
        int columnCount ( const QModelIndex &parent = QModelIndex() ) const;

        QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
        bool setData ( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags ( const QModelIndex &index ) const;

        bool insertRows ( int row, int count, const QModelIndex &parent = QModelIndex() );
        bool removeRows ( int row, int count, const QModelIndex &parent = QModelIndex() );

    signals:
        void wasModified();

    public slots:
        void setModified(bool modified = true);
        void setDamping(double damping);

    private:
        //! If the data has been modified
        bool m_modified;

        //! Damping in percent
        double m_damping;

        //! Period values in seconds
        QVector<double> m_period;

        //! Spectral acceleration in gravity
        QVector<double> m_sa;
};
#endif
