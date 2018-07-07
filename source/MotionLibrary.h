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

#ifndef MOTION_LIBRARY_H
#define MOTION_LIBRARY_H

#include "AbstractMotion.h"
#include "MyAbstractTableModel.h"

#include <QDataStream>
#include <QJsonObject>

class MotionLibrary : public MyAbstractTableModel
{
    Q_OBJECT

    friend QDataStream & operator<< (QDataStream & out, const MotionLibrary* ml);
    friend QDataStream & operator>> (QDataStream & in, MotionLibrary* ml);

public:
    explicit MotionLibrary(QObject *parent = nullptr);

    //! Table columns
    enum Column {
        NameColumn,
        DescriptionColumn,
        TypeColumn,
        PgaColumn,
        PgvColumn,
        ScaleColumn
    };

    //! Approach for preforming the analysi
    enum Approach {
        TimeSeries, //!< Use time series
        RandomVibrationTheory //!< Use random vibration theory
    };

    static QStringList approachList();

    QString toHtml() const;

    //!@{ Methods modifying the library
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //!@}

    bool saveData() const;

    //! Number of enabled motions
    int motionCount() const;

    //! Add a motion to the model
    void addMotion(AbstractMotion *motion);

    //! Return the motion at the index
    AbstractMotion* motionAt(int row);

    //! Update the data along a row
    void updateRow(int row);

    Approach approach() const;
    void setApproach(Approach approach);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

signals:    
    void wasModified();
    void approachChanged(int approach);
    void saveDataChanged(bool saveData);

public slots:
    void setSaveData(bool b);
    void setApproach(int approach);
    virtual void setReadOnly(bool readOnly);

protected slots:
    void updateUnits();

protected:    
    //! Approach used to characterize input motions
    Approach _approach;

    //! If the motion data should be saved within the file -- only important for time series
    bool _saveData;

    //! List of motions
    QList<AbstractMotion*> _motions;

};

#endif // MOTION_LIBRARY_H
