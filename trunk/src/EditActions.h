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

#include <QObject>
#include <QAction>

//! Class used to facilitate universal edit actions for multiple dialogs.
/*!
 * More details?
 */

class EditActions : public QObject
{
    Q_OBJECT

    public:
        static EditActions * instance();

        QAction * copyAction();
        QAction * pasteAction();
        QAction * cutAction();
        QAction * clearAction();

    protected slots:
        void paste();
        void copy();
        void cut();
        void clear();

    protected:
        EditActions(QObject * parent = 0);

        static EditActions * m_instance;
        
        QAction * m_copyAction;
        QAction * m_pasteAction;
        QAction * m_cutAction;
        QAction * m_clearAction;
};
