#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QObject>
#include <QVariant>
#include "CommunicationInterface.h"

using ResultCallback = std::function<void(int, QVariant)>; // <ecode, data>

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CLDEVICE_EXPORT ProtocolHandler : public QObject
{
    Q_OBJECT
public:
    virtual void attach(CommunicationInterfaceWPtr comm, const DeviceInfo &info) = 0;
    // virtual void detach(CommunicationInterfacePtr comm) = 0;

    virtual void upload(const QByteArray &data, ResultCallback cb = nullptr) = 0;
    virtual void preview(ResultCallback cb = nullptr) = 0;
    virtual void start(ResultCallback cb = nullptr) = 0;
    virtual void pause(ResultCallback cb = nullptr) = 0;
    virtual void resume(ResultCallback cb = nullptr) = 0;
    virtual void stop(ResultCallback cb = nullptr) = 0;
    virtual void home(int axis = XY_AXIS, ResultCallback cb = nullptr) = 0;
    virtual void move(int axis, qreal distance, qreal speed = 1000, ResultCallback cb = nullptr) = 0;

    virtual void sendCommand(QStringList cmds, ResultCallback cb = nullptr) = 0;
    virtual void moveXY(qreal x, qreal y, bool absolute, qreal speed = 1000, ResultCallback cb = nullptr) = 0;

    virtual void autoFocus(ResultCallback cb = nullptr) = 0;
    virtual void laserOn(int power, ResultCallback cb = nullptr) = 0;
    virtual void laserOff(ResultCallback cb = nullptr) = 0;

    // [0-100]
    virtual void getExposure(ResultCallback cb = nullptr) = 0;
    virtual void setExposure(int value, ResultCallback cb = nullptr) = 0;

    virtual void getFireDetectLevel(ResultCallback cb = nullptr) = 0;
    virtual void setFireDetectLevel(FireDetectLevel level, ResultCallback cb = nullptr) = 0;

    virtual void getCameraConfig(ResultCallback cb = nullptr) = 0;
    virtual void setCameraConfig(QByteArray data, ResultCallback cb = nullptr) = 0;

    virtual DeviceStatus status() const = 0;

signals:
    void disconnected();
    void deviceStatusChanged();
};

using ProtocolHandlerPtr = std::shared_ptr<ProtocolHandler>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // PROTOCOLHANDLER_H
