#ifndef PROTOCOLHANDLERSERIAL_H
#define PROTOCOLHANDLERSERIAL_H

#include <QObject>
#include "ProtocolHandler.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class ProtocolHandlerSerialPrivate;
class CLDEVICE_EXPORT ProtocolHandlerSerial : public ProtocolHandler
{
    Q_OBJECT
public:
    ProtocolHandlerSerial();
    ~ProtocolHandlerSerial();

    void attach(CommunicationInterfaceWPtr comm, const DeviceInfo &info) override;

    void upload(const QByteArray &data, ResultCallback cb = nullptr) override;
    void preview(ResultCallback cb = nullptr) override;
    void start(ResultCallback cb = nullptr) override;
    void pause(ResultCallback cb = nullptr) override;
    void resume(ResultCallback cb = nullptr) override;
    void stop(ResultCallback cb = nullptr) override;
    void home(int axis = XY_AXIS, ResultCallback cb = nullptr) override;
    void move(int axis, qreal distance, qreal speed = 1000, ResultCallback cb = nullptr) override;

    void sendCommand(QStringList cmds, ResultCallback cb = nullptr) override;
    void moveXY(qreal x, qreal y, bool absolute, qreal speed = 1000, ResultCallback cb = nullptr) override;

    void autoFocus(ResultCallback cb = nullptr) override;
    void laserOn(int power, ResultCallback cb = nullptr) override;
    void laserOff(ResultCallback cb = nullptr) override;

    void getExposure(ResultCallback cb = nullptr) override;
    void setExposure(int value, ResultCallback cb = nullptr) override;

    void getFireDetectLevel(ResultCallback cb = nullptr) override;
    void setFireDetectLevel(FireDetectLevel level, ResultCallback cb = nullptr) override;

    void getCameraConfig(ResultCallback cb = nullptr) override;
    void setCameraConfig(QByteArray data, ResultCallback cb = nullptr) override;

    DeviceStatus status() const override;

protected:
    void timerEvent(QTimerEvent *e) override;

signals:

private:
    std::shared_ptr<ProtocolHandlerSerialPrivate> d;
};

DEVICE_END_NAMESPACE
CL_END_NAMESPACE

#endif // PROTOCOLHANDLERSERIAL_H
