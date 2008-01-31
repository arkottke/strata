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
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

HelpDialog::HelpDialog( QWidget * parent, Qt::WindowFlags f )
    : QDialog(parent, f)
{
    QGridLayout * layout = new QGridLayout;
    layout->setColumnStretch( 3, 1);

    // Back button
    QPushButton * backButton = new QPushButton( QIcon(":/images/go-previous.svg"), tr("Back"));
    backButton->setEnabled(false);

    layout->addWidget( backButton, 0, 0);

    // Foward button
    QPushButton * forwardButton = new QPushButton( QIcon(":/images/go-next.svg"), tr("Forward"));
    forwardButton->setEnabled(false);

    layout->addWidget( forwardButton, 0, 1);

    // Home button
    QPushButton * homeButton = new QPushButton( QIcon(":/images/go-home.svg"), tr("Home"));

    layout->addWidget( homeButton, 0, 2);

    // Address line edit
    m_addressLineEdit = new QLineEdit;
    m_addressLineEdit->setReadOnly(true);

    layout->addWidget( m_addressLineEdit, 0, 3, 1, 2);

    // Browser area
    m_textBrowser = new QTextBrowser;
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setSource(QUrl("qrc:/docs/index.html"));

    layout->addWidget( m_textBrowser, 1, 0, 1, 5);

    // Close button
    QPushButton * closeButton = new QPushButton(tr("Close"));

    layout->addWidget( closeButton, 2, 4);

    // Connections
    connect( backButton, SIGNAL(clicked()), m_textBrowser, SLOT(backward()));
    connect( forwardButton, SIGNAL(clicked()), m_textBrowser, SLOT(forward()));
    connect( homeButton, SIGNAL(clicked()), m_textBrowser, SLOT(home()));

    connect( m_textBrowser, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
    connect( m_textBrowser, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));

    connect( m_textBrowser, SIGNAL(sourceChanged(QUrl)), this, SLOT(setCurrentAddress(QUrl)));

    // Add the layout to the dialog
    setLayout(layout);

    // Set the window icon and text
    setWindowIcon(QIcon(":/images/help-browser.svg"));
    setWindowTitle(tr("Strata Help"));
}

void HelpDialog::setCurrentAddress(const QUrl & url)
{
    m_addressLineEdit->setText(url.toString(QUrl::RemovePath));
}

void HelpDialog::gotoLink(const QString & link)
{
    m_textBrowser->setSource(QUrl(link));

    // Show the dialog if it is hidden
    if (!isVisible())
        show();

    activateWindow();
}
