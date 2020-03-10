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

    friend auto operator<< (QDataStream & out, const MotionLibrary* ml) -> QDataStream &;
    friend auto operator>> (QDataStream & in, MotionLibrary* ml) -> QDataStream &;

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

    static auto approachList() -> QStringList;

    auto toHtml() const -> QString;

    //!@{ Methods modifying the library
    virtual auto rowCount(const QModelIndex & parent = QModelIndex()) const -> int;
    virtual auto columnCount(const QModelIndex & parent = QModelIndex()) const -> int;

    virtual auto data(const QModelIndex & index, int role = Qt::DisplayRole) const -> QVariant;
    virtual auto headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const -> QVariant;

    virtual auto flags(const QModelIndex & index) const -> Qt::ItemFlags;

    virtual auto setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) -> bool;

    virtual auto removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) -> bool;
    //!@}

    auto saveData() const -> bool;

    //! Number of enabled motions
    auto motionCount() const -> int;

    //! Add a motion to the model
    void addMotion(AbstractMotion *motion);

    //! Return the motion at the index
    auto motionAt(int row) -> AbstractMotion*;

    //! Update the data along a row
    void updateRow(int row);

    auto approach() const -> Approach;
    void setApproach(Approach approach);

    void fromJson(const QJsonObject &json);
    auto toJson() const -> QJsonObject;

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
