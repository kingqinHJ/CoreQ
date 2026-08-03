#include "ProtocolHandlerSerial.h"

#define ENSURE_COMM() \
    auto comm = d->comm.lock(); \
    if (!comm) \
        return;

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class ProtocolHandlerSerialPrivate
{
public:
    int timerId = 0;

    DeviceInfo info;
    CommunicationInterfaceWPtr comm;

    DeviceStatus status;
};

ProtocolHandlerSerial::ProtocolHandlerSerial()
{
    d.reset(new ProtocolHandlerSerialPrivate);
    d->timerId = startTimer(1000);
}

ProtocolHandlerSerial::~ProtocolHandlerSerial()
{
    if (auto comm = d->comm.lock())
        comm->clearAllPendingCallbacks();

    killTimer(d->timerId);
    LOGD("%s", qUtf8Printable(d->info.address));
}

void ProtocolHandlerSerial::attach(CommunicationInterfaceWPtr comm, const DeviceInfo &info)
{
    if (auto _comm = comm.lock()) {
        if (_comm->type() == CT_Serial) {
            d->info = info;
            d->comm = comm;
        }
    }
}

void ProtocolHandlerSerial::upload(const QByteArray &data, ResultCallback cb)
{

}

void ProtocolHandlerSerial::preview(ResultCallback cb)
{

}

void ProtocolHandlerSerial::start(ResultCallback cb)
{

}

void ProtocolHandlerSerial::pause(ResultCallback cb)
{

}

void ProtocolHandlerSerial::resume(ResultCallback cb)
{

}

void ProtocolHandlerSerial::stop(ResultCallback cb)
{
    ENSURE_COMM()

}

void ProtocolHandlerSerial::home(int axis, ResultCallback cb)
{
    ENSURE_COMM()

    if (axis == Z_AXIS)
        comm->send("$HZ\n");
    else
        comm->send("$H\n");
}

void ProtocolHandlerSerial::move(int axis, qreal distance, qreal speed, ResultCallback cb)
{
    ENSURE_COMM()

    QString cmd;
    switch (axis) {
    case X_AXIS:
        cmd = "$x\nG91\nG21\nG1 X" + QString::number(distance) + " F" + QString::number(speed) + "\nG90\n";
        break;
    case Y_AXIS:
        cmd = "$x\nG91\nG21\nG1 Y" + QString::number(distance) + " F" + QString::number(speed) + "\nG90\n";
        break;
    case Z_AXIS:
        cmd = "$x\nG91\nG21\nG1 Z" + QString::number(distance) + " F" + QString::number(speed) + "\nG90\n";
        break;
    default:
        break;
    }

    if (!cmd.isEmpty())
        comm->send(cmd.toUtf8());
}

void ProtocolHandlerSerial::sendCommand(QStringList cmds, ResultCallback cb)
{

}

void ProtocolHandlerSerial::moveXY(qreal x, qreal y, bool absolute, qreal speed, ResultCallback cb)
{

}

void ProtocolHandlerSerial::autoFocus(ResultCallback cb)
{

}

void ProtocolHandlerSerial::laserOn(int power, ResultCallback cb)
{
    ENSURE_COMM()

    QString cmd = "$x\nM3 G1 F200 S" + QString::number(power*10) + "\n";
    comm->send(cmd.toUtf8());
}

void ProtocolHandlerSerial::laserOff(ResultCallback cb)
{
    ENSURE_COMM()

    comm->send("M5\n");
}

void ProtocolHandlerSerial::getExposure(ResultCallback cb)
{

}

void ProtocolHandlerSerial::setExposure(int value, ResultCallback cb)
{

}

void ProtocolHandlerSerial::getFireDetectLevel(ResultCallback cb)
{

}

void ProtocolHandlerSerial::setFireDetectLevel(FireDetectLevel level, ResultCallback cb)
{

}

void ProtocolHandlerSerial::getCameraConfig(ResultCallback cb)
{

}

void ProtocolHandlerSerial::setCameraConfig(QByteArray data, ResultCallback cb)
{

}

DeviceStatus ProtocolHandlerSerial::status() const
{
    return d->status;
}

void ProtocolHandlerSerial::timerEvent(QTimerEvent *e)
{
    ENSURE_COMM()

    comm->send("?\n");
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
