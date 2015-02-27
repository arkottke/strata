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

#ifndef RVT_MOTION_H_
#define RVT_MOTION_H_

#include "AbstractRvtMotion.h"

#include <QDataStream>
#include <boost/property_tree/ptree.hpp>

using boost::property_tree::ptree;


/*! A class that uses random vibration theory to compute the reponse.
 * The motion is characterized by the Fourier Amplitude and random vibration
 * thereory is used to predict the expected response.
 */

class RvtMotion : public AbstractRvtMotion
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const RvtMotion * rm);
    friend QDataStream & operator>> (QDataStream & in, RvtMotion * rm);

public:
    //! Constructor
    RvtMotion(QObject * parent = 0);

    //!{@ Method for editing the model with a QTableView
    virtual Qt::ItemFlags flags( const QModelIndex & index) const;

    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!}@

    virtual const QVector<double> & freq() const;
    virtual QString toHtml() const;
    virtual bool loadFromTextStream(QTextStream &stream, double scale = 1.);

    void ptRead(const ptree &pt);
    void ptWrite(ptree &pt) const;

public slots:
    void setDuration(double d);

protected:
    //! Vector of frequency values
    QVector<double> m_freq;
};
#endif
