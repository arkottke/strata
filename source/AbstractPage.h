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

#ifndef ABSTRACT_PAGE_H
#define ABSTRACT_PAGE_H

#include <QWidget>

class SiteResponseModel;

class AbstractPage : public QWidget
{
Q_OBJECT
public:
    explicit AbstractPage(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual void setModel(SiteResponseModel* model) = 0;

signals:
    void linkActivated(const QString& link);

public slots:
    virtual void setReadOnly(bool readOnly);

};

#endif // ABSTRACTPAGE_H
