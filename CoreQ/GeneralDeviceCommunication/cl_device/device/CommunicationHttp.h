#ifndef COMMUNICATIONHTTP_H
#define COMMUNICATIONHTTP_H

#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CommunicationHttpPrivate;
class CLDEVICE_EXPORT CommunicationHttp : public CommunicationInterface
{
    Q_OBJECT
public:
    CommunicationHttp();
    ~CommunicationHttp();

    void open() override;
    void close() override;

    ConnectionType type() const override { return CT_Http; }

    QString address() const override;
    void setAddress(const QString &address) override;

    int get(const QString &path, HttpHeaders headers = HttpHeaders(), int connectTimeout = 10000, int dataTimeout = 10000) override;
    int post(const QString &path, const QByteArray &body, HttpHeaders headers = HttpHeaders(), int connectTimeout = 10000, int dataTimeout = 10000) override;

    void onResponse(int seq, ResponseCallback callback, QObject *receiver = nullptr) override;

    void clearAllPendingCallbacks() override;

    void setResponseFilter(ResponseFilter filter) override;

signals:

private:
    std::shared_ptr<CommunicationHttpPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // COMMUNICATIONHTTP_H
