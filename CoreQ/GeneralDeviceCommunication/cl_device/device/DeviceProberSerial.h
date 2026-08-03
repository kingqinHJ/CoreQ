#ifndef DEVICEPROBERSERIAL_H
#define DEVICEPROBERSERIAL_H

#include <QObject>
#include "DeviceProber.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceProberSerialPrivate;
class CLDEVICE_EXPORT DeviceProberSerial : public DeviceProber
{
public:
    DeviceProberSerial();
    ~DeviceProberSerial();

    void probe(CommunicationInterfaceWPtr comm, const DeviceIdentifier &id) override;
    void flush(QString address) override;

private:
    void onResponseInfoCmd(const QString &key, const QByteArray &data);
    void onResponsePropertyCmd(const QString &key, const QByteArray &data);
    void onResponseInitCmd(const QString &key, const QByteArray &data);

private:
    std::shared_ptr<DeviceProberSerialPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEPROBERSERIAL_H
