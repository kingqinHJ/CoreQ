#ifndef DEVICEDISCOVERY_H
#define DEVICEDISCOVERY_H

#include <QObject>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CLDEVICE_EXPORT DeviceDiscovery : public QObject
{
    Q_OBJECT
public:
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;

signals:
    void deviceDiscovered(const DeviceIdentifier &id);
    void deviceRemoved(const DeviceIdentifier &id);
};

using DeviceDiscoveryPtr = std::shared_ptr<DeviceDiscovery>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEDISCOVERY_H
