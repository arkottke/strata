#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPushButton>
#include <QProgressBar>
#include <QUrl>

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    UpdateDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);

protected slots:
    void replyFinished(QNetworkReply * reply);
    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
    void writeToFile();

    void cancel();
    void yes();

protected:
    enum Task {
        Query,
        FetchHead,
        Download
    };

    enum MessageType {
        Question,
        Action,
        Closure
    };

    void setMessage(const QString & message, MessageType type);

    //! Current network task
    Task m_task;

    //! URL of the strata.cgi file.
    QUrl m_url;

    //! Network manager
    QNetworkAccessManager * m_manager;

    //! Destination of the update
    QString m_pathName;

    //! Network reply to do the download
    QNetworkReply * m_reply;

    //! File device used to save the file to the disk
    QFile * m_file;

    //! If the task has been canceled
    bool m_canceled;

    // Elements of the GUI
    QLabel * m_label;
    QProgressBar * m_progressBar;
    QPushButton * m_cancelPushButton;
    QPushButton * m_closePushButton;
    QPushButton * m_noPushButton;
    QPushButton * m_yesPushButton;
};

#endif // UPDATEDIALOG_H
