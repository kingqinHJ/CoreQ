#ifndef DEVICEDISCOVERYUDP_H
#define DEVICEDISCOVERYUDP_H

#include "DeviceDiscovery.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceDiscoveryUdpPrivate;
class CLDEVICE_EXPORT DeviceDiscoveryUdp : public DeviceDiscovery
{
    Q_OBJECT
public:
    DeviceDiscoveryUdp();
    ~DeviceDiscoveryUdp();

    bool start() override;
    void stop() override;
    bool isRunning() override;

private:
    std::shared_ptr<DeviceDiscoveryUdpPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEDISCOVERYUDP_H
