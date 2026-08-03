#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include <QObject>
#include "DeviceDef.h"

using ResultCallback = std::function<void(int, QVariant)>; // <ecode, data>

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class CommunicationManager;

class DeviceControllerPrivate;
class CLDEVICE_EXPORT DeviceController : public QObject
{
    Q_OBJECT
public:
    explicit DeviceController(const DeviceInfo &info, CommunicationManager *commMgr);
    ~DeviceController();

    void upload(const QByteArray &data, ResultCallback cb = nullptr);
    void preview(ResultCallback cb = nullptr);
    void start(ResultCallback cb = nullptr);
    void pause(ResultCallback cb = nullptr);
    void resume(ResultCallback cb = nullptr);
    void stop(ResultCallback cb = nullptr);
    void home(int axis = XY_AXIS, ResultCallback cb = nullptr);
    void move(int axis, qreal distance, qreal speed = 1000, ResultCallback cb = nullptr);

    void sendCommand(QStringList cmds, ResultCallback cb = nullptr);
    void moveXY(qreal x, qreal y, bool absolute, qreal speed = 1000, ResultCallback cb = nullptr);

    void autoFocus(ResultCallback cb = nullptr);
    void laserOn(int power, ResultCallback cb = nullptr);
    void laserOff(ResultCallback cb = nullptr);

    void getExposure(ResultCallback cb = nullptr);
    void setExposure(int value, ResultCallback cb = nullptr);

    void getFireDetectLevel(ResultCallback cb = nullptr);
    void setFireDetectLevel(FireDetectLevel level, ResultCallback cb = nullptr);

    void getCameraConfig(ResultCallback cb = nullptr);
    void setCameraConfig(QByteArray data, ResultCallback cb = nullptr);

    DeviceInfo info() const;
    DeviceConfig config() const;
    void setConfig(const DeviceConfig &v);
    DeviceStatus status() const;

signals:
    void disconnected();
    void deviceStatusChanged();
    void deviceConfigChanged();

private:
    std::shared_ptr<DeviceControllerPrivate> d;
};

using DeviceControllerPtr = std::shared_ptr<DeviceController>;
using DeviceControllerWPtr = std::weak_ptr<DeviceController>;

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // DEVICECONTROLLER_H
