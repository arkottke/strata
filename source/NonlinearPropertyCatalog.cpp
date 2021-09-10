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

#include "NonlinearPropertyCatalog.h"

#include "ModulusFactory.h"
#include "DampingFactory.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

NonlinearPropertyCatalog::NonlinearPropertyCatalog()
{
    _modulusFactory = new ModulusFactory;
    _dampingFactory = new DampingFactory;

    _fileName = QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                 + "/nonlinearCurves";

    QFile file(_fileName);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> *(_modulusFactory) >> *(_dampingFactory);
    } 
}

NonlinearPropertyCatalog::~NonlinearPropertyCatalog()
{
    delete _modulusFactory;
    delete _dampingFactory;
}

auto NonlinearPropertyCatalog::modulusFactory() -> ModulusFactory*
{
    return _modulusFactory;
}

auto NonlinearPropertyCatalog::dampingFactory() -> DampingFactory*
{
    return _dampingFactory;
}

auto NonlinearPropertyCatalog::save() const -> bool
{
    const QString dest = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir;

    if (!dir.exists(dest) && !dir.mkpath(dest)) {
        return false;
    }

    QFile file(_fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << *(_modulusFactory) << *(_dampingFactory);
    } else {
        return false;
    }

    return true;
}
