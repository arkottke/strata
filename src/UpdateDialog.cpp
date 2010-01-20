#include "UpdateDialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QNetworkReply>
#include <QVBoxLayout>

#include <QDebug>

UpdateDialog::UpdateDialog(QWidget * parent, Qt::WindowFlags f)
{
    m_url = QUrl("http://accipter.org/cgi-bin/strata.cgi");
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    m_file = new QFile;


    // Create the layout
    QVBoxLayout * layout = new QVBoxLayout;

    m_label = new QLabel("Checking if an update is available...");
    layout->addWidget(m_label);

    layout->addSpacing(20);
    layout->addStretch(1);

    m_progressBar = new QProgressBar;
    m_progressBar->setMinimum(0);
    layout->addWidget(m_progressBar);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    m_cancelPushButton = new QPushButton(tr("Cancel"));
    connect(m_cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));
    buttonLayout->addWidget(m_cancelPushButton);

    m_closePushButton = new QPushButton(tr("Close"));
    connect(m_closePushButton, SIGNAL(clicked()), this, SLOT(accept()));
    buttonLayout->addWidget(m_closePushButton);

    m_noPushButton = new QPushButton(tr("No"));
    connect(m_noPushButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(m_noPushButton);

    m_yesPushButton = new QPushButton(tr("Yes"));
    connect(m_yesPushButton, SIGNAL(clicked()), this, SLOT(yes()));
    buttonLayout->addWidget(m_yesPushButton);

    layout->addLayout(buttonLayout);

    setLayout(layout);

    // Start the update process
    QUrl url = m_url;
    url.addQueryItem("action", "query");

    setMessage(tr("Checking if an update is available..."), Action);

    m_canceled = false;
    m_task = Query;
    QNetworkReply * reply = m_manager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
}

UpdateDialog::~UpdateDialog()
{
    delete m_manager;
    delete m_file;
}

void UpdateDialog::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_canceled) {
        m_reply->abort();
        return;
    }

    m_progressBar->setMaximum(bytesTotal);
    m_progressBar->setValue(bytesReceived);
}

void UpdateDialog::cancel()
{
    m_canceled = true;;
}

void UpdateDialog::yes()
{
    switch(m_task) {
    case Query:
        {
            setMessage(tr("Downloading latest version..."), Action);
            // Fetch the latest version
            QUrl url = m_url;
            url.addQueryItem("action", "download");
            url.addQueryItem("version", "latest");
            m_task = FetchHead;
            m_reply = m_manager->head(QNetworkRequest(url));
            connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
                    this, SLOT(updateProgress(qint64,qint64)));

            break;
        }
    default:
        break;
    }
}

void UpdateDialog::replyFinished(QNetworkReply * reply)
{
    if (m_canceled || reply->error() != QNetworkReply::NoError) {
        qDebug() << "Request failed, " << reply->errorString() << reply->url();
        if (m_file->isOpen() && m_task == Download) {
            m_file->close();
            if (QFile::exists(m_pathName)) {
                QFile::remove(m_pathName);
            }
        }
        close();
        return;
    }

    qDebug() << "Request succeeded";
    qDebug() << reply->url();

    switch (m_task) {
    case Query:
        {
            int latestRev = reply->readAll().trimmed().toInt();
            int currentRev = int(REVISION);

            if (currentRev < latestRev) {
                setMessage(tr("A new version of Strata is available.\n"
                               "Do you want to download the updated version?"), Question);
            } else {
                setMessage(tr("No updates currently available."), Closure);
            }
            break;
        }
    case FetchHead:
        {
            QUrl url = reply->header(QNetworkRequest::LocationHeader).toUrl();
            QString fileName = QFileInfo(url.path()).fileName();

            // Start the download
            m_task = Download;
            m_reply = m_manager->get(QNetworkRequest(url));

            // Prompt for a new fileName -- FIXME parent
            QString pathName = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation)
                               + QDir::separator() + fileName;

            m_pathName = QFileDialog::getSaveFileName(this, tr("Saving %1").arg(fileName), pathName);

            if (m_pathName.isEmpty()) {
                // Cancel the download
                m_reply->close();
                return;
            }

            // Check if the file exists, and if so delete it
            if (QFile::exists(m_pathName)) {
                QFile::remove(m_pathName);
            }

            // Open the file for writing
            m_file->setFileName(m_pathName);

            if (!m_file->open(QIODevice::WriteOnly)) {
                qDebug() << "Error opening" << m_pathName;
                m_reply->close();
                return;
            }

            connect(m_reply, SIGNAL(readyRead()), this, SLOT(writeToFile()));
            connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)),
                    this, SLOT(updateProgress(qint64,qint64)));

            break;
        }
    case Download:
        {
            m_reply = 0;
            m_file->close();

            // Update the dialog
            setMessage(tr("Please close Strata and then run the installer"), Closure);
            break;
        }
    }

    delete reply;
}

void UpdateDialog::writeToFile()
{
    if (m_file->isOpen()) {
        m_file->write(m_reply->readAll());
    }
}


void UpdateDialog::setMessage(const QString & message, MessageType type)
{
    m_label->setText(message);

    switch(type)
    {
    case Question:
        m_progressBar->setShown(false);
        m_cancelPushButton->setShown(false);
        m_closePushButton->setShown(false);
        m_noPushButton->setShown(true);
        m_yesPushButton->setShown(true);

        m_yesPushButton->setDefault(true);
        break;
    case Action:
        m_progressBar->setShown(true);
        m_cancelPushButton->setShown(true);
        m_closePushButton->setShown(false);
        m_noPushButton->setShown(false);
        m_yesPushButton->setShown(false);
        break;
    case Closure:
        m_progressBar->setShown(false);
        m_cancelPushButton->setShown(false);
        m_closePushButton->setShown(true);
        m_noPushButton->setShown(false);
        m_yesPushButton->setShown(false);

        m_closePushButton->setDefault(false);
        break;
    }
}
