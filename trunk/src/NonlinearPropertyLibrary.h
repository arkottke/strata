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

#ifndef NONLINEAR_PROPERTY_LIBRARY_H_
#define NONLINEAR_PROPERTY_LIBRARY_H_

#include "NonlinearProperty.h"
#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QStringList>

class NonlinearPropertyLibrary : public QObject
{
    friend class SoilTypeTableModel;
    friend class NonlinearPropertyLibraryDialog;

    public:
        NonlinearPropertyLibrary();
        ~NonlinearPropertyLibrary();

        //! List of all normalized shear modulus models
        QStringList modulusList() const;
        
        //! List of all damping models
        QStringList dampingList() const;

    public slots:
        //! Load the user defined models
        void load();
        //! Save the user defined models
        void save();

    protected:
        QList<NonlinearProperty*> & modulus();
        QList<NonlinearProperty*> & damping();

    private:
        //! List of the shear modulus reduction models
        QList<NonlinearProperty*> m_modulus;
        
        //! List of the damping models
        QList<NonlinearProperty*> m_damping;

        //! File that stores the user defined data
        QString m_fileName;

        //! Create the hard coded models
        void createModels();
};
#endif
