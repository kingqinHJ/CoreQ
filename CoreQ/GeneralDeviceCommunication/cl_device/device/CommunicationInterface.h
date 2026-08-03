#ifndef COMMUNICATIONINTERFACE_H
#define COMMUNICATIONINTERFACE_H

#include <QObject>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

using ResponseCallback = std::function<void(const QByteArray&)>;
using ResponseFilter = std::function<void(const QByteArray&)>;
using HttpHeaders = QMap<QByteArray, QByteArray>;

class CLDEVICE_EXPORT CommunicationInterface : public QObject
{
    Q_OBJECT
public:
    virtual ConnectionType type() const = 0;

    virtual QString address() const = 0;
    virtual void setAddress(const QString &address) = 0;

    virtual void open() {}
    virtual void close() {}

    // Stream
    virtual int send(const QByteArray &data) { return false; }
    virtual bool receive(QByteArray &data) { return false; }

    // Http
    virtual int get(const QString &path, HttpHeaders headers = HttpHeaders(), int connectTimeout = 5000, int dataTimeout = 5000) { return false; }
    virtual int post(const QString &path, const QByteArray &body, HttpHeaders headers = HttpHeaders(), int connectTimeout = 5000, int dataTimeout = 5000) { return false; }

    virtual void onResponse(int seq, ResponseCallback callback, QObject *receiver = nullptr) = 0;

    virtual void clearAllPendingCallbacks() = 0;

    // 直接回调，非事件回调，注意跨线程处理
    virtual void setResponseFilter(ResponseFilter filter) { }

signals:
    void communicationError();
};

using CommunicationInterfacePtr = std::shared_ptr<CommunicationInterface>;
using CommunicationInterfaceWPtr = std::weak_ptr<CommunicationInterface>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // COMMUNICATIONINTERFACE_H
