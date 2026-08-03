#ifndef DEVICECAPABILITYMANAGER_H
#define DEVICECAPABILITYMANAGER_H

#include <QObject>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceCapabilityManagerPrivate;
class CLDEVICE_EXPORT DeviceCapabilityManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceCapabilityManager(QObject *parent = nullptr);
    ~DeviceCapabilityManager();

    DeviceCapability getCapability(DeviceType model) const;
    QString getModelName(DeviceType model) const;

    QList<DeviceCapability> getCapabilityList() const;

signals:

private:
    std::shared_ptr<DeviceCapabilityManagerPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICECAPABILITYMANAGER_H
