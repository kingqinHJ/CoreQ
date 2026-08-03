#ifndef DEVICEPROBERHTTP_H
#define DEVICEPROBERHTTP_H

#include <QObject>
#include "DeviceProber.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceProberHttpPrivate;
class CLDEVICE_EXPORT DeviceProberHttp : public DeviceProber
{
public:
    DeviceProberHttp();
    ~DeviceProberHttp();

    void probe(CommunicationInterfaceWPtr comm, const DeviceIdentifier &id) override;
    void flush(QString address) override;

private:
    void onResponse(const QString &key, const QByteArray &data);

private:
    std::shared_ptr<DeviceProberHttpPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEPROBERHTTP_H
