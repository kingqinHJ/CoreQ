#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>

#include <QMap>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_CONFIG(http)
#include <QHttpMultiPart>
#endif

#define s_http HttpManager::instance()

#define REPLY_SUCCESS(reply) reply->error() == QNetworkReply::NoError

class HttpManagerReplyWrapper;
using HttpManagerReplyWrapperPtr = std::shared_ptr<HttpManagerReplyWrapper>;
using HttpManagerHeaders = QMap<QByteArray, QByteArray>;

class HttpManagerRequest;
class HttpManagerPrivate;
class HttpManager : public QObject
{
    Q_OBJECT
public:
    enum PRINT_MODE
    {
        PRINT_NONE = 0x0000,
        PRINT_REQUEST = 0x0001,
        PRINT_RESPONSE = 0x0002,
        PRINT_ALL = PRINT_REQUEST | PRINT_RESPONSE,
    };

    enum METHOD {
        HTTP_GET,
        HTTP_POST,
        HTTP_DELETE,
    };

    explicit HttpManager(QObject *parent = nullptr);
    ~HttpManager();

    bool start();
    void stop();
    void abort();

    HttpManagerReplyWrapperPtr send(const HttpManagerRequest &req);

    HttpManagerReplyWrapperPtr get (QString url, HttpManagerHeaders headers = {},
                                   int mode = PRINT_NONE);
    HttpManagerReplyWrapperPtr post(QString url, QByteArray body = QByteArray(),
                                    HttpManagerHeaders headers = {},
                                    int mode = PRINT_NONE);
    HttpManagerReplyWrapperPtr post(QString url, QIODevice *data,
                                    HttpManagerHeaders headers = {},
                                    int mode = PRINT_NONE);
#if QT_CONFIG(http)
    HttpManagerReplyWrapperPtr post(QString url, QHttpMultiPart *multi_part,
                                    HttpManagerHeaders headers = {},
                                    int mode = PRINT_NONE);
#endif
    HttpManagerReplyWrapperPtr deleteResource(QString url, HttpManagerHeaders headers = {},
                                              int mode = PRINT_NONE);

    static HttpManager *instance() { return self; }

private:
    static HttpManager *self;
    std::shared_ptr<HttpManagerPrivate> d;
};

using HandleFinishedFunc = std::function<void(QNetworkReply*)>;
using HandleReadyReadFunc = std::function<void(QNetworkReply*)>;
using HandleDonwloadProgressFunc = std::function<void(qint64, qint64)>;
using HandleUploadProgressFunc = std::function<void(qint64, qint64)>;

class HttpManagerReplyWrapperPrivate;
class HttpManagerReplyWrapper : public QObject
{
public:
    HttpManagerReplyWrapper(QNetworkReply *reply,
                            int id = -1,
                            int connect_timeout = -1,
                            int data_timeout = -1,
                            QObject *parent = nullptr);
    ~HttpManagerReplyWrapper();

    QNetworkReply *reply();

    int connectTimeout() const;
    void setConnectTimeout(int v);

    int dataTimeout() const;
    void setDataTimeout(int v);

    QByteArray body() const;
    void setBody(const QByteArray &v);

    HttpManagerReplyWrapper* onFinished(HandleFinishedFunc func);
    HttpManagerReplyWrapper* onReadyRead(HandleReadyReadFunc func);
    HttpManagerReplyWrapper* onDonwloadProgress(HandleDonwloadProgressFunc func);
    HttpManagerReplyWrapper* onUploadProgress(HandleUploadProgressFunc func);

    int id();
    void reset();
    void abort();
#ifndef Q_OS_WASM
    void waitForFinished();
#endif

    qint64 elapsed() const;

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    std::shared_ptr<HttpManagerReplyWrapperPrivate> d;
};

class HttpManagerRequest {
public:
    int method = HttpManager::HTTP_GET;
    QString url;
    HttpManagerHeaders headers;
    int print_mode = HttpManager::PRINT_NONE;

    QByteArray body;
    QIODevice *dev = nullptr;
    QHttpMultiPart *multi_part = nullptr;
};

#endif // HTTPMANAGER_H
