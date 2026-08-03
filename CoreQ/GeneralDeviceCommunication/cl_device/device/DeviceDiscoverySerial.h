#ifndef DEVICEDISCOVERYSERIAL_H
#define DEVICEDISCOVERYSERIAL_H

#include "DeviceDiscovery.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceDiscoverySerialPrivate;
class CLDEVICE_EXPORT DeviceDiscoverySerial : public DeviceDiscovery
{
    Q_OBJECT
public:
    DeviceDiscoverySerial();
    ~DeviceDiscoverySerial();

    bool start() override;
    void stop() override;
    bool isRunning() override;

protected:
    void timerEvent(QTimerEvent *e) override;

public:
    void updateDevice(bool delay = true);

private:
    std::shared_ptr<DeviceDiscoverySerialPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEDISCOVERYSERIAL_H
