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

#ifndef MY_PROGRESS_DIALOG_H_
#define MY_PROGRESS_DIALOG_H_

#include <QDialog>
#include <QProgressBar>

class MyProgressDialog : public QDialog
{
    Q_OBJECT

    public:
        MyProgressDialog( const QString & labelText, QWidget * parent = 0, Qt::WindowFlags f = 0 ); 

        const bool * okToContinue() const;
        QProgressBar * progressBar() const;
    
    signals:
        void canceled();
    
    private slots:
        void cancel();

    private:
        QProgressBar * m_progressBar;
        bool m_okToContinue;
};
#endif
