#ifndef DEVICECONFIGMANAGER_H
#define DEVICECONFIGMANAGER_H

#include <QObject>
#include "DeviceDef.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceConfigManagerPrivate;
class CLDEVICE_EXPORT DeviceConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceConfigManager(QObject *parent = nullptr);
    ~DeviceConfigManager();

    QString dataPath() const;
    void setDataPath(QString v);

    DeviceConfig getConfig(const QString &deviceId) const;
    void updateConfig(const DeviceConfig &config);

protected:
    void timerEvent(QTimerEvent *e) override;

signals:

private:
    std::shared_ptr<DeviceConfigManagerPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICECONFIGMANAGER_H
