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

#include "HelpDialog.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

HelpDialog::HelpDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    auto *layout = new QGridLayout;
    layout->setColumnStretch(3,1);

    auto * backPushButton = new QPushButton(QIcon(":/images/go-previous.svg"), tr("Back"));
    layout->addWidget( backPushButton, 0, 0);
    
    auto * forwardPushButton = new QPushButton(QIcon(":/images/go-next.svg"), tr("Forward"));
    layout->addWidget( forwardPushButton, 0, 1);

    auto * homePushButton = new QPushButton(QIcon(":/images/go-home.svg"), tr("Home"));
    layout->addWidget( homePushButton, 0, 2);

    _urlLineEdit = new QLineEdit;
    _urlLineEdit->setReadOnly(true);
    layout->addWidget( _urlLineEdit, 0, 3, 1, 2);


    // QHBoxLayout * rowLayout = new QHBoxLayout;
    // Table of contents
    // QTextBrowser * tocBrowser = new QTextBrowser;
    // tocBrowser->setSource(QUrl("qrc:/doc/manual2.html"));
    // tocBrowser->setOpenLinks(false);

    // rowLayout->addWidget( tocBrowser );

    // Main view
    _textBrowser = new QTextBrowser;
    // _textBrowser->setSource(QUrl("qrc:/doc/manual3.html"));
    _textBrowser->setSource(QUrl("qrc:/docs/index.html"));
    // connect( tocBrowser, SIGNAL(anchorClicked(QUrl)), _textBrowser, SLOT(setSource(QUrl)));
    connect( backPushButton, SIGNAL(clicked()), _textBrowser, SLOT(backward()));
    connect( _textBrowser, SIGNAL(backwardAvailable(bool)), backPushButton, SLOT(setEnabled(bool)));
    connect( forwardPushButton, SIGNAL(clicked()), _textBrowser, SLOT(forward()));
    connect( _textBrowser, SIGNAL(forwardAvailable(bool)), forwardPushButton, SLOT(setEnabled(bool)));
    connect( homePushButton, SIGNAL(clicked()), _textBrowser, SLOT(home()));
    connect( _textBrowser, SIGNAL(sourceChanged(QUrl)), SLOT(setCurrentAddress(QUrl)));

    // rowLayout->addWidget( _textBrowser, 1 );

    // layout->addLayout( rowLayout, 1, 0, 1, 5);
    layout->addWidget( _textBrowser, 1, 0, 1, 5);

    auto * closePushButton = new QPushButton(tr("Close"));
    connect( closePushButton, SIGNAL(clicked()), SLOT(close()));
    layout->addWidget( closePushButton, 2, 4);

    // Add the layout to the dialog
    setLayout(layout);

    // Set the window icon and text
    setWindowIcon(QIcon(":/images/help-browser.svg"));
    setWindowTitle(tr("Strata Help"));
}

void HelpDialog::setCurrentAddress(const QUrl & url)
{
    _urlLineEdit->setText(url.toString());
}

void HelpDialog::gotoLink(const QString & link)
{
    _textBrowser->setSource(QUrl(link));

    // Show the dialog if it is hidden
    if (!isVisible())
        show();

    activateWindow();
}
