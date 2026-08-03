#include "CommunicationHttp.h"
#include "global/HttpManager.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

struct ResponseInfo
{
    int seq = 0;
    QObject *receiver;
    ResponseCallback callback;
};

class CommunicationHttpPrivate
{
public:
    CommunicationHttp *q;

    int seq = 1;

    QString address;
    std::shared_ptr<HttpManager> http;

    QMap<int, ResponseInfo> resMap;

    ResponseFilter filter;

    QString makeUrl(const QString &path) {
        return QString("http://%1%2").arg(address, path);
    }

    void invokeCallback(int seq, QByteArray body);
};

void CommunicationHttpPrivate::invokeCallback(int seq, QByteArray body)
{
    QMetaObject::invokeMethod(q, [=](){
        ResponseInfo res;
        if (resMap.contains(seq))
            res = resMap.take(seq);
        if (res.callback) {
            if (res.receiver) {
                QMetaObject::invokeMethod(res.receiver, [cb = res.callback, body]() {
                    cb(body);
                });
            }
            else {
                res.callback(body);
            }
        }
    });
}

CommunicationHttp::CommunicationHttp()
{
    d.reset(new CommunicationHttpPrivate);
    d->q = this;
}

CommunicationHttp::~CommunicationHttp()
{
    CommunicationHttp::close();
    LOGD("%s", qUtf8Printable(d->address));
}

void CommunicationHttp::open()
{
    if (!d->http) {
        d->http = std::make_shared<HttpManager>();
        d->http->start();
    }
}

void CommunicationHttp::close()
{
    if (d->http) {
        d->http->stop();
        d->http.reset();
    }
}

QString CommunicationHttp::address() const
{
    return d->address;
}

void CommunicationHttp::setAddress(const QString &address)
{
    d->address = address;
}

int CommunicationHttp::get(const QString &path, HttpHeaders headers, int connectTimeout, int dataTimeout)
{
    if (!d->http)
        return 0;

    auto task = d->http->get(d->makeUrl(path), headers);
    int seq = d->seq++;
    task->setConnectTimeout(connectTimeout);
    task->setDataTimeout(dataTimeout);
#ifdef CL_ENABLE_HTTP_PRINT
    LOGD("send(%s):", qUtf8Printable(d->address)) << path;
#endif
    task->onFinished([this, seq](QNetworkReply *reply) {
        QByteArray body;
        if (reply->error() == QNetworkReply::NoError) {
            body = reply->readAll();
            if (d->filter) d->filter(body);
#ifdef CL_ENABLE_HTTP_PRINT
            if (body.size() < 1024)
                LOGD("recv(%s):", qUtf8Printable(d->address)) << body.data();
            else
                LOGD("recv(%s):", qUtf8Printable(d->address)) << body.size();
#endif
        }

        d->invokeCallback(seq, body);
    });
    return seq;
}

int CommunicationHttp::post(const QString &path, const QByteArray &body, HttpHeaders headers, int connectTimeout, int dataTimeout)
{
    if (!d->http)
        return 0;

    auto task = d->http->post(d->makeUrl(path), body, headers);
    int seq = d->seq++;
    task->setConnectTimeout(connectTimeout);
    task->setDataTimeout(dataTimeout);
#ifdef CL_ENABLE_HTTP_PRINT
    if (!body.isEmpty() && body.size() < 1024)
        LOGD("send(%s):", qUtf8Printable(d->address)) << path << "\n" << body.data();
    else
        LOGD("send(%s):", qUtf8Printable(d->address)) << path << body.size();
#endif
    task->onFinished([this, seq](QNetworkReply *reply) {
        QByteArray body;
        if (reply->error() == QNetworkReply::NoError) {
            body = reply->readAll();
            if (d->filter) d->filter(body);
#ifdef CL_ENABLE_HTTP_PRINT
            if (body.size() < 1024)
                LOGD("recv(%s):", qUtf8Printable(d->address)) << body.data();
            else
                LOGD("recv(%s):", qUtf8Printable(d->address)) << body.size();
#endif
        }

        d->invokeCallback(seq, body);
    });
    return seq;
}

void CommunicationHttp::onResponse(int seq, ResponseCallback callback, QObject *receiver)
{
    if (!callback)
        return;

    d->resMap.insert(seq, {seq, receiver, callback});
}

void CommunicationHttp::clearAllPendingCallbacks()
{
    d->resMap.clear();
}

void CommunicationHttp::setResponseFilter(ResponseFilter filter)
{
    d->filter = filter;
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
