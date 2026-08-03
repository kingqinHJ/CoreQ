#ifndef COMMUNICATIONSERIAL_H
#define COMMUNICATIONSERIAL_H

#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CommunicationSerialPrivate;
class CLDEVICE_EXPORT CommunicationSerial : public CommunicationInterface
{
    Q_OBJECT
public:
    CommunicationSerial();
    ~CommunicationSerial();

    ConnectionType type() const override { return CT_Serial; }

    QString address() const override;
    void setAddress(const QString &address) override;

    void open() override;
    void close() override;
    int send(const QByteArray &data) override;
    bool receive(QByteArray &data) override;

    void onResponse(int seq, ResponseCallback callback, QObject *receiver = nullptr) override;

    void clearAllPendingCallbacks() override;

    void setResponseFilter(ResponseFilter filter) override;

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onErrorOccurred();

    void checkResponse();
    void invokeCallback();

signals:

private:
    std::shared_ptr<CommunicationSerialPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // COMMUNICATIONSERIAL_H
