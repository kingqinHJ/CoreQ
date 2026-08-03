#include "SSHHelper.h"
#include "WorkerThread.h"
#include "Utils.h"

#ifdef HAVE_QUICK
#include "qml/QmlUtils.h"
#endif

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <QFileInfo>
#include <QFile>
#include <QUuid>
#include <QThread>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>

#ifdef Q_OS_LINUX
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#define PRINT_ERRORSTRING(format, ...) \
    do { \
        setErrorString(QString::asprintf(format, ##__VA_ARGS__)); \
        if (d->session) setErrorCode(libssh2_session_last_errno(d->session)); \
        LOGW(format, ##__VA_ARGS__); \
    } while (false)

class SSHHelperPrivate
{
public:
    SSHHelper *q;
    WorkerThread worker_thread;

    libssh2_socket_t sock = LIBSSH2_INVALID_SOCKET;
    LIBSSH2_SESSION *session = NULL;
    int seq = 0;

    bool is_logged_in = false;
    bool abort = true;
    bool abort_scp = true;

    QString error_string;
    int error_code = LIBSSH2_ERROR_NONE;

    static int s_ref;
};
int SSHHelperPrivate::s_ref = 0;

SSHHelper::SSHHelper(QObject *parent) :
    QObject(parent)
{
    if (SSHHelperPrivate::s_ref == 0)
        libssh2_init(0);
    SSHHelperPrivate::s_ref++;

    d.reset(new SSHHelperPrivate);
    d->q = this;
    d->worker_thread.setObjectName("SSH");
}

SSHHelper::~SSHHelper()
{
    d->abort = true;
    d->abort_scp = true;

    logout();
    disconnectFromHost();

    d->worker_thread.waitForDone();

    SSHHelperPrivate::s_ref--;
    if (SSHHelperPrivate::s_ref == 0)
        libssh2_exit();

    LOG_THIS();
}

bool SSHHelper::isLoggedIn() const
{
    return d->is_logged_in;
}

void SSHHelper::setLoggedIn(bool v)
{
    if (d->is_logged_in == v)
        return;

    d->is_logged_in = v;
    emit loggedInChanged();
}

QString SSHHelper::errorString() const
{
    return d->error_string;
}

void SSHHelper::setErrorString(QString v)
{
    if (d->error_string == v)
        return;

    d->error_string = v;
    emit errorStringChanged();
}

int SSHHelper::errorCode() const
{
    return d->error_code;
}

void SSHHelper::setErrorCode(int v)
{
    if (d->error_code == v)
        return;

    d->error_code = v;
    emit errorCodeChanged();;
}

bool SSHHelper::connectToHost(const QString &host, const quint16 &port)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, host, port]() mutable {
        if (d->sock != -1) {
            PRINT_ERRORSTRING("No need to repeat the connection");
            return;
        }

#ifdef WIN32
        WSADATA wsa;
        // initialize winsock service
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            PRINT_ERRORSTRING("WSAStartup failed with error");
            return;
        }
#endif

        d->sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        sin.sin_addr.s_addr = inet_addr(qPrintable(host));
        state = ::connect(d->sock, (sockaddr*)(&sin), sizeof(sockaddr_in)) == 0;
        if (!state)
            PRINT_ERRORSTRING("Host connection failed");
    }, true);

    return state;
}

void SSHHelper::disconnectFromHost()
{
    if (d->sock == LIBSSH2_INVALID_SOCKET)
        return;

    d->worker_thread.runOnWorkerThread([this]() mutable {
        shutdown(d->sock, 2);
#ifdef WIN32
        closesocket(d->sock);
#else
        close(d->sock);
#endif
        d->sock = LIBSSH2_INVALID_SOCKET;

        setLoggedIn(false);
    }, true);
}

bool SSHHelper::login(const QString &username, const QString &password)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, username, password]() mutable {
        if (d->session != NULL) {
            PRINT_ERRORSTRING("No need to repeat login");
            return;
        }

        d->session = libssh2_session_init();

        if (libssh2_session_handshake(d->session, d->sock)) {
            PRINT_ERRORSTRING("libssh2_session_handshake error: %d", libssh2_session_last_errno(d->session));
            return;
        }

        libssh2_hostkey_hash(d->session, LIBSSH2_HOSTKEY_HASH_SHA256);
        libssh2_userauth_list(d->session, qPrintable(username), username.size());

        int rc = libssh2_userauth_password(d->session, qPrintable(username), qPrintable(password));
        if (rc == 0) {
            state = true;
            setLoggedIn(true);
        }
        else {
            PRINT_ERRORSTRING("libssh2_userauth_password error: %d", rc);
            libssh2_session_free(d->session);
            d->session = NULL;
        }
    }, true);

    return state;
}

bool SSHHelper::login(const QString &username, const QString &publickey,
                      const QString &privatekey, const QString &passphrase)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, username, publickey, privatekey, passphrase]() mutable {
        if (d->session != NULL) {
            PRINT_ERRORSTRING("No need to repeat login");
            return;
        }

        d->session = libssh2_session_init();

        if (libssh2_session_handshake(d->session, d->sock)) {
            PRINT_ERRORSTRING("libssh2_session_handshake error: %d", libssh2_session_last_errno(d->session));
            return;
        }

        auto pubkeyData = Utils::readFromFile(publickey);
        auto prikeyData = Utils::readFromFile(privatekey);
        int rc = libssh2_userauth_publickey_frommemory(d->session, qPrintable(username), username.size(),
                                                       pubkeyData, pubkeyData.size(),
                                                       prikeyData, prikeyData.size(),
                                                       qPrintable(passphrase));
        if (rc == 0) {
            state = true;
            setLoggedIn(true);
        }
        else {
            PRINT_ERRORSTRING("libssh2_userauth_publickey_frommemory error: %d", rc);
            libssh2_session_free(d->session);
            d->session = NULL;
        }
    }, true);

    return state;
}

void SSHHelper::logout()
{
    if (d->session == NULL)
        return;

    d->worker_thread.runOnWorkerThread([=]() mutable {
        if (d->session == NULL)
            return;

        libssh2_session_disconnect(d->session, "Normal Shutdown");
        libssh2_session_free(d->session);
        d->session = NULL;

        setLoggedIn(false);
    }, true);
}

bool SSHHelper::sshExec(const QString &cmd, QByteArray &data)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, cmd, &data] () mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(d->session);
        if (!channel) {
            PRINT_ERRORSTRING("libssh2_channel_open_session error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> channel_guard(nullptr, [channel](void*){
            libssh2_channel_close(channel);
            int exitcode = libssh2_channel_get_exit_status(channel);
            char *exitsignal = (char *)"none";
            libssh2_channel_get_exit_signal(channel, &exitsignal,
                                            NULL, NULL, NULL, NULL, NULL);

            libssh2_channel_free(channel);
        });

        int rc = libssh2_channel_exec(channel, qUtf8Printable(cmd));
        if (rc != 0) {
            PRINT_ERRORSTRING("libssh2_channel_exec error: %d", rc);
            return;
        }

        char buffer[0x4000];
        ssize_t nread;
        do {
            nread = libssh2_channel_read(channel, buffer, sizeof(buffer));
            if (nread > 0) {
                data.append(buffer, nread);
            }
        } while (nread > 0);

        state = true;
    }, true);

    return state;
}

bool SSHHelper::scpDownload(const QString &from, const QString &to, SSHDonwloadProgressFunc func)
{
    bool state = false;
    d->abort_scp = false;
    d->worker_thread.runOnWorkerThread([this, &state, from, to, func]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        int seq = d->seq++;
        LOGTD2(seq, "begin: %s >> %s", qUtf8Printable(from), qUtf8Printable(to));
        std::shared_ptr<void> guard(NULL, [seq, &state](void*){
            LOGTD2(seq, "end: %s", state ? "true" : "false");
        });

        libssh2_struct_stat fileinfo;
        LIBSSH2_CHANNEL *channel = libssh2_scp_recv2(d->session, qUtf8Printable(from), &fileinfo);
        if (!channel) {
            PRINT_ERRORSTRING("libssh2_scp_recv2 error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> channel_guard(NULL, [channel](void*){
            libssh2_channel_free(channel);
        });

        QFile to_file(to);
        if (to_file.open(QIODevice::WriteOnly)) {
            qint64 bytes_received = 0;
            qint64 bytes_total = fileinfo.st_size;

            QElapsedTimer elapsed;
            elapsed.start();

            if (func) func(bytes_received, bytes_total);

            char mem[1024];
            while (!d->abort_scp && bytes_received < bytes_total) {
                int amount = sizeof(mem);

                if((bytes_total - bytes_received) < amount)
                    amount = (int)(bytes_total - bytes_received);

                ssize_t nread = libssh2_channel_read(channel, mem, amount);
                if (nread > 0) {
                    to_file.write(mem, nread);
                }
                else if(nread < 0) {
                    PRINT_ERRORSTRING("libssh2_channel_read error: %d", nread);
                    break;
                }

                bytes_received += nread;

                if (func && elapsed.elapsed() > 1000) {
                    elapsed.restart();
                    func(bytes_received, bytes_total);
                }
            }

            if (func) func(bytes_received, bytes_total);

            to_file.close();
            state = bytes_received == bytes_total;
        }
        else {
            PRINT_ERRORSTRING("open error: %s", qUtf8Printable(to));
        }
    }, true);

    return state;
}

bool SSHHelper::scpUpload(const QString &from, const QString &to, SSHUploadProgressFunc func)
{
    bool state = false;
    d->abort_scp = false;
    d->worker_thread.runOnWorkerThread([this, &state, from, to, func]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        int seq = d->seq++;
        LOGTD2(seq, "begin: %s >> %s", qUtf8Printable(from), qUtf8Printable(to));
        std::shared_ptr<void> guard(NULL, [seq, &state](void*){
            LOGTD2(seq, "end: %s", state ? "true" : "false");
        });

        QFile from_file(from);
        if (from_file.open(QIODevice::ReadOnly)) {
            LIBSSH2_CHANNEL *channel = libssh2_scp_send(d->session, qUtf8Printable(to), from_file.permissions() & 0777, (long)from_file.size());
            if (!channel) {
                PRINT_ERRORSTRING("libssh2_scp_send error: %d", libssh2_session_last_errno(d->session));
                return;
            }
            std::shared_ptr<void> channel_guard(NULL, [channel](void*) {
                libssh2_channel_send_eof(channel);
                libssh2_channel_wait_eof(channel);
                libssh2_channel_wait_closed(channel);
                libssh2_channel_free(channel);
            });

            qint64 bytes_sent = 0;
            qint64 bytes_total = from_file.size();

            QElapsedTimer elapsed;
            elapsed.start();

            if (func) func(bytes_sent, bytes_total);

            while (!d->abort_scp && !from_file.atEnd()) {
                auto data = from_file.read(1024);
                size_t nread = data.size();
                char *ptr = data.data();

                while (!d->abort_scp && nread > 0) {
                    ssize_t nwritten = libssh2_channel_write(channel, ptr, nread);
                    if (nwritten < 0) {
                        PRINT_ERRORSTRING("libssh2_channel_write error: %d", nwritten);
                        break;
                    }
                    else {
                        /* nwritten indicates how many bytes were written this time */
                        ptr += nwritten;
                        nread -= nwritten;
                        bytes_sent += nwritten;
                    }
                }

                if (func && elapsed.elapsed() > 1000) {
                    elapsed.restart();
                    func(bytes_sent, bytes_total);
                }
            }

            if (func) func(bytes_sent, bytes_total);

            from_file.close();
            state = bytes_sent = bytes_total;
        }
        else {
            PRINT_ERRORSTRING("open error: %s", qUtf8Printable(from));
        }
    }, true);

    return state;
}

void SSHHelper::scpAbort()
{
    d->abort_scp = true;
}

QList<SSHHelper::FileInfo> SSHHelper::sftpList(const QString &path, bool *ok)
{
    if (ok) *ok = false;

    QList<FileInfo> list;
    d->worker_thread.runOnWorkerThread([this, &list, path, ok]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_opendir(sftp_session, qUtf8Printable(path));
        if (!sftp_handle) {
            PRINT_ERRORSTRING("libssh2_sftp_opendir error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> handle_guard(NULL, [sftp_handle](void*){
            libssh2_sftp_close_handle(sftp_handle);
        });

        do {
            char mem[512];
            char longentry[512];
            LIBSSH2_SFTP_ATTRIBUTES attrs;

            /* loop until we fail */
            int rc = libssh2_sftp_readdir_ex(sftp_handle, mem, sizeof(mem), longentry, sizeof(longentry), &attrs);
            if (rc > 0) {
                if(mem[0] != '\0') {
                    FileInfo info;
                    info.path = path;
                    info.name = mem;
                    info.filesize = attrs.filesize;
                    info.mtime = QDateTime::fromSecsSinceEpoch(attrs.mtime);
                    info.atime = QDateTime::fromSecsSinceEpoch(attrs.atime);
                    info.folder = longentry[0] == 'd';
                    list.append(info);
                }
            }
            else {
                break;
            }
        } while(1);

        *ok = true;
    }, true);

    return list;
}

bool SSHHelper::sftpRead(const QString &from, QByteArray &data)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, &data, from]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_open(sftp_session, qUtf8Printable(from),
                                                             LIBSSH2_FXF_READ, 0);
        if (!sftp_handle) {
            PRINT_ERRORSTRING("libssh2_sftp_opendir error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> handle_guard(NULL, [sftp_handle](void*){
            libssh2_sftp_close_handle(sftp_handle);
        });

        char mem[1024];
        do {
            /* loop until we fail */
            ssize_t nread = libssh2_sftp_read(sftp_handle, mem, sizeof(mem));
            if (nread > 0) {
                data.append(mem, nread);
            }
            else {
                break;
            }
        } while(1);

        state = true;
    }, true);

    return state;
}

bool SSHHelper::sftpWrite(const QString &to, const QByteArray &data)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, data, to]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_open(sftp_session, qUtf8Printable(to),
                                                             LIBSSH2_FXF_WRITE |
                                                             LIBSSH2_FXF_CREAT |
                                                             LIBSSH2_FXF_TRUNC,
                                                             LIBSSH2_SFTP_S_IRUSR |
                                                             LIBSSH2_SFTP_S_IWUSR |
                                                             LIBSSH2_SFTP_S_IRGRP |
                                                             LIBSSH2_SFTP_S_IROTH);
        if (!sftp_handle) {
            PRINT_ERRORSTRING("libssh2_sftp_opendir error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> handle_guard(NULL, [sftp_handle](void*){
            libssh2_sftp_close_handle(sftp_handle);
        });

        const char *ptr = data.data();
        size_t nread = data.size();
        do {
            /* write data in a loop until we block */
            ssize_t nwritten = libssh2_sftp_write(sftp_handle, ptr, nread);
            if (nwritten < 0) {
                PRINT_ERRORSTRING("libssh2_sftp_write error: %d", nwritten);
                break;
            }
            ptr += nwritten;
            nread -= nwritten;
        } while(nread);

        state = nread == 0;
    }, true);

    return state;
}

bool SSHHelper::sftpRemove(const QString &filename)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, filename]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        int rc = libssh2_sftp_unlink(sftp_session, qUtf8Printable(filename));
        if (rc != 0)
            PRINT_ERRORSTRING("libssh2_sftp_unlink error: %d", rc);
        state = rc == 0;
    }, true);

    return state;
}

bool SSHHelper::sftpRename(const QString &source, const QString &target)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, source, target]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        int rc = libssh2_sftp_rename(sftp_session, qUtf8Printable(source), qUtf8Printable(target));
        if (rc != 0)
            PRINT_ERRORSTRING("libssh2_sftp_rename error: %d", rc);
        state = rc == 0;
    }, true);

    return state;
}

bool SSHHelper::sftpMkdir(const QString &path)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([this, &state, path]() mutable {
        if (d->session == NULL) {
            PRINT_ERRORSTRING("Please login to SSH first");
            return;
        }

        LIBSSH2_SFTP *sftp_session = libssh2_sftp_init(d->session);
        if (!sftp_session) {
            PRINT_ERRORSTRING("libssh2_sftp_init error: %d", libssh2_session_last_errno(d->session));
            return;
        }
        std::shared_ptr<void> session_guard(NULL, [sftp_session](void*){
            libssh2_sftp_shutdown(sftp_session);
        });

        /* Make a directory via SFTP */
        int rc = libssh2_sftp_mkdir(sftp_session, qUtf8Printable(path),
                                    LIBSSH2_SFTP_S_IRWXU |
                                        LIBSSH2_SFTP_S_IRGRP |
                                        LIBSSH2_SFTP_S_IXGRP |
                                        LIBSSH2_SFTP_S_IROTH |
                                        LIBSSH2_SFTP_S_IXOTH);

        if (rc != 0)
            PRINT_ERRORSTRING("libssh2_sftp_mkdir error: %d", rc);
        state = rc == 0;
    }, true);

    return state;
}

#ifdef HAVE_QUICK
void SSHHelper::qLogin(QString host, int port, QString username, QString password, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, host, port, username, password, callback_id]() mutable {
        QString msg;
        bool state = connectToHost(host, port);
        if (state) {
            state = login(username, password);
        }

        if (!state) {
            disconnectFromHost();
            msg = d->error_string;
        }

        QmlUtils::invokeMethod(this, callback_id, {state, msg});
    });
}

void SSHHelper::qLogout(QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, callback_id]() mutable {
        bool state = true;
        logout();
        disconnectFromHost();
        QmlUtils::invokeMethod(this, callback_id, {state});
    });
}

void SSHHelper::qSftpList(QString path, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, path, callback_id]() mutable {
        QString msg;
        bool state = false;

        auto list = sftpList(path, &state);

        std::sort(list.begin(), list.end(), [](const FileInfo &l, const FileInfo &r){
            if (l.folder != r.folder)
                return l.folder;
            else
                return l.name < r.name;
        });

        QVariantList __list;
        for (auto &it: list) {
            __list.append(QVariantMap {
                { "path", it.path },
                { "name", it.name },
                { "filesize", Utils::formatFileSize(it.filesize) },
                { "mtime", it.mtime.toString("yyyy-MM-dd hh:mm:ss") },
                { "atime", it.atime.toString("yyyy-MM-dd hh:mm:ss") },
                { "folder", it.folder },
            });
        }

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg, __list});
    });
}

void SSHHelper::qSftpRemove(QString filename, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, filename, callback_id]() mutable {
        QString msg;
        bool state = sftpRemove(filename);

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg});
    });
}

void SSHHelper::qSftpRename(QString source, QString target, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, source, target, callback_id]() mutable {
        QString msg;
        bool state = sftpRename(source, target);

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg});
    });
}

void SSHHelper::qSftpMkdir(QString path, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, path, callback_id]() mutable {
        QString msg;
        bool state = sftpMkdir(path);

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg});
    });
}

void SSHHelper::qScpUpload(QString local_file, QString remote_path, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, local_file, remote_path, callback_id]() mutable {
        auto name = QFileInfo(local_file).fileName();
        QString to = cleanPath(remote_path+"/"+name);
        QString msg;
        bool state = scpUpload(local_file, to);

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg});
    });
}

void SSHHelper::qScpDownload(QString remote_file, QString local_path, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, remote_file, local_path, callback_id]() mutable {
        auto name = QFileInfo(remote_file).fileName();
        QString to = cleanPath(local_path+"/"+name);
        QString msg;
        bool state = scpDownload(remote_file, to);

        if (!state)
            msg = d->error_string;

        QmlUtils::invokeMethod(this, callback_id, {state, msg, to});
    });
}

QString SSHHelper::cleanPath(QString path)
{
    QString clean_path = path;
    while (clean_path.contains("//"))
        clean_path = clean_path.replace("//", "/");
    while (clean_path.contains("/./"))
        clean_path = clean_path.replace("/./", "/");

    if (clean_path.startsWith("/.."))
        clean_path = "/"+clean_path.mid(3);

    if (clean_path == "/.")
        clean_path = "/";

    clean_path = QDir::cleanPath(clean_path);
    return clean_path;
}

bool SSHHelper::isUploadAllowed(QString filename)
{
    return QFileInfo(filename).isFile();
}
#endif
