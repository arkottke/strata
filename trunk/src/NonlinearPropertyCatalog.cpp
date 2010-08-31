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

#include "NonlinearPropertyCatalog.h"

#include "ModulusFactory.h"
#include "DampingFactory.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>

NonlinearPropertyCatalog::NonlinearPropertyCatalog()
{
    m_modulusFactory = new ModulusFactory;
    m_dampingFactory = new DampingFactory;

    m_fileName = QDesktopServices::storageLocation(QDesktopServices::DataLocation)
                 + "/nonlinearCurves";

    QFile file(m_fileName);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file);
            in >> *(m_modulusFactory) >> *(m_dampingFactory);
        } else {
            qDebug() << "Failed to open:" << qPrintable(m_fileName);
        }
    } else {
        qDebug() << "NonlinearPropertyCatalog file doesn't exist.";
    }
}

NonlinearPropertyCatalog::~NonlinearPropertyCatalog()
{
    delete m_modulusFactory;
    delete m_dampingFactory;
}

ModulusFactory* const NonlinearPropertyCatalog::modulusFactory()
{
    return m_modulusFactory;
}

DampingFactory* const NonlinearPropertyCatalog::dampingFactory()
{
    return m_dampingFactory;
}

bool NonlinearPropertyCatalog::save() const
{
    qDebug() << "Saving data!";

    const QString dest = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QDir dir;

    if (!dir.exists(dest) && !dir.mkpath(dest)) {
        qDebug() << "Failed to created:" << qPrintable(dest);
        return false;
    }

    QFile file(m_fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << *(m_modulusFactory) << *(m_dampingFactory);
    } else {
        return false;
    }

    return true;
}
