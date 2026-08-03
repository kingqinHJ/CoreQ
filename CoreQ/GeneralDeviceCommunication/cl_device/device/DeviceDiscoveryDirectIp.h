#ifndef DEVICEDISCOVERYDIRECTIP_H
#define DEVICEDISCOVERYDIRECTIP_H

#include <QObject>

class DeviceDiscoveryDirectIp : public QObject
{
    Q_OBJECT
public:
    explicit DeviceDiscoveryDirectIp(QObject *parent = nullptr);

signals:
};

#endif // DEVICEDISCOVERYDIRECTIP_H
