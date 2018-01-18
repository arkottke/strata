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

#ifndef COMPUTE_PAGE_H_
#define COMPUTE_PAGE_H_

#include "AbstractPage.h"

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QTime>

class SiteResponseModel;

//! A class for starting the computation and tracking the progress.

class ComputePage : public AbstractPage
{
    Q_OBJECT

public:
    ComputePage(QWidget* parent = 0, Qt::WindowFlags f = 0);

    virtual void setModel(SiteResponseModel* model);

public slots:
    virtual void setReadOnly(bool readOnly);

signals:
    void startCalculation();
    void saveRequested();
    
protected slots:
    void compute();
    void updateEta(int value);

    void reset();

protected:
    QTextEdit * _logView;

    QProgressBar * _progressBar;
    QLineEdit * _etaLineEdit;

    QPushButton * _computeButton;
    QPushButton * _cancelButton;

    QTime _timer;

    SiteResponseModel* _model;
};
#endif
