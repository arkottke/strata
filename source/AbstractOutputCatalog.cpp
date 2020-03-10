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

#include "AbstractOutputCatalog.h"

#include "AbstractOutput.h"
#include "AbstractProfileOutput.h"
#include "MotionLibrary.h"
#include "OutputCatalog.h"

#include <QStringList>

AbstractOutputCatalog::AbstractOutputCatalog(OutputCatalog *outputCatalog) :
    MyAbstractTableModel(outputCatalog), 
    _outputCatalog(outputCatalog),
    _approach(MotionLibrary::TimeSeries)
{
}

void AbstractOutputCatalog::setApproach(int approach)
{
    beginResetModel();
    _approach = (MotionLibrary::Approach)approach;

    // Disable time series only outputs
    if (_approach == MotionLibrary::RandomVibrationTheory) {
        for (auto *ao : outputs()) {
            auto *apo = qobject_cast<AbstractProfileOutput *>(ao);
            if (apo && apo->timeSeriesOnly())
                apo->setEnabled(false);
        }
    }

    endResetModel();
}

auto AbstractOutputCatalog::data(const QModelIndex & index, int role ) const -> QVariant
{
    return MyAbstractTableModel::data(index, role);
}

