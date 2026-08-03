#ifndef DEVICEPROBER_H
#define DEVICEPROBER_H

#include <QObject>
#include "CommunicationInterface.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CLDEVICE_EXPORT DeviceProber : public QObject
{
    Q_OBJECT
public:
    virtual void probe(CommunicationInterfaceWPtr comm, const DeviceIdentifier &id) = 0;
    virtual void flush(QString address) = 0;

signals:
    void probeSucceeded(const DeviceInfo &info);
    void probeFailed(const DeviceIdentifier &id);
};

using DeviceProberPtr = std::shared_ptr<DeviceProber>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICEPROBER_H
