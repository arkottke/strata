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

#include "HelpDialog.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

HelpDialog::HelpDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch(3,1);

    QPushButton * backPushButton = new QPushButton(QIcon(":/images/go-previous.svg"), tr("Back"));
    layout->addWidget( backPushButton, 0, 0);
    
    QPushButton * forwardPushButton = new QPushButton(QIcon(":/images/go-next.svg"), tr("Forward"));
    layout->addWidget( forwardPushButton, 0, 1);

    QPushButton * homePushButton = new QPushButton(QIcon(":/images/go-home.svg"), tr("Home"));
    layout->addWidget( homePushButton, 0, 2);

    m_urlLineEdit = new QLineEdit;
    m_urlLineEdit->setReadOnly(true);
    layout->addWidget( m_urlLineEdit, 0, 3, 1, 2);


    // QHBoxLayout * rowLayout = new QHBoxLayout;
    // Table of contents
    // QTextBrowser * tocBrowser = new QTextBrowser;
    // tocBrowser->setSource(QUrl("qrc:/doc/manual2.html"));
    // tocBrowser->setOpenLinks(false);

    // rowLayout->addWidget( tocBrowser );

    // Main view
    m_textBrowser = new QTextBrowser;
    // m_textBrowser->setSource(QUrl("qrc:/doc/manual3.html"));
    m_textBrowser->setSource(QUrl("qrc:/docs/index.html"));
    // connect( tocBrowser, SIGNAL(anchorClicked(QUrl)), m_textBrowser, SLOT(setSource(QUrl)));
    connect( backPushButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
    connect( m_textBrowser, SIGNAL(backwardAvailable(bool)), backPushButton, SLOT(setEnabled(bool)));
    connect( forwardPushButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
    connect( m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardPushButton, SLOT(setEnabled(bool)));
    connect( homePushButton, SIGNAL(clicked()), m_textBrowser, SLOT(home()));
    connect( m_textBrowser, SIGNAL(sourceChanged(QUrl)), SLOT(setCurrentAddress(QUrl)));

    // rowLayout->addWidget( m_textBrowser, 1 );

    // layout->addLayout( rowLayout, 1, 0, 1, 5);
    layout->addWidget( m_textBrowser, 1, 0, 1, 5);

    QPushButton * closePushButton = new QPushButton(tr("Close"));
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
    m_urlLineEdit->setText(url.toString());
}

void HelpDialog::gotoLink(const QString & link)
{
    m_textBrowser->setSource(QUrl(link));

    // Show the dialog if it is hidden
    if (!isVisible())
        show();

    activateWindow();
}
