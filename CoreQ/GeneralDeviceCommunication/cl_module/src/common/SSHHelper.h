#ifndef SSHHELPER_H
#define SSHHELPER_H

#include <QObject>
#include <QSharedPointer>
#include <QDateTime>
#include <QJSValue>
#include <functional>

using SSHDonwloadProgressFunc = std::function<void(qint64, qint64)>;
using SSHUploadProgressFunc = std::function<void(qint64, qint64)>;

class SSHHelperPrivate;
class SSHHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loggedInChanged)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorStringChanged)
    Q_PROPERTY(int errorCode READ errorCode WRITE setErrorCode NOTIFY errorCodeChanged FINAL)

public:
    explicit SSHHelper(QObject *parent=NULL);
    ~SSHHelper();

    bool isLoggedIn() const;
    void setLoggedIn(bool v);

    QString errorString() const;
    void setErrorString(QString v);

    int errorCode() const;
    void setErrorCode(int v);

    bool connectToHost(const QString &host, const quint16 &port = 22);
    void disconnectFromHost();

    bool login(const QString &username, const QString &password);
    bool login(const QString &username, const QString &publickey,
               const QString &privatekey, const QString &passphrase);
    void logout();

    bool sshExec(const QString &cmd, QByteArray &data);

    bool scpDownload(const QString &from, const QString &to, SSHDonwloadProgressFunc func = NULL);
    bool scpUpload(const QString &from, const QString &to, SSHUploadProgressFunc func = NULL);
    void scpAbort();

    struct FileInfo {
        QString path;
        QString name;
        qint64 filesize;
        QDateTime mtime;
        QDateTime atime;
        bool folder;
    };
    QList<FileInfo> sftpList(const QString &path, bool *ok = NULL);
    bool sftpRead(const QString &from, QByteArray &data);
    bool sftpWrite(const QString &to, const QByteArray &data);
    bool sftpRemove(const QString &filename);
    bool sftpRename(const QString &source, const QString &target);
    bool sftpMkdir(const QString &path);

public slots:
#ifdef HAVE_QUICK
    void qLogin(QString host, int port, QString username, QString password, QJSValue callback);
    void qLogout(QJSValue callback);

    void qSftpList(QString path, QJSValue callback);
    void qSftpRemove(QString filename, QJSValue callback);
    void qSftpRename(QString source, QString target, QJSValue callback);
    void qSftpMkdir(QString path, QJSValue callback);

    void qScpUpload(QString local_file, QString remote_path, QJSValue callback);
    void qScpDownload(QString remote_file, QString local_path, QJSValue callback);

    QString cleanPath(QString path);
    bool isUploadAllowed(QString filename);
#endif

signals:
    void loggedInChanged();
    void errorStringChanged();
    void errorCodeChanged();

private:
    QSharedPointer<SSHHelperPrivate> d;
};

#endif // SSHHELPER_H
