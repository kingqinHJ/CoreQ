#include "DeviceController.h"
#include "CommunicationManager.h"
#include "ProtocolHandlerSerial.h"
#include "ProtocolHandlerHttp.h"

CL_BEGIN_NAMESPACE
DEVICE_BEGIN_NAMESPACE

class DeviceControllerPrivate
{
public:
    DeviceInfo info;
    CommunicationManager *commMgr = nullptr;

    ProtocolHandlerPtr handler;
};

DeviceController::DeviceController(const DeviceInfo &info, CommunicationManager *commMgr)
{
    d.reset(new DeviceControllerPrivate);
    d->info = info;
    if (!info.laser.isEmpty())
        d->info.cap.defaultLaser = info.laser;
    d->commMgr = commMgr;

    auto comm = d->commMgr->create(info.address, info.connectionType);
    switch (info.connectionType) {
    case CT_Http:
        d->handler = std::make_shared<ProtocolHandlerHttp>();
        break;
    case CT_Serial:
        d->handler = std::make_shared<ProtocolHandlerSerial>();
        break;
    default:
        break;
    }

    if (d->handler) {
        d->handler->attach(comm, info);
        connect(d->handler.get(), &ProtocolHandler::disconnected,
                this, &DeviceController::disconnected);
        connect(d->handler.get(), &ProtocolHandler::deviceStatusChanged,
                this, &DeviceController::deviceStatusChanged);
        LOGD("%s", qUtf8Printable(info.address));
    }
    else {
        LOGW("%s:%d", qUtf8Printable(info.address), info.connectionType);
    }
}

DeviceController::~DeviceController()
{
    d->handler.reset();
    d->commMgr->destroy(d->info.address, false);
    LOGD("%s", qUtf8Printable(d->info.address));
}

void DeviceController::upload(const QByteArray &data, ResultCallback cb)
{
    if (d->handler)
        d->handler->upload(data, cb);
}

void DeviceController::preview(ResultCallback cb)
{
    if (d->handler)
        d->handler->preview(cb);
}

void DeviceController::start(ResultCallback cb)
{
    if (d->handler)
        d->handler->start(cb);
}

void DeviceController::pause(ResultCallback cb)
{
    if (d->handler)
        d->handler->pause(cb);
}

void DeviceController::resume(ResultCallback cb)
{
    if (d->handler)
        d->handler->resume(cb);
}

void DeviceController::stop(ResultCallback cb)
{
    if (d->handler)
        d->handler->stop(cb);
}

void DeviceController::home(int axis, ResultCallback cb)
{
    if (d->handler)
        d->handler->home(axis, cb);
}

void DeviceController::move(int axis, qreal distance, qreal speed, ResultCallback cb)
{
    if (d->handler)
        d->handler->move(axis, distance, speed, cb);
}

void DeviceController::sendCommand(QStringList cmds, ResultCallback cb)
{
    if (d->handler)
        d->handler->sendCommand(cmds, cb);
}

void DeviceController::moveXY(qreal x, qreal y, bool absolute, qreal speed, ResultCallback cb)
{
    if (d->handler)
        d->handler->moveXY(x, y, absolute, speed, cb);
}

void DeviceController::autoFocus(ResultCallback cb)
{
    if (d->handler)
        d->handler->autoFocus(cb);
}

void DeviceController::laserOn(int power, ResultCallback cb)
{
    if (d->handler)
        d->handler->laserOn(power, cb);
}

void DeviceController::laserOff(ResultCallback cb)
{
    if (d->handler)
        d->handler->laserOff(cb);
}

void DeviceController::getExposure(ResultCallback cb)
{
    if (d->handler)
        d->handler->getExposure(cb);
}

void DeviceController::setExposure(int value, ResultCallback cb)
{
    if (d->handler)
        d->handler->setExposure(value, cb);
}

void DeviceController::getFireDetectLevel(ResultCallback cb)
{
    if (d->handler)
        d->handler->getFireDetectLevel(cb);
}

void DeviceController::setFireDetectLevel(FireDetectLevel level, ResultCallback cb)
{
    if (d->handler)
        d->handler->setFireDetectLevel(level, cb);
}

void DeviceController::getCameraConfig(ResultCallback cb)
{
    if (d->handler)
        d->handler->getCameraConfig(cb);
}

void DeviceController::setCameraConfig(QByteArray data, ResultCallback cb)
{
    if (d->handler)
        d->handler->setCameraConfig(data, cb);
}

DeviceInfo DeviceController::info() const
{
    return d->info;
}

DeviceConfig DeviceController::config() const
{
    DeviceConfig v = d->info.cfg;
    if (v.deviceId.isEmpty())
        v.deviceId = d->info.id();
    v.applyDefaults(d->info.cap);
    return v;
}

void DeviceController::setConfig(const DeviceConfig &v)
{
    d->info.cfg = v;
    emit deviceConfigChanged();
}

DeviceStatus DeviceController::status() const
{
    if (d->handler)
        return d->handler->status();
    else
        return DeviceStatus();
}

DEVICE_END_NAMESPACE
CL_END_NAMESPACE
